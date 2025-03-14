#pragma once
#include <set>
#include <vector>
#include <map>
#include <array>
#include <math.h>
#include <numeric>
#include <memory>
#include <Eigen/SparseLU>
#include <Eigen/Dense>
#include<Eigen/SparseCholesky>	
//#include<Eigen/SparseLU>
//#include<Eigen/SparseQR>
#include<Eigen/SparseQR>
//Make more efficient triplet changes. For now it is a vector that gets reevaluated at analyse.
//Make all stifness matrices only triangular stuff
#include "raylib.h"
//float error tolerance
#define ERR_TOLERANCE 0.000000001
#define RENDER_SCALING_FACTOR 0.1

namespace Beams {
	using Point = Vector3;


	struct Node {
		float x, y, z;
		float xRender;
		float yRender;
		float zRender;

		Eigen::Index id; //for global matrix and DOFs use
		int pos; //position of Coords in the points vector;
		bool free_flag = true;
		std::set<size_t> inElements; 
		Node(float x_, float y_, float z_,size_t id_) {
			x = x_;
			y = y_;
			z = z_;

			xRender = x_*RENDER_SCALING_FACTOR;
			yRender = y_*RENDER_SCALING_FACTOR;
			zRender = z_*RENDER_SCALING_FACTOR;

			id = id_;
		}
		/*bool operator< (const Node  & v) {
			if (std::abs(x - v.x) >ERR_TOLERANCE) return x < v.x;
			if (std::abs(y - v.y) >ERR_TOLERANCE) return y < v.y;
			return z < v.z;
		}*/
	};

	bool operator< (Node const& l, Node const& v) {
		if (std::abs(l.x - v.x) > ERR_TOLERANCE) return l.x < v.x;
		if (std::abs(l.y - v.y) > ERR_TOLERANCE) return l.y < v.y;
		return l.z < v.z;
	}

	//UNUSED -> maybe for map with common local stiffness vectors mapped to element length. Not gonna b faster
	class Length {
		float l;

		bool operator< (const Length& v) {
			if (std::abs(l - v.l) > ERR_TOLERANCE) return l < v.l;
			return false;
		}
	};

	class Section {
	public:
		float Area;
		float Izz;
		float Iyy;
		float Modulus;
		float G;

		//std::map<Length, Eigen::SparseMatrix<float>> LengthStiffnessMap;//Not implemented. I'll have the matrices hopefully next to the cosine stuff for the operations
		float EIz12; 
		float EIy12; 
		float EIz6;
		float EIy6;
		float EIz4;
		float EIy4;
		float EIz2;
		float EIy2;


		Section(float _Area, float _Modulus, float _G, float _Izz, float _Iyy = NULL) {
			Area = _Area;
			Izz = _Izz;
			Iyy = (_Iyy == NULL) ? _Izz : Iyy;
			Modulus = _Modulus;
			G = _G;
			EIz12 = 12 * Modulus * Izz;
			EIy12 = 12 * Modulus * Iyy;
			EIz6 = 6 * Modulus * Izz;
			EIy6 = 6 * Modulus * Iyy;
			EIz4 = 4 * Modulus * Izz;
			EIy4 = 4 * Modulus * Iyy;
			EIz2 = 2 * Modulus * Izz;
			EIy2 = 2 * Modulus * Iyy;
		}
	};
	
	class vBeam {
		Eigen::Index id;
		
		Eigen::SparseMatrix<float> localBmatrix;

		float Len;
		//make properties for common stifness matrices
		size_t sectionId;
		

