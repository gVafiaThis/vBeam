#pragma once
#include <set>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
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
#include <algorithm>

//double error tolerance
#define ERR_TOLERANCE 0.000000001
#define RENDER_SCALING_FACTOR 0.1

//#define DEBUG_PRINTS



namespace Beams {

	struct Node {
		double x, y, z;
		double xRender;
		double yRender;
		double zRender;

		size_t matrixPos; //for matrix use
		
		int pos; //position of Coords in the points vector;

		bool free_flag = true; //DOFs not used in stifness matrix
		std::set<Eigen::Index> inElements;
		Node(double x_, double y_, double z_, size_t id_) {
			x = x_;
			y = y_;
			z = z_;

			xRender = x_ * RENDER_SCALING_FACTOR;
			yRender = y_ * RENDER_SCALING_FACTOR;
			zRender = z_ * RENDER_SCALING_FACTOR;

			matrixPos = id_;
		}
		/*bool operator< (const Node  & v) {
			if (std::abs(x - v.x) >ERR_TOLERANCE) return x < v.x;
			if (std::abs(y - v.y) >ERR_TOLERANCE) return y < v.y;
			return z < v.z;
		}*/
	};

	//UNUSED -> potentially to be used for map on nodes to check duplication. 
	bool operator< (Node const& l, Node const& v) {
		if (std::abs(l.x - v.x) > ERR_TOLERANCE) return l.x < v.x;
		if (std::abs(l.y - v.y) > ERR_TOLERANCE) return l.y < v.y;
		return l.z < v.z;
	}

	//UNUSED -> maybe for map with common local stiffness matrices mapped to element length. 
	class Length {
		double l;

		bool operator< (const Length& v) {
			if (std::abs(l - v.l) > ERR_TOLERANCE) return l < v.l;
			return false;
		}
	};


	class NodeContainer {
		std::vector<Node> Nodes;
		std::set<size_t> deleted;


		Node& get_NotDeleted(size_t pos) {
			auto it = deleted.find(pos);
			auto endIt = deleted.end();
			while (it != endIt) {
				pos++;
				it = deleted.find(pos);
			}
			return Nodes[pos];
		}

	public:
		class notDeleted_const_iterator {

		public:
			using iterator_category = std::forward_iterator_tag; 
			using difference_type = std::ptrdiff_t;
			using value_type = const Node;
			using pointer =  const Node*;
			using reference =  const Node&;

			explicit notDeleted_const_iterator(const std::vector<Node>& nodes, const std::set<size_t>& deleted, size_t pos) : m_nodes(nodes), m_deleted(deleted), m_pos(pos) {}

			reference operator*() const { return m_nodes[m_pos]; }
			//pointer operator->() { return m_ptr; }
			notDeleted_const_iterator& operator++() { 
				//m_ptr++; 
				m_pos++;
				while (m_deleted.find(m_pos) != m_deleted.end()) {
					m_pos++;
					//m_ptr++;
				}
				
				return *this; 
			}
			notDeleted_const_iterator operator++(int) { notDeleted_const_iterator tmp = *this; ++(*this); return tmp; }
			friend bool operator== (const notDeleted_const_iterator& a, const notDeleted_const_iterator& b) { return a.m_pos == b.m_pos; };
			friend bool operator!= (const notDeleted_const_iterator& a, const notDeleted_const_iterator& b) { return a.m_pos != b.m_pos; };
		private:	
			//pointer m_ptr;
			const std::vector<Node>& m_nodes;
			const std::set<size_t>& m_deleted;
			size_t m_pos;
		};
		
		notDeleted_const_iterator begin() const {  

			return notDeleted_const_iterator(Nodes, deleted, 0+(!size() * Nodes.size())); 
		}
		notDeleted_const_iterator end() const { return notDeleted_const_iterator(Nodes, deleted, Nodes.size()); }

		void emplace(Vector3& point) {
			if (deleted.size() > 0) {
				auto firstIt = deleted.begin();
				size_t pos = *firstIt;
				Nodes[pos].x = point.x;
				Nodes[pos].y = point.y;
				Nodes[pos].z = point.z;
				Nodes[pos].xRender = point.x*RENDER_SCALING_FACTOR;
				Nodes[pos].yRender = point.y*RENDER_SCALING_FACTOR;
				Nodes[pos].zRender = point.z*RENDER_SCALING_FACTOR;

				Nodes[pos].matrixPos = -1;
				Nodes[pos].free_flag= true; //extra safety

				deleted.erase(firstIt);
				return;
			}

			Nodes.emplace_back(point.x, point.y, point.z, -1);
			Nodes.back().pos = Nodes.size() - 1;


			return;

		}