		void calc_BMatrixTEST() { 
			float Area = 100;
			float Izz = 100;
			float Iyy = 100;
			float Modulus = 210000;
			float Lsq = Len * Len;
			float sixLen = 6 * Len;
			float EIz12 = 12 * Modulus * Izz / pow(Len, 3);
			float EIy12 = 12 * Modulus * Iyy / pow(Len, 3);
			float EIz6 = 6 * Modulus * Izz / pow(Len, 2);
			float EIy6 = 6 * Modulus * Iyy / pow(Len, 2);
			float EIz4 = 4 * Modulus * Izz / Len;
			float EIy4 = 4 * Modulus * Iyy / Len;
			float EIz2 = 2 * Modulus * Izz / Len;
			float EIy2 = 2 * Modulus * Iyy / Len;
			std::vector<Eigen::Triplet<float>> locStiffness_triplets(47); 




			float k1 = Modulus * Area / Len;
			float k2 = Modulus * Area / Len;// FIX THIS THIS IS WRONG <--------------------------------------------------
			locStiffness_triplets.emplace_back(0, 0, k1);
			locStiffness_triplets.emplace_back(0, 6, -k1);
			locStiffness_triplets.emplace_back(1, 1, EIz12);
			locStiffness_triplets.emplace_back(1, 5, EIz6);
			locStiffness_triplets.emplace_back(1, 7, -EIz12);
			locStiffness_triplets.emplace_back(1, 11, EIz6);
			locStiffness_triplets.emplace_back(2, 2, EIy12);
			locStiffness_triplets.emplace_back(2, 4, -EIy6);
			locStiffness_triplets.emplace_back(2, 8, -EIy12);
			locStiffness_triplets.emplace_back(2, 10, -EIy6);
			locStiffness_triplets.emplace_back(3, 3, k2);
			locStiffness_triplets.emplace_back(3, 9, -k2);
			locStiffness_triplets.emplace_back(4, 2, -EIy6);
			locStiffness_triplets.emplace_back(4, 4, EIy4);
			locStiffness_triplets.emplace_back(4, 8, EIy6);
			locStiffness_triplets.emplace_back(4, 10, EIy2);
			locStiffness_triplets.emplace_back(5, 1, EIz6);
			locStiffness_triplets.emplace_back(5, 5, EIz4);
			locStiffness_triplets.emplace_back(5, 7, -EIz6);
			locStiffness_triplets.emplace_back(5, 11, EIz2);
			locStiffness_triplets.emplace_back(6, 0, -k1);
			locStiffness_triplets.emplace_back(6, 6, k1);
			locStiffness_triplets.emplace_back(7, 1, -EIz12);
			locStiffness_triplets.emplace_back(7, 5, -EIz6);
			locStiffness_triplets.emplace_back(7, 7, EIz12);
			locStiffness_triplets.emplace_back(7, 11, -EIz6);
			locStiffness_triplets.emplace_back(8, 2, -EIy12);
			locStiffness_triplets.emplace_back(8, 4, EIy6);
			locStiffness_triplets.emplace_back(8, 8, EIy12);
			locStiffness_triplets.emplace_back(8, 10, EIy6);
			locStiffness_triplets.emplace_back(9, 3, -k2);
			locStiffness_triplets.emplace_back(9, 9, k2);
			locStiffness_triplets.emplace_back(10, 2, -EIy6);
			locStiffness_triplets.emplace_back(10, 4, EIy2);
			locStiffness_triplets.emplace_back(10, 8, EIy6);
			locStiffness_triplets.emplace_back(10, 10, EIy4);
			locStiffness_triplets.emplace_back(11, 1, EIz6);
			locStiffness_triplets.emplace_back(11, 5, EIz2);
			locStiffness_triplets.emplace_back(11, 7, -EIz6);
			locStiffness_triplets.emplace_back(11, 11, EIz4);

			localBmatrix.setFromTriplets(locStiffness_triplets.begin(), locStiffness_triplets.end());

		};

		void calc_Len(Node& N2, Node& N1) {
			Len = std::pow((N2.x - N1.x), 2) + std::pow((N2.y - N1.y), 2) + std::pow((N2.z - N1.z), 2);
			Len = std::sqrt(Len);
		};
	
		void calc_BMatrix(const Section section) {
			// make it in a way to have only ONE division with the L. propably faster.
			float Area = section.Area;
			float Izz = section.Izz;
			float Iyy = section.Iyy;
			float Modulus = section.Modulus;
			float Lsq = Len * Len;
			float Lcb = Len * Len * Len;
			float EIz12 = section.EIz12 / Lcb;
			float EIy12 = section.EIy12 / Lcb;
			float EIz6 = section.EIz6 / Lsq;
			float EIy6 = section.EIy6 / Lsq;
			float EIz4 = section.EIz4 / Len;
			float EIy4 = section.EIy4 / Len;
			float EIz2 = section.EIz2 / Len;
			float EIy2 = section.EIy2 / Len;
			std::vector<Eigen::Triplet<float>> locStiffness_triplets(47);




			float k1 = Modulus * Area / Len;
			float k2 = Modulus * Area / Len;// FIX THIS THIS IS WRONG <--------------------------------------------------
			locStiffness_triplets.emplace_back(0, 0, k1);
			locStiffness_triplets.emplace_back(0, 6, -k1);
			locStiffness_triplets.emplace_back(1, 1, EIz12);
			locStiffness_triplets.emplace_back(1, 5, EIz6);
			locStiffness_triplets.emplace_back(1, 7, -EIz12);
			locStiffness_triplets.emplace_back(1, 11, EIz6);
			locStiffness_triplets.emplace_back(2, 2, EIy12);
			locStiffness_triplets.emplace_back(2, 4, -EIy6);
			locStiffness_triplets.emplace_back(2, 8, -EIy12);
			locStiffness_triplets.emplace_back(2, 10, -EIy6);
			locStiffness_triplets.emplace_back(3, 3, k2);
			locStiffness_triplets.emplace_back(3, 9, -k2);
			locStiffness_triplets.emplace_back(4, 2, -EIy6);
			locStiffness_triplets.emplace_back(4, 4, EIy4);
			locStiffness_triplets.emplace_back(4, 8, EIy6);
			locStiffness_triplets.emplace_back(4, 10, EIy2);
			locStiffness_triplets.emplace_back(5, 1, EIz6);
			locStiffness_triplets.emplace_back(5, 5, EIz4);
			locStiffness_triplets.emplace_back(5, 7, -EIz6);
			locStiffness_triplets.emplace_back(5, 11, EIz2);
			locStiffness_triplets.emplace_back(6, 0, -k1);
			locStiffness_triplets.emplace_back(6, 6, k1);
			locStiffness_triplets.emplace_back(7, 1, -EIz12);
			locStiffness_triplets.emplace_back(7, 5, -EIz6);
			locStiffness_triplets.emplace_back(7, 7, EIz12);
			locStiffness_triplets.emplace_back(7, 11, -EIz6);
			locStiffness_triplets.emplace_back(8, 2, -EIy12);
			locStiffness_triplets.emplace_back(8, 4, EIy6);
			locStiffness_triplets.emplace_back(8, 8, EIy12);
			locStiffness_triplets.emplace_back(8, 10, EIy6);
			locStiffness_triplets.emplace_back(9, 3, -k2);
			locStiffness_triplets.emplace_back(9, 9, k2);
			locStiffness_triplets.emplace_back(10, 2, -EIy6);
			locStiffness_triplets.emplace_back(10, 4, EIy2);
			locStiffness_triplets.emplace_back(10, 8, EIy6);
			locStiffness_triplets.emplace_back(10, 10, EIy4);
			locStiffness_triplets.emplace_back(11, 1, EIz6);
			locStiffness_triplets.emplace_back(11, 5, EIz2);
			locStiffness_triplets.emplace_back(11, 7, -EIz6);
			locStiffness_triplets.emplace_back(11, 11, EIz4);

			localBmatrix.setFromTriplets(locStiffness_triplets.begin(), locStiffness_triplets.end());

		};


	public:
		//Node* node1;
		//Node* node2;
		//Node* node3; //cross section  orietnation
		size_t node1Pos, node2Pos, node3Pos;
		//Eigen::Index n1Id, n2Id, n3Id;