		void remove(size_t pos) {
			if (pos >= Nodes.size()) return;
			deleted.insert(pos);
			return;
		}

		//Gets non deleted size
		size_t  size() const {
			return Nodes.size() - deleted.size();
		}

		void clear() {
			Nodes.clear();
			deleted.clear();
		}

		//Returns nth not deleted node. For iterations
		const Node& get_notDeleted(size_t pos) const {
			auto endIt = deleted.end();
			
			size_t undeleted = (deleted.begin() == deleted.end()) ? 0 :  *deleted.begin();
			for (size_t i = undeleted; i <= pos; i++) {
				if (deleted.find(i) != deleted.end()) pos++;
			}

			return Nodes[pos];

		}

		//Returns Node despite if its deleted or not. For Elements
		const Node& get_byPos(size_t pos) const {
			return Nodes[pos];
		}

		void setFree_byPos(size_t pos, bool free) {
			Nodes[pos].free_flag = free;
			return;
		}

		bool getFree_byPos(size_t pos) {
			return Nodes[pos].free_flag;
		}

		void add_InElement_byPos(size_t pos, Eigen::Index id) {
			Node& node = Nodes[pos];
			node.inElements.insert(id);
			//free gets set from outside here. 
		}

		void remove_InElement_byPos(size_t pos, Eigen::Index id) {
			Node& node = Nodes[pos];
			node.inElements.erase(id);
			node.free_flag = !node.inElements.size();
		}

		void setMatrixPos_byPos(size_t pos, Eigen::Index matPos) {
			Nodes[pos].matrixPos = matPos;
		}

	};

	class Section {
	public:
		std::vector<size_t> inElements;
		double Area;
		double Ixx;
		double Izz;
		double Iyy;
		double Modulus;
		double G;

		//preCalculated stuff for this section
		double EIz12;
		double EIy12;
		double EIz6;
		double EIy6;
		double EIz4;
		double EIy4;
		double EIz2;
		double EIy2;

		Section() {
			Area = 10;
			Ixx = 10;
			Izz = 10;
			Iyy = 10;
			Modulus = 10;
			G = 10;
			EIz12 = 12 * Modulus * Izz;
			EIy12 = 12 * Modulus * Iyy;
			EIz6 = 6 * Modulus * Izz;
			EIy6 = 6 * Modulus * Iyy;
			EIz4 = 4 * Modulus * Izz;
			EIy4 = 4 * Modulus * Iyy;
			EIz2 = 2 * Modulus * Izz;
			EIy2 = 2 * Modulus * Iyy;
		}