		vBeam(Eigen::Index id_, Node& N1, Node& N2, Node& N3, size_t _sectionId, const Section& section) {
			localBmatrix.resize(12, 12);
			N1.free_flag = false;
			N2.free_flag = false;
			id = id_;

			N1.inElements.insert(static_cast<size_t>(id));
			N2.inElements.insert(static_cast<size_t>(id));
			N3.inElements.insert(static_cast<size_t>(id));

			node1Pos = N1.pos;
			node2Pos = N2.pos;
			node3Pos = N3.pos;



			sectionId = _sectionId;
			calc_Len(N2,N1);

			calc_BMatrix(section);
		};
	
		void reCalc(std::vector<Node>& Nodes,const Section& section) {
			calc_Len(Nodes[node2Pos], Nodes[node1Pos]);
			calc_BMatrix(section);
		}

		const size_t getSectionId() {
			return sectionId;
		}

		Eigen::Index getID() {
			return id;
		}

		void LocalMatrix2GlobalTriplets(std::vector<Eigen::Triplet<float>>& globalTriplets,std::vector<Node>& Nodes ,Section& section) {
			//using direction cosine matrix because reasons and it works also and no gimbal lock or whatever this stuff is 
			static const Eigen::Vector3d xAxis(1, 0, 0);
			static const Eigen::Vector3d yAxis(0, 1, 0);
			static const Eigen::Vector3d zAxis(0, 0, 1);


			Eigen::Vector3d localX_unit(Nodes[node2Pos].x - Nodes[node1Pos].x, Nodes[node2Pos].y - Nodes[node1Pos].y, Nodes[node2Pos].z - Nodes[node1Pos].z);
			Eigen::Vector3d localY_unit(Nodes[node3Pos].x - Nodes[node1Pos].x, Nodes[node3Pos].y - Nodes[node1Pos].y, Nodes[node3Pos].z - Nodes[node1Pos].z);//a vector in XY local plane
			Eigen::Vector3d localZ_unit;



			localX_unit = localX_unit / localX_unit.norm();
			localY_unit = localY_unit - localY_unit.dot(localX_unit) * localX_unit;// Y local  vector
			localY_unit = localY_unit / localY_unit.norm(); //Y local unit Vector

			
			localZ_unit = localX_unit.cross(localY_unit);
			//if (std::abs(localX_unit.dot(localY_unit)) > ERR_TOLERANCE) {
				//localY_unit == localZ_unit.cross(localX_unit);
			//}


			//std::cout << "BeamVectors -> \n X:\n" << localX_unit << "  \nY:\n" << localY_unit << "  \nZ:\n" << localZ_unit << " \n ";


			std::vector<Eigen::Triplet<float>> dirCosineMat_triplets(37);


			for (size_t i = 0; i < 12; i += 3) {
				dirCosineMat_triplets.emplace_back(0 + i, 0 + i, localX_unit.adjoint() * xAxis);
				dirCosineMat_triplets.emplace_back(0 + i, 1 + i, localX_unit.adjoint() * yAxis);
				dirCosineMat_triplets.emplace_back(0 + i, 2 + i, localX_unit.adjoint() * zAxis);

				dirCosineMat_triplets.emplace_back(1 + i, 0 + i, localY_unit.adjoint() * xAxis);
				dirCosineMat_triplets.emplace_back(1 + i, 1 + i, localY_unit.adjoint() * yAxis);
				dirCosineMat_triplets.emplace_back(1 + i, 2 + i, localY_unit.adjoint() * zAxis);

				dirCosineMat_triplets.emplace_back(2 + i, 0 + i, localZ_unit.adjoint() * xAxis);
				dirCosineMat_triplets.emplace_back(2 + i, 1 + i, localZ_unit.adjoint() * yAxis);
				dirCosineMat_triplets.emplace_back(2 + i, 2 + i, localZ_unit.adjoint() * zAxis);

				/*if (i == 0) {// SHOW DIRECTION COSINE MATRIX
					Eigen::SparseMatrix<float> test(3, 3);
					test.setFromTriplets(dirCosineMat_triplets.begin(), dirCosineMat_triplets.end());
				}*/
			}


			Eigen::SparseMatrix<float> cosMatrix(12, 12);
			cosMatrix.setFromTriplets(dirCosineMat_triplets.begin(), dirCosineMat_triplets.end());

			Eigen::SparseMatrix<float> globalB = cosMatrix * localBmatrix * (cosMatrix.transpose());
			std::cout << Eigen::MatrixXf(globalB);

			//for (int k = 0; k < globalB.outerSize(); ++k)
			Eigen::Index nid1 = Nodes[node1Pos].id;
			Eigen::Index nid2 = Nodes[node2Pos].id;

			for (int k = 0; k < 6; ++k)
			{

				Eigen::SparseMatrix<float>::InnerIterator it(globalB, k);
				size_t counter = 0;
				std::ptrdiff_t i;
				for (i = 0; i < 6; ++i) //local Node 1 for Node 1 in global
				{					
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					float val = (it + i).value();
					
					globalTriplets.emplace_back(nid1*6  + row, nid1 * 6 + col, val);

				}
				for (i = 6; it+i; ++i) {
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					float val = (it + i).value();

					globalTriplets.emplace_back(nid2 * 6 + row-6, nid1 * 6 + col, val);
				}
			}
			for (int k = 6; k < globalB.outerSize(); ++k)
			{

				Eigen::SparseMatrix<float>::InnerIterator it(globalB, k);
				size_t counter = 0;
				std::ptrdiff_t i;
				for (i = 0; i < 6; ++i) //local Node 1 for Node 1 in global
				{
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					float val = (it + i).value();

					globalTriplets.emplace_back(nid1 * 6 + row , nid2 * 6 + col - 6, val);

				}
				for (i = 6; it+i; ++i) {
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					float val = (it + i).value();

					globalTriplets.emplace_back(nid2 * 6 + row - 6, nid2 * 6 + col - 6, val);
				}
			}
		}
	
};


	
	class Model {
		std::vector<Eigen::Triplet<float>> globalK_triplets;
		std::vector<Eigen::Triplet<float>> globalF_triplets;
		//Eigen::VectorXf U , F;
		Eigen::SparseVector<float> U, F;
		
		std::vector<Node> Nodes;
		std::map<Point,size_t> pointMap;//Unfortunately, Duplicate. 

		std::vector<Section> Sections;

		size_t nId_Last = 0;
		//std::vector <std::ptrdiff_t> Element_Nids;
		std::vector<vBeam> Elements;
		Eigen::Index eId_Last = 0;

		std::vector<size_t> BCpinned;//UNUSED- and going to be unused.
		std::vector<size_t> BCfixed;
		std::map<size_t, std::array<float,6>> Forces;


		bool solved = false;
		//Raygui does not render large positions well (thigns far away vanish,  probably because I dont use a custom shader). Thus I divide all deflection data /10
		//Done here to only do it once and not every frame.
		Eigen::SparseVector<float> Urender;
		float scaleFactor = 1; //scale Happens Here As Well to do it once and not in every frame

	public:
		
		void addNode(Point point) {
			// I dont mind time spent here. 
			//solved = false;
			std::vector<size_t> elNodePos;
			
			
			Nodes.emplace_back(point.x, point.y, point.z, Nodes.size());
			Nodes.back().pos = Nodes.size() - 1;
			
		}

		void removeNode(size_t pos) {//remove node from nth position
			Node& nToRemove = Nodes[pos];


			for (size_t i = pos; i < Nodes.size(); i++) {//correct positions in the vector
				Nodes[i].pos--;
				

			}
			

			std::vector<Eigen::Index> elementIdsToRemove;
			for (auto& element : Elements) {
				if (std::find(Nodes[pos].inElements.begin(), Nodes[pos].inElements.end(), Nodes[pos].id) != Nodes[pos].inElements.end()) {
					solved = false;
					removeElement(element.getID());
					continue;
				}
				size_t nPos = (element.node1Pos > pos) ? element.node1Pos-- : element.node1Pos;
				nPos = (element.node2Pos > pos) ? element.node2Pos-- : element.node2Pos;
				nPos = (element.node3Pos > pos) ? element.node3Pos-- : element.node3Pos;

			}
			for (auto& id : elementIdsToRemove) {

			}

			Nodes.erase(Nodes.begin() + pos);
			//Points.erase(Points.begin() + pos); 
			//Nodes.erase(Nodes.begin() + pos);
		}