		Section(double _Area, double _Modulus, double _G, double _Ixx, double _Iyy, double _Izz) {
			Area = _Area;
			Izz = _Izz;
			Ixx = _Ixx;


			Iyy = (_Iyy == NULL) ? _Izz : _Iyy;
			Ixx = (_Ixx == NULL) ? 0.001 : _Ixx;//Hackia. Bad 

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

		Eigen::SparseMatrix<double> localBmatrix;
		Eigen::SparseMatrix<double> rotMatrix; //Cosine Matrix


		double Len;
		//make properties for common stifness matrices
		size_t sectionId;

		std::array<Eigen::Vector3d,3> localUnitVectors;


		void calc_BMatrixTEST() {
			double Area = 100;
			double Izz = 100;
			double Iyy = 100;
			double Modulus = 210000;
			double Lsq = Len * Len;
			double sixLen = 6 * Len;
			double EIz12 = 12 * Modulus * Izz / pow(Len, 3);
			double EIy12 = 12 * Modulus * Iyy / pow(Len, 3);
			double EIz6 = 6 * Modulus * Izz / pow(Len, 2);
			double EIy6 = 6 * Modulus * Iyy / pow(Len, 2);
			double EIz4 = 4 * Modulus * Izz / Len;
			double EIy4 = 4 * Modulus * Iyy / Len;
			double EIz2 = 2 * Modulus * Izz / Len;
			double EIy2 = 2 * Modulus * Iyy / Len;
			std::vector<Eigen::Triplet<double>> locStiffness_triplets(47);




			double k1 = Modulus * Area / Len;
			double k2 = Modulus * Area / Len;// FIX THIS THIS IS WRONG <--------------------------------------------------
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

		void calc_Len(const Node& N2, const Node& N1) {
			Len = std::pow((N2.x - N1.x), 2) + std::pow((N2.y - N1.y), 2) + std::pow((N2.z - N1.z), 2);
			Len = std::sqrt(Len);
		};

		void calc_BMatrix(const Section section) {
			localBmatrix.data().clear();
			double Area = section.Area;
			double Izz = section.Izz;
			double Iyy = section.Iyy;
			double Modulus = section.Modulus;

			double Lsq = 1 / std::pow(Len, 2);
			double Lcb = 1 / std::pow(Len, 3);
			double invLen = 1 / Len;
			double EIz12 = section.EIz12 * Lcb;
			double EIy12 = section.EIy12 * Lcb;
			double EIz6 = section.EIz6 * Lsq;
			double EIy6 = section.EIy6 * Lsq;
			double EIz4 = section.EIz4 * invLen;
			double EIy4 = section.EIy4 * invLen;
			double EIz2 = section.EIz2 * invLen;
			double EIy2 = section.EIy2 * invLen;

			//Possible overflow issues with above? Further testing
			/*double EIz12 = 12 * Modulus * Izz / pow(Len, 3);
			double EIy12 = 12 * Modulus * Iyy / pow(Len, 3);
			double EIz6 = 6 * Modulus * Izz / pow(Len, 2);
			double EIy6 = 6 * Modulus * Iyy / pow(Len, 2);
			double EIz4 = 4 * Modulus * Izz / pow(Len ,1);
			double EIy4 = 4 * Modulus * Iyy / pow(Len ,1);
			double EIz2 = 2 * Modulus * Izz / pow(Len ,1);
			double EIy2 = 2 * Modulus * Iyy / pow(Len ,1);*/




			std::vector<Eigen::Triplet<double>> locStiffness_triplets;




			double k1 = Modulus * Area / Len;
			double k2 = section.Ixx * section.G / Len;// FIX THIS THIS IS WRONG <--------------------------------------------------
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
#ifdef DEBUG_PRINTS
			static bool printed = false;
			if (printed)return;
			std::cout << "\n-------------------------------------------------------------------------------\n Local B Matrix\n" << Eigen::MatrixXd(localBmatrix) << "\n";
			printed = true;
#endif
		};

		void calc_LocalUnitVectors(const Node& N1, const Node& N2, const Node& N3) {
			static const Eigen::Vector3d xAxis(1, 0, 0);
			static const Eigen::Vector3d yAxis(0, 1, 0);
			static const Eigen::Vector3d zAxis(0, 0, 1);

			Eigen::Vector3d localX_unit(N2.x - N1.x, N2.y - N1.y, N2.z - N1.z);
			Eigen::Vector3d localY_unit(N3.x - N1.x, N3.y - N1.y, N3.z - N1.z);//a vector in XY local plane
			Eigen::Vector3d localZ_unit;



			localX_unit = localX_unit / localX_unit.norm();
			localY_unit = localY_unit - localY_unit.dot(localX_unit) * localX_unit;// Y local  vector
			localY_unit = localY_unit / localY_unit.norm(); //Y local unit Vector


			localZ_unit = localX_unit.cross(localY_unit);


			//copy here has very low cost. 
			localUnitVectors[0] = localX_unit;
			localUnitVectors[1] = localY_unit;
			localUnitVectors[2] = localZ_unit;

		}

		void calc_rotMatrix() {
			static const Eigen::Vector3d xAxis(1, 0, 0);
			static const Eigen::Vector3d yAxis(0, 1, 0);
			static const Eigen::Vector3d zAxis(0, 0, 1);


			Eigen::Vector3d localX_unit = localUnitVectors[0];
			Eigen::Vector3d localY_unit = localUnitVectors[1];
			Eigen::Vector3d localZ_unit = localUnitVectors[2];


			std::vector<Eigen::Triplet<double>> dirCosineMat_triplets(37);


			for (size_t i = 0; i < 12; i += 3) {
				dirCosineMat_triplets.emplace_back(0 + i, 0 + i, localX_unit.adjoint() * xAxis);
				dirCosineMat_triplets.emplace_back(0 + i, 1 + i, localY_unit.adjoint() * xAxis);
				dirCosineMat_triplets.emplace_back(0 + i, 2 + i, localZ_unit.adjoint() * xAxis);

				dirCosineMat_triplets.emplace_back(1 + i, 0 + i, localX_unit.adjoint() * yAxis);
				dirCosineMat_triplets.emplace_back(1 + i, 1 + i, localY_unit.adjoint() * yAxis);
				dirCosineMat_triplets.emplace_back(1 + i, 2 + i, localZ_unit.adjoint() * yAxis);

				dirCosineMat_triplets.emplace_back(2 + i, 0 + i, localX_unit.adjoint() * zAxis);
				dirCosineMat_triplets.emplace_back(2 + i, 1 + i, localY_unit.adjoint() * zAxis);
				dirCosineMat_triplets.emplace_back(2 + i, 2 + i, localZ_unit.adjoint() * zAxis);
			}

			
			rotMatrix.data().clear();


			rotMatrix.setFromTriplets(dirCosineMat_triplets.begin(), dirCosineMat_triplets.end());
		}
	public:

		size_t node1Pos, node2Pos, node3Pos;


		vBeam(Eigen::Index id_, const Node& N1, const Node& N2, const Node& N3, size_t _sectionId, const Section& section) {
			localBmatrix.resize(12, 12);
			rotMatrix.resize(12, 12);
			id = id_;
			
			node1Pos = N1.pos;
			node2Pos = N2.pos;
			node3Pos = N3.pos;


			sectionId = _sectionId;
			calc_Len(N2, N1);

			calc_BMatrix(section);

			calc_LocalUnitVectors(N1, N2, N3);

			calc_rotMatrix();
		};

		void reCalc(NodeContainer& Nodes, const Section& section) {
			calc_Len(Nodes.get_byPos(node2Pos), Nodes.get_byPos(node1Pos));
			calc_BMatrix(section);
			calc_LocalUnitVectors(Nodes.get_byPos(node1Pos), Nodes.get_byPos(node2Pos), Nodes.get_byPos(node3Pos));
			calc_rotMatrix();
		}

		const size_t getSectionId() const {
			return sectionId;
		}

		Eigen::Index getID() {
			return id;
		}


		void LocalMatrix2GlobalTriplets(std::vector<Eigen::Triplet<double>>& globalTriplets, NodeContainer& Nodes, Section& section) {
			

			


			Eigen::SparseMatrix<double> globalB = rotMatrix * localBmatrix * rotMatrix.transpose();
			//std::cout << Eigen::MatrixXd(globalB);
#ifdef DEBUG_PRINTS

			std::cout << "\n\nCOS MATRIX FOR ELEMENT"<< id<<"__________\n"<<Eigen::MatrixXd(cosMatrix)<<"\n\n";
			std::cout << "\n\n" << globalB.coeff(1, 1)<<'\n';
#endif // DEBUG_PRINTS

			Eigen::Index nid1 = Nodes.get_byPos(node1Pos).matrixPos;
			Eigen::Index nid2 = Nodes.get_byPos(node2Pos).matrixPos;

			for (int k = 0; k < 6; ++k)
			{

				Eigen::SparseMatrix<double>::InnerIterator it(globalB, k);
				size_t counter = 0;
				std::ptrdiff_t i;
				for (i = 0; i < 6; ++i) //local Node 1 for Node 1 in global
				{
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					double val = (it + i).value();

					globalTriplets.emplace_back(nid1 * 6 + row, nid1 * 6 + col, val);

				}
				for (i = 6; it + i; ++i) {
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					double val = (it + i).value();

					globalTriplets.emplace_back(nid2 * 6 + row - 6, nid1 * 6 + col, val);
				}
			}
			for (int k = 6; k < globalB.outerSize(); ++k)
			{

				Eigen::SparseMatrix<double>::InnerIterator it(globalB, k);
				size_t counter = 0;
				std::ptrdiff_t i;
				for (i = 0; i < 6; ++i) //local Node 1 for Node 1 in global
				{
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					double val = (it + i).value();



					globalTriplets.emplace_back(nid1 * 6 + row, nid2 * 6 + col - 6, val);

				}
				for (i = 6; it + i; ++i) {
					Eigen::Index row = (it + i).row(); // row index
					Eigen::Index col = (it + i).col(); // col index (here it is equal to k)
					double val = (it + i).value();


					globalTriplets.emplace_back(nid2 * 6 + row - 6, nid2 * 6 + col - 6, val);
				}
			}
		}


		const std::array<std::array<double, 3>, 3> getLocalUnitVectors() const {
			std::array<double, 3> xUnit{localUnitVectors[0].coeff(0), localUnitVectors[0].coeff(1), localUnitVectors[0].coeff(2)};
			std::array<double, 3> yUnit{localUnitVectors[1].coeff(0), localUnitVectors[1].coeff(1), localUnitVectors[1].coeff(2)};
			std::array<double, 3> zUnit{localUnitVectors[2].coeff(0), localUnitVectors[2].coeff(1), localUnitVectors[2].coeff(2)};
			return std::array<std::array<double, 3>, 3>{xUnit,yUnit,zUnit};
		}


	};


	class Model {

		NodeContainer Nodes;
		std::vector<size_t> nodesPos_InMatrixOrder;

		std::map<size_t,Section> Sections;
		size_t secIdNext = 0;

		std::vector<vBeam> Elements;
		Eigen::Index eId_Last = 0;


		size_t noDofs = 0;
		bool solved = false;

		std::set<size_t> BCpinned;//UNUSED- and going to be unused.
		std::set<size_t> BCfixed;
		std::map<size_t, std::array<double, 6>> Forces;//node position to force. Position refers to All nodes, taking into account the deleted stuff.

		Eigen::SparseVector<double> U, F;
		//Raygui does not render large positions well (things far away vanish,  probably because I dont use a custom shader). Thus I divide all deflection data /10
		//Done here to only do it once and not every frame.
		Eigen::SparseVector<double> Urender;
		double scaleFactor = 1; //scale for output deformations
		

		

	public:

		void addNode(Vector3& point) {
			Nodes.emplace(point);
		}

		void addNode(Vector3 point) {
			Nodes.emplace(point);
		}

		//remove node from nth position 
		void removeNode(size_t pos) {
			const Node& node =  Nodes.get_byPos(pos);

			size_t elPos = 0;
			std::set<size_t> toRemove;//Remove from end to beginning. 
			for (auto& element : Elements) {//iterate elements only once. TODO: change with removeElementId when you do binary search
				if (std::find(node.inElements.begin(), node.inElements.end(), element.getID()) != node.inElements.end()) {
					toRemove.insert(elPos);
				}
				elPos++;
			}
			for (auto rit = toRemove.rbegin(); rit != toRemove.rend(); rit++) {
 				removeElement(*rit);
			}

			auto it = Forces.find(pos);
			if (it != Forces.end()) {
				Forces.erase(it);
			}

			auto itB = BCfixed.find(pos);
			if (itB != BCfixed.end()) BCfixed.erase(itB);

			Nodes.remove(pos);	
		}

		bool addElement(size_t n1Pos, size_t n2Pos, size_t n3Pos, size_t sectionID) {

			if (Sections.size() - 1 < sectionID) return false;
			solved = false;


			Elements.emplace_back(eId_Last, Nodes.get_byPos(n1Pos), Nodes.get_byPos(n2Pos), Nodes.get_byPos(n3Pos), sectionID, Sections[sectionID]);
			Sections[sectionID].inElements.emplace_back(eId_Last);

			

			//Update Nodes that are part of Element
			Nodes.add_InElement_byPos(n1Pos, eId_Last);
			Nodes.add_InElement_byPos(n2Pos, eId_Last);
			Nodes.add_InElement_byPos(n3Pos, eId_Last);
			Nodes.setFree_byPos(n1Pos, false);
			Nodes.setFree_byPos(n2Pos, false);

			eId_Last++;
			return true;
		}

		bool removeElement(size_t ePos) {
			if (ePos >= Elements.size()) return false;

			vBeam& el = Elements[ePos];

			solved = false;

			Eigen::Index eId = el.getID();
			Nodes.remove_InElement_byPos(el.node1Pos, eId);
			Nodes.remove_InElement_byPos(el.node2Pos, eId);
			Nodes.remove_InElement_byPos(el.node3Pos, eId);


			

			Elements.erase(Elements.begin() + ePos); 

			return true;


		}

		void oneElementTest() {

			addNode(Vector3{ 0,0,0 });
			addNode(Vector3{ 50, 0, 0 });
			addNode(Vector3{ 0.2, 50,0 });
			addNode(Vector3{ 100, 0, 0 });
			addNode(Vector3{ 150, 0, 0 });
			addNode(Vector3{ 200, 0, 0 });
			addNode(Vector3{ 25, 50, 0 });
			addNode(Vector3{ 75, 50, 0 });
			addNode(Vector3{ 125, 50, 0 });


			addSection(100, 210000, 80000, 1000, 100, 100);

			addElement(0, 1, 2, 0);
			addElement(1, 3, 2, 0);
			addElement(3, 4, 2, 0);
			addElement(4, 5, 2, 0);


			BCfixed.emplace(0);
			Forces[4][1] = 100;

		};

		void addSection(double _Area, double _Modulus, double _G, double _Ixx, double _Iyy, double _Izz) {
			Sections.emplace(std::make_pair(secIdNext, Section{ _Area, _Modulus, _G, _Ixx, _Iyy, _Izz }));
			secIdNext++;
		}

		//nodePos is the position containing deleted nodes. 
		void addForce(size_t nodePos, size_t Dof, double val) {
			std::array<double,6> ar{ 0,0,0,0,0,0 };
			auto it = Forces.emplace(std::make_pair(nodePos, ar));
			it.first->second[Dof] = val;
			solved = false;

		}

		void solve() {

			//----------------------------------------------------------------------------------------------------
			//Setup 
			//----------------------------------------------------------------------------------------------------
			//TODO: Check for unconstrained model
			std::vector<Eigen::Triplet<double>> globalK_triplets;//Contains global stiffness matrix triplets
			

			if (BCfixed.size() + BCpinned.size() < 1) {
				solved = false;
				return;
			}


			//----------------------------------------------------------------------------------------------------
			//Setting of node dof positions in stiffness matrix
			//----------------------------------------------------------------------------------------------------
			nodesPos_InMatrixOrder.clear();
			noDofs = 0;
			std::unordered_set<size_t> included;
			for (auto& element : Elements) {
				size_t n1Pos = element.node1Pos;
				size_t n2Pos = element.node2Pos;


				if (included.emplace(n1Pos).second) {
					Nodes.setMatrixPos_byPos(n1Pos, nodesPos_InMatrixOrder.size());
					nodesPos_InMatrixOrder.push_back(n1Pos);
					noDofs += 6;
				}

				if (included.emplace(n2Pos).second) {
					Nodes.setMatrixPos_byPos(n2Pos, nodesPos_InMatrixOrder.size());
					nodesPos_InMatrixOrder.push_back(n2Pos);
					noDofs += 6;
				}
			}

			if (noDofs < 1) return;
			
			#ifdef DEBUG_PRINTS
				std::cout << "\n------------------------------------\nNO DOFs: " << noDofs << "\n";
			#endif // DEBUG_PRINTS
			


			//----------------------------------------------------------------------------------------------------
			//Populate global stiffness matrix (triplets) from each element
			//----------------------------------------------------------------------------------------------------
			#ifdef DEBUG_PRINTS
				Eigen::SparseMatrix<double> testGlobAll(noDofs, noDofs);
			#endif

			for (auto& element : Elements) {
				element.LocalMatrix2GlobalTriplets(globalK_triplets, Nodes, Sections[element.getSectionId()]);

			

			}

			#ifdef DEBUG_PRINTS
				testGlobAll.setFromTriplets(globalK_triplets.begin(), globalK_triplets.end());
				std::cout << "\n------------------------------------\n Glob Matrix noRowDeletion:\n " << Eigen::MatrixXd(testGlobAll) << "\n";
			#endif // DEBUG_PRINTS

			size_t noDofsUsed = noDofs- 6 * (BCfixed.size()) - 3 * BCpinned.size();
			Eigen::SparseMatrix<double> globMatr(noDofsUsed, noDofsUsed);



			//----------------------------------------------------------------------------------------------------
			//Stifness Marix Row/Column Elimination from BCs & create Global K Matrix
			//----------------------------------------------------------------------------------------------------
			//TODO: Better solution without triplet duplication. (faster?)
			
			std::vector<Eigen::Triplet<double>> triplets_AfterBCs;
			std::vector<Eigen::Triplet<double>> triplets_AfterBCsALL;
			int row, col, val;
			int minusRow, minusCol;
			

			for (auto& triplet : globalK_triplets) {
				row = triplet.row(); col = triplet.col(); val = triplet.value();
				minusRow = 0; minusCol = 0; 
				bool isInBC = false;

				for (size_t fixBCid : BCfixed) {
					size_t BCstart = Nodes.get_byPos(fixBCid).matrixPos * 6;
					size_t BCend = Nodes.get_byPos(fixBCid).matrixPos * 6 + 5;
					const static size_t BCdofs = 6;

					if (row > BCend) minusRow += BCdofs;//if after BC, row is BC dofs less.(row elimination.)
					else if(row >= BCstart){ //not after BC end dof but after BC start dof means in BC dofs
						isInBC = true;
						break; // go to next, no matter the column
					}

					if (col > BCend) minusCol+= BCdofs;
					else if (col >= BCstart) {
						isInBC = true;
						break;
					}
					
				}
				if(!isInBC) triplets_AfterBCs.emplace_back(row-minusRow, col-minusCol, val);
			
			//#ifdef DEBUG_PRINTS
				if (isInBC) val = -101;
				triplets_AfterBCsALL.emplace_back(row, col, val);
			//#endif // DEBUG_PRINTS
			
			}

			globMatr.setFromTriplets(triplets_AfterBCs.begin(), triplets_AfterBCs.end());
			
			#ifdef DEBUG_PRINTS
				Eigen::SparseMatrix<double> asd(noDofs, noDofs);
				asd.setFromTriplets(triplets_AfterBCsALL.begin(), triplets_AfterBCsALL.end());
				std::cout << "\n------------------------------------\n Glob Matrix with noted removed Rows/Colsn\n " << Eigen::MatrixXd(asd) << "\n";
				std::cout << "\n------------------------------------\n Glob Matrix after row elimination\n " << Eigen::MatrixXd(globMatr) << "\n";
		
			#endif // DEBUG_PRINTS
			


			//----------------------------------------------------------------------------------------------------
			//F Vector Creation and Row Elimination
			//----------------------------------------------------------------------------------------------------
			
			F.resize(globMatr.rows());

			std::vector<Eigen::Triplet<double>> Ftriplets_AfterBCs;
			//make forces vector 
			for (auto& force : Forces) {
				size_t forceMatrixPos = Nodes.get_byPos(force.first).matrixPos*6;
				bool inBC = false;
				size_t minusPos = 0;
				for (size_t BCid : BCfixed) {
					size_t BCstart = Nodes.get_byPos(BCid).matrixPos*6;
					size_t BCend = Nodes.get_byPos(BCid).matrixPos*6 + 5;
					if (forceMatrixPos > BCend) minusPos += 6;
					else if (forceMatrixPos >= BCstart) {
						inBC = true;
						break;
					}

				}
				forceMatrixPos -= minusPos;
				if (inBC || forceMatrixPos >= F.rows()) continue;

				F.insert((Eigen::Index)forceMatrixPos) = force.second[0];
				F.insert((Eigen::Index)forceMatrixPos + 1) = force.second[1];
				F.insert((Eigen::Index)forceMatrixPos + 2) = force.second[2];
				F.insert((Eigen::Index)forceMatrixPos + 3) = force.second[3];
				F.insert((Eigen::Index)forceMatrixPos + 4) = force.second[4];
				F.insert((Eigen::Index)forceMatrixPos + 5) = force.second[5];

			}
			#ifdef DEBUG_PRINTS
				std::cout << "\n--------------------------------\nForce Vector\n" << F << "\n";
			#endif // DEBUG_PRINTS



			//----------------------------------------------------------------------------------------------------
			//Solving
			//----------------------------------------------------------------------------------------------------
			//TODO: Proper handling of failed solves.




			Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
			solver.compute(globMatr);
			
			if (solver.info() == Eigen::Success) {
				// Decomposition Succesfull
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
				Urender = U * RENDER_SCALING_FACTOR * scaleFactor;
#ifdef DEBUG_PRINTS
				//std::cout << "\n--------------------------------\nForce Vector\n" << Eigen::VectorXd(F) << "\n";
				//std::cout << "\n--------------------------------\Deflection Vector\n" << Eigen::VectorXd(U) << "\n";

#endif // DEBUG_PRINTS
				return;
			}

			std::cout << "Sparse LU decomposition failed\nConverting to Dense\n";
			Eigen::MatrixXd A(globMatr);// DO EXCEPTION HERE if this fails Also
			U = A.lu().solve(Eigen::VectorXd(F)).sparseView();
			std::cout << F << "\n" << A << "\n" << U;
			Urender.resize(U.rows());
			Urender = U * RENDER_SCALING_FACTOR * scaleFactor;
			solved = true;

		}

		const NodeContainer& getNodes() {
			return Nodes;
		}

		const std::vector<vBeam>& getElements() {
			return Elements;
		}

		Vector3 getDeflection(size_t nodeMatrixPos) {
			if (!solved) return Vector3Zero();

			size_t fixedNodesBefore = 0;
			for (size_t fixedId : BCfixed) {
				size_t BCmatrixPos = Nodes.get_byPos(fixedId).matrixPos;
				if (BCmatrixPos < nodeMatrixPos) fixedNodesBefore++;
				else if (BCmatrixPos == nodeMatrixPos) return Vector3Zero();
			}
			size_t afterBcId = nodeMatrixPos * 6 - fixedNodesBefore * 6;
			if (afterBcId >= U.rows()) return Vector3Zero(); //Free node, not in Stifness matrix


			return Vector3{ (float)U.coeff(afterBcId),(float)U.coeff(afterBcId + 1),(float)U.coeff(afterBcId + 2) };

		}

		Vector3 getDeflectionRender(size_t nodeMatrixPos) {//doesn't give rotations. 
			if (!solved) return Vector3Zero();

			if (nodeMatrixPos >= nodesPos_InMatrixOrder.size()) return Vector3Zero();//not in stifness matrix
			const Node& node = Nodes.get_byPos(nodesPos_InMatrixOrder[nodeMatrixPos]);

			if (node.free_flag) return Vector3Zero();

			size_t fixedNodesBefore = 0;
			for (size_t fixedId : BCfixed) {
				size_t BCmatrixPos = Nodes.get_byPos(fixedId).matrixPos;
				if (BCmatrixPos < nodeMatrixPos) fixedNodesBefore++;
				else if (BCmatrixPos == nodeMatrixPos) return Vector3Zero();
			}

			size_t afterBcId = nodeMatrixPos * 6 - fixedNodesBefore * 6;

			return Vector3{ (float)Urender.coeff(afterBcId) ,(float)Urender.coeff(afterBcId + 1) ,(float)Urender.coeff(afterBcId + 2) };

		}

		Vector3 getForce(size_t nodePos) {
			auto it = Forces.find(nodePos);

			if (it == Forces.end()) return Vector3Zero();
			return Vector3{ (float)it->second[0],(float)it->second[1],(float)it->second[2] };
		}

		void removeForce(size_t nodePos) {
			auto it = Forces.erase(nodePos);
			solved = false;

		}

		const std::map<size_t, std::array<double, 6>>& getForces() const  {
			return Forces;
		}

		void addBCfixed(size_t nodePos) {
			solved = false;
			if (Nodes.get_byPos(nodePos).free_flag) return;
			BCfixed.emplace(nodePos);
		}

		void removeBCfixed(size_t nodePos) {
			BCfixed.erase(nodePos);
			solved = false;

		}

		const std::set<size_t>& getBCfixed() {
			return BCfixed;
		}

		void printDeformed() {
			for (auto& node : Nodes) {
			
				Vector3 a;
				if (node.free_flag) {
					a = Vector3Zero();
					continue;
				}
				a = getDeflection(node.matrixPos);
				std::cout << a.x << " " << a.y << " " << a.z << " \n";
			}

		}

		void printU() {
			std::cout << Eigen::VectorXd{ U } << "\n";
		}

		void printF() {
			std::cout << Eigen::VectorXd{ F } << "\n";
		}

		bool isSolved() {
			return solved;
		}

		const std::map<size_t,Section>& getSections() {
			return Sections;
		}

		void modifySection(const size_t Id, double _Area, double _Modulus, double _G, double _Ixx, double _Iyy, double _Izz) {
			auto it = Sections.find(Id);

			if (it != Sections.end()) {
				solved = false;
				Section& sec = it->second;
				
				sec.Area = _Area;
				sec.Izz = (_Izz < 0) ? 100 : _Izz;
				sec.Ixx = (_Ixx < 0) ? 100 : _Ixx;
				sec.Iyy = (_Iyy < 0) ? _Izz : _Iyy;
				sec.Modulus = _Modulus;
				sec.G = _G;

				sec.EIz12 = 12 * _Modulus * _Izz;
				sec.EIy12 = 12 * _Modulus * _Iyy;
				sec.EIz6 = 6 * _Modulus * _Izz;
				sec.EIy6 = 6 * _Modulus * _Iyy;
				sec.EIz4 = 4 * _Modulus * _Izz;
				sec.EIy4 = 4 * _Modulus * _Iyy;
				sec.EIz2 = 2 * _Modulus * _Izz;
				sec.EIy2 = 2 * _Modulus * _Iyy;
				for (auto& element : Elements) {
					if (std::find(sec.inElements.begin(), sec.inElements.end(), element.getID()) != sec.inElements.end()) {
						element.reCalc(Nodes, sec);
					}
				}
			}
		}
	
		void clear() {
			U.data().clear();
			F.data().clear();


			Nodes.clear();

			Sections.clear();
			secIdNext = 0;

			Elements.clear();
			eId_Last = 0;

			nodesPos_InMatrixOrder.clear();

			BCpinned.clear();//UNUSED- and going to be unused.
			BCfixed.clear();
			Forces.clear();//node position to force. Position refers to All nodes, taking into account the deleted stuff.

			noDofs = 0;

			solved = false;
			Urender.data().clear();
			scaleFactor = 1; 
		}
};
};