		bool addElement(size_t n1, size_t n2, size_t n3,size_t sectionID) {

			if (Sections.size() - 1 < sectionID) return false;
			
			solved = false;


			F.resize(F.size() + ((int)Nodes[n1].free_flag + (int)Nodes[n2].free_flag)*6);//free nodes about to be in element add 6 Dofs to F

			Elements.emplace_back(eId_Last, Nodes[n1], Nodes[n2], Nodes[n3],sectionID,Sections[sectionID]);

					
			
			eId_Last++;
			return true;
		}

		bool removeElement(Eigen::Index eId) {
			//should do binary search here. Elements are ID sorted (ensure this 100% first)
			size_t counter = 0;
			for (counter = 0; counter < Elements.size(); counter++) {// E ti na kanw re file kakos tropos gia na ta kanw iterate. Binary Search?<----------------------------------;
				vBeam& el = Elements[counter];
				if (el.getID() == eId) {

					std::set<size_t> &n1Els = Nodes[el.node1Pos].inElements;
					std::set<size_t>& n2Els = Nodes[el.node2Pos].inElements;
					std::set<size_t>& n3Els = Nodes[el.node3Pos].inElements;

					n1Els.erase(n1Els.find((size_t)eId));
					n2Els.erase(n2Els.find((size_t)eId));
					n3Els.erase(n3Els.find((size_t)eId));


					if (n1Els.size() == 0)Nodes[el.node1Pos].free_flag = true;
					if (n2Els.size() == 0)Nodes[el.node2Pos].free_flag = true;

					Elements.erase(Elements.begin() + counter); //menoun ta alla. Tha mporousa na ta kanw iterate kai na riksw ta elID tous... Alla tha eprepe na allaksei to inElements.

					//break;
					return true;
				}
			}
			return false;
		}

		void oneElementTest() {
			/*
			Nodes.emplace_back(0, 0, 0, 0);
			Nodes.emplace_back(50, 0, 0, 1);
			Nodes.emplace_back(0.2, 1, 0, 2);
			Nodes.emplace_back(100, 0, 0, 3);
			Nodes.emplace_back(150, 0, 0, 4);
			Nodes.emplace_back(200, 0, 0, 5);
			
			Elements.emplace_back(0, Nodes[0], Nodes[1], Nodes[2], 100, 100);
			Elements.emplace_back(1, Nodes[1], Nodes[3], Nodes[2], 100, 100);
			Elements.emplace_back(1, Nodes[3], Nodes[4], Nodes[2], 100, 100);
			Elements.emplace_back(1, Nodes[4], Nodes[5], Nodes[2], 100, 100);
			*/
			addNode(Point{ 0,0,0 });
			addNode(Point{50, 0, 0});
			addNode(Point{0.2, 50,0});
			addNode(Point{100, 0, 0});
			addNode(Point{150, 0, 0});
			addNode(Point{200, 0, 0});
			addNode(Point{25, 50, 0});
			addNode(Point{75, 50, 0 });
			addNode(Point{125, 50, 0 });


			addSection(100, 210000, 80000, 100);

			addElement(0, 1, 2, 0);
			addElement(1, 3, 2, 0);
			addElement(3, 4, 2, 0);
			addElement(4, 5, 2, 0);


			BCfixed.push_back(0);
			Forces[4][1] = 100;
			//Force.emplace(std::make_pair({ 0,100.0f,0,0,0,0 });

			//size_t Fsize = 54;
			//F.resize(24);
			//F << 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0;
			//F.insert(19) = 100;

		};
				
		void addSection(float _Area, float _Modulus, float _G, float _Izz, float _Iyy = NULL) {
			Sections.emplace_back(_Area, _Modulus, _G, _Izz, _Iyy = NULL);
			
		}
		
		void addForce(size_t nodePos, size_t Dof, float val) {

		}

		void solve() {
			//PENDING CHECKING FOR CONDITIONS ETC
			globalK_triplets.clear();
			globalF_triplets.clear();
			if (BCfixed.size() + BCpinned.size() < 1) {
				solved = false;
				return;
			}


			//find problem DOFs and Make unused nodes last. Initialize globMatrix.
			size_t noDofs = 0;
			size_t foundUnused = 0;
			size_t lastID = 0;

			std::vector<size_t> nodes_IdOrder(Nodes.size());
			std::iota(nodes_IdOrder.begin(), nodes_IdOrder.end(), 0);


			//Bubble Sort Nodes Based on IDs. 
			bool hasSwapped = true;
			size_t lastUnsorted = Nodes.size();
			while (hasSwapped) {
				hasSwapped = false;
				for (int i = 0; i < lastUnsorted-1; i++) {
					if (Nodes[nodes_IdOrder[i]].id > Nodes[nodes_IdOrder[i + 1]].id) {
						size_t large = nodes_IdOrder[i];
						nodes_IdOrder[i] = nodes_IdOrder[i + 1];
						nodes_IdOrder[i + 1] = large;
						hasSwapped = true;
					}
				}
				lastUnsorted--;
			}
			
			//ON SORTED BY IDs ARRAY, put free nodes last.
			for (auto& pos : nodes_IdOrder) {
				Node& node = Nodes[pos];
				//ensure no gaps in Ids. not sure for this 
				if (node.id - lastID > 1) {
					foundUnused++;
				}
				lastID = node.id;

				if (!node.free_flag) {
					node.id = node.id - foundUnused;
					noDofs++;
				}
				else {
					node.id = Nodes.size() - 1 - foundUnused;//unused nodes go above other unused nodes in the end 
					foundUnused++;

				}
			}



			if (noDofs < 1) return;

			//populate global stigness matrix triplets -- Helper (free) Elements (not beam nodes) must have last IDs for correct placement in the matrix
			Eigen::SparseMatrix<float> testGlobAll(noDofs*10, noDofs*10);
			for (auto& element : Elements) {
				element.LocalMatrix2GlobalTriplets(globalK_triplets,Nodes,Sections[element.getSectionId()]);
				testGlobAll.setFromTriplets(globalK_triplets.begin(),globalK_triplets.end());
			}
			

			noDofs = 6 * (noDofs - BCfixed.size()) - 3 * BCpinned.size();
			Eigen::SparseMatrix<float> globMatr(noDofs, noDofs);


			//remove rows and columns from BCs
			//Unfortunately I Have to duplicate. Pending better solution. 
			std::vector<Eigen::Triplet<float>> triplets_AfterBCs;
			int row,col,val;
			for (auto& triplet : globalK_triplets) {
				row = triplet.row(); col = triplet.col(); val = triplet.value();
				for (size_t fixBCid : BCfixed) {
					if (row < fixBCid * 6 && col < fixBCid * 6) {
						triplets_AfterBCs.emplace_back(row, col, val);
					}
					else if (row < fixBCid * 6 && col > fixBCid * 6 + 5) {
						triplets_AfterBCs.emplace_back(row, col - 6, val);
					}
					else if (row > fixBCid * 6 + 5 && col < fixBCid * 6) {
						triplets_AfterBCs.emplace_back(row - 6, col , val);
					}
					else if (row > fixBCid * 6 + 5 && col > fixBCid * 6 + 5) {
						triplets_AfterBCs.emplace_back(row-6, col - 6, val);
					}
				}
			}


			//Populate globMatrix matrix from triplets
			globMatr.setFromTriplets(triplets_AfterBCs.begin(), triplets_AfterBCs.end());
			//std::cout << Eigen::MatrixXf(globMatr)<<std::endl;
			F.resize(globMatr.rows());
			

			std::vector<Eigen::Triplet<float>> Ftriplets_AfterBCs;
			//make forces vector 
			for (auto& force : Forces) {
				size_t forceMatrixPos = force.first*6;
		
				for (size_t BCid : BCfixed) {
					forceMatrixPos = (BCid < forceMatrixPos) ? forceMatrixPos - 6 : forceMatrixPos;
				}
				F.insert((Eigen::Index)forceMatrixPos) = force.second[0];
				F.insert((Eigen::Index)forceMatrixPos+1) = force.second[1];
				F.insert((Eigen::Index)forceMatrixPos + 2) = force.second[2];
				F.insert((Eigen::Index)forceMatrixPos + 3) = force.second[3];
				F.insert((Eigen::Index)forceMatrixPos+4) = force.second[4];
				F.insert((Eigen::Index)forceMatrixPos+5) = force.second[5];

			}
			std::cout << "\n" << F << "\n";

			

			Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
			solver.compute(globMatr);

			if (solver.info() == Eigen::Success) {
				// decomposition Succesfull
				std::cout << "Sparse LU Decomposition Successful\n";
				U = solver.solve(F);
				if (solver.info() != Eigen::Success) {
					// solving failed
					std::cout << "solving failed - Exiting";
					return;
				}
				std::cout << "Sparse Solving Successful\n";
				solved = true;
				std::cout << U;
				Urender = U * 0.1 * scaleFactor;

				return;
			}
			std::cout << "Sparse LU decomposition failed\nConverting to Dense\n";
			Eigen::MatrixXf A(globMatr);                       // DO EXCEPTION HERE if this fails Also
			U = A.lu().solve(Eigen::VectorXf(F)).sparseView(); 
			std::cout << F <<"\n" << A << "\n" << U;
			Urender.resize(U.rows());
			Urender = U * RENDER_SCALING_FACTOR * scaleFactor;
			solved = true;
			
		}
		
		const std::vector<Node>& getNodes() {
			return Nodes;
		}

		const std::vector<vBeam>& getElements() {
			return Elements;
		}

		Point getDeflection(size_t nodeId) {
			if (!solved) return Vector3Zero();

			size_t fixedNodesBefore = 0;
			for (size_t fixedId : BCfixed) {
				if (fixedId < nodeId) fixedNodesBefore++;
				else if (fixedId == nodeId) return Vector3Zero();
			}
			size_t afterBcId= nodeId * 6 - fixedNodesBefore * 6;
			if (afterBcId >= U.rows()) return Vector3Zero(); //Free node, not in Stifness matrix


			return Vector3{ U.coeff(afterBcId),U.coeff(afterBcId + 1),U.coeff(afterBcId + 2) };

		}

		Vector3 getDeflectionRender(size_t nodeId) {
			if (!solved) return Vector3Zero();

			size_t fixedNodesBefore = 0;
			for (size_t fixedId : BCfixed) {
				if (fixedId < nodeId) fixedNodesBefore++;
				else if (fixedId == nodeId) return Vector3Zero();
			}
			size_t afterBcId = nodeId * 6 - fixedNodesBefore * 6;
			if (afterBcId >= U.rows()) return Vector3Zero(); //Free node, not in Stifness matrix


			return Vector3{ Urender.coeff(afterBcId) ,Urender.coeff(afterBcId + 1) ,Urender.coeff(afterBcId + 2)  };

		}

		Point getForce(size_t nodeId) {
			auto it = Forces.find(nodeId);
			
			if (it == Forces.end()) return Vector3Zero();
			return Vector3{ it->second[0],it->second[1],it->second[2] };


		}

		void printDeformed() {
			for (auto& node : Nodes) {
				Point a;
				if (node.free_flag) {
					a = Vector3Zero();
					continue;
				}
				a = getDeflection(node.id);
				std::cout << a.x << " " << a.y << " " << a.z << " \n";
			}
		}
		
		bool isSolved() {
			return solved;
		}
	};
};



