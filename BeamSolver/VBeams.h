#pragma once

#include <vector>
#include <map>
#include <array>
#include <math.h>
#include <memory>
#include <Eigen/SparseLU>
#include <Eigen/Dense>
#include<Eigen/SparseCholesky>	
//#include<Eigen/SparseLU>
//#include<Eigen/SparseQR>
#include<Eigen/SparseQR>
//Make more efficient triplet changes. For now it is a vector that gets reevaluated at analyse.
//Make all stifness matrices only triangular stuff

//double error tolerance
#define ERR_TOLERANCE 0.000000001



namespace Beams {
	struct Node {
		double x, y, z;
		size_t id;
		bool free_flag = true;
		std::vector<size_t> inElements; //UNUSED
		Node(double x_, double  y_, double  z_,size_t id_) {
			x = x_;
			y = y_;
			z = z_;
			id = id_;
		}
	};


	using Coord3d = std::array<double, 3>;
	using Trip = Eigen::Triplet<double> ;

	Eigen::SparseMatrix<double> A;
	
	class vBeam {
		size_t id;
		std::shared_ptr<Node> node1;
		std::shared_ptr<Node> node2;
		std::shared_ptr<Node> node3; //cross section  orietnation



		Eigen::SparseMatrix<double> localBmatrix;

		double Len;
		//make properties for common stifness matrices
		double Area;
		double Izz;
		double Iyy;
		double Modulus = 210000;
	

		void calc_BMatrix() {
			double Lsq = Len * Len;
			double sixLen = 6 * Len;
			size_t start = id * 12;
			double EIz12 = 12 * Modulus * Izz / pow(Len, 3);
			double EIy12 = 12 * Modulus * Iyy / pow(Len, 3);
			double EIz6 = 6 * Modulus * Izz / pow(Len, 2);
			double EIy6 = 6 * Modulus * Iyy / pow(Len, 2);
			double EIz4 = 4 * Modulus * Izz / Len;
			double EIy4 = 4 * Modulus * Iyy / Len;
			double EIz2 = 2 * Modulus * Izz / Len;
			double EIy2 = 2 * Modulus * Iyy / Len;
			static std::vector<Eigen::Triplet<double>> locStiffness_triplets(47); //make this joined with global tiplets for less memory




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

		void calc_Len() {
			Len = std::pow((node2->x -node1->x), 2) + std::pow((node2->y -node1->y), 2) + std::pow((node2->z -node1->z), 2);
			Len = std::sqrt(Len);
		};
	
		

	public:
		vBeam(int id_, Node& N1, Node& N2, Node& N3, double A, double momentY, double momentZ = NULL) {
			localBmatrix.resize(12, 12);
			node1 = std::make_shared<Node>(N1);
			node2 = std::make_shared<Node>(N2);
			node3 = std::make_shared<Node>(N3);
			N1.free_flag = false;
			N2.free_flag = false;
			

			id = id_;
			calc_Len();
			Area = A;
			Iyy = momentY;
			if (momentZ == NULL) { Izz = Iyy; }
			else Izz = momentZ;
			calc_BMatrix();
		};
	
		void reCalc() {
			calc_Len();
			calc_BMatrix();
		}

		void LocalMatrix2GlobalTriplets(std::vector<Eigen::Triplet<double>>& globalTriplets) {
			//using direction cosine matrix because reasons and it works also and no gimbal lock or whatever this stuff is 
			static const Eigen::Vector3d xAxis(1, 0, 0);
			static const Eigen::Vector3d yAxis(0, 1, 0);
			static const Eigen::Vector3d zAxis(0, 0, 1);


			Eigen::Vector3d localX_unit(node2->x - node1->x, node2->y - node1->y, node2->z - node1->z);
			Eigen::Vector3d localY_unit(node3->x - node1->x, node3->y - node1->y, node3->z - node1->z);//a vector in XY local plane
			Eigen::Vector3d localZ_unit;



			localX_unit = localX_unit / localX_unit.norm();
			localY_unit = localY_unit - localY_unit.dot(localX_unit) * localX_unit;// Y local  vector
			localY_unit = localY_unit / localY_unit.norm(); //Y local unit Vector

			
			localZ_unit = localX_unit.cross(localY_unit);
			//if (std::abs(localX_unit.dot(localY_unit)) > ERR_TOLERANCE) {
				//localY_unit == localZ_unit.cross(localX_unit);
			//}


			std::cout << "BeamVectors -> \n X:\n" << localX_unit << "  \nY:\n" << localY_unit << "  \nZ:\n" << localZ_unit << " \n ";


			std::vector<Eigen::Triplet<double>> dirCosineMat_triplets(37);


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
					Eigen::SparseMatrix<double> test(3, 3);
					test.setFromTriplets(dirCosineMat_triplets.begin(), dirCosineMat_triplets.end());
				}*/
			}


			Eigen::SparseMatrix<double> cosMatrix(12, 12);
			cosMatrix.setFromTriplets(dirCosineMat_triplets.begin(), dirCosineMat_triplets.end());

			Eigen::SparseMatrix<double> globalB = cosMatrix * localBmatrix * (cosMatrix.transpose());


			//THIS IS WRONG! IT TAKES INTO ACCOUNT  THE ELEMENT ID WHEN IT SHOULD TAKE THE NODE ID TO INPUT TO GLOBAL K
			for (int k = 0; k < globalB.outerSize(); ++k)
			{
				for (Eigen::SparseMatrix<double>::InnerIterator it(globalB, k); it; ++it)
				{
					
					
					Eigen::Index row = it.row(); // row index
					Eigen::Index col = it.col(); // col index (here it is equal to k)
					double val = it.value();
					
					globalTriplets.emplace_back(row, col, val);
				}
			}





		}
	};


	

	class Model {
		std::vector<Eigen::Triplet<double>> globalK_triplets;
		Eigen::VectorXd U , F;
		

		std::vector<Node> Nodes;
		std::vector<vBeam> Elements;


		std::vector<size_t> BCpinned;
		std::vector<size_t> BCfixed;
		

	public:
		void oneElementTest() {
			Nodes.emplace_back(0, 0, 0, 0);
			Nodes.emplace_back(1000, 0, 0, 1);
			Nodes.emplace_back(0.2, 1, 0, 2);
			Nodes.emplace_back(-1000, 0, 0, 3);

			Elements.emplace_back(0, Nodes[0], Nodes[1], Nodes[2], 100, 100);
			Elements.emplace_back(1, Nodes[0], Nodes[3], Nodes[2], 100, 100);
			BCfixed.push_back(0);

			F.resize(12);
			F << 0, 100, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0;

		};

		void solve() {
			//find problem DOFs and Make unused nodes last. Initialize globMatrix.
			size_t noDofs = 0;
			size_t foundUnused = 0;
			for (auto& node : Nodes) {
				std::cout << node.free_flag << std::endl;
				if (!node.free_flag) {
					node.id = node.id - foundUnused;
					noDofs++;
				}
				else {
					node.id = Nodes.size() - 1 - foundUnused;//unused nodes go above other unused nodes in the end 
					foundUnused++;

				}
			}
			noDofs = 6 * (noDofs - BCfixed.size()) - 3 * BCpinned.size();
			Eigen::SparseMatrix<double> globMatr(noDofs, noDofs);


			//populate global stigness matrix triplets -- (THE FOLLOWING IS WRONG (currently). 2GlobalTriplets must be corrected.) Helper Elements (not beam nodes) must have last IDs for correct placement in the matrix
			for (auto& element : Elements) {
				element.LocalMatrix2GlobalTriplets(globalK_triplets);
			}

			
			Eigen::SparseMatrix<double> testGlobAll(50, 50);
			testGlobAll.setFromTriplets(globalK_triplets.begin(),globalK_triplets.end());
			std::cout << testGlobAll<<std::endl; //SHOW GLOBAL MATRIX


			//remove rows and columns from BCs
			//Unfortunately I Have to duplicate
			std::vector<Trip> triplets_AfterBCs;
			int row,col,val;
			for (auto& triplet : globalK_triplets) {
				row = triplet.row(); col = triplet.col(); val = triplet.value();
				
				//na to allaksw kapws. na kanw map ta ij twn triplets ston pinaka kapws? na kserw poia tha valw kai poia oxi. 

				//mesa sto Node pou vgaw eimai an eimai px [ node id= 2 ]-> 2*6<=row<=2*6+5	
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
			
			//solve
			/*A << 21000, -20998, -20998, 0, 0, 0,
				-20998, 20997, 20997, 0, 0, 0,
				-20998, 20997, 20997, 0, 0, 0,
				0, 0, 0, 21000, -20998, -20998,
				0, 0, 0, -20998, 21006, 21006,
				0, 0, 0, -20998, 21006, 21006;
			*/ 
			Eigen::SparseMatrix<double> S(6, 6);
			

			Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
			solver.compute(globMatr);
			//std::cout << A << globMatr;
			//std::cout << U<<std::endl;
			//return;
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

				std::cout << U;
				return;
			}
			std::cout << "Sparse LU decomposition failed\nConverting to Dense\n";
			Eigen::MatrixXd A(globMatr);
			U = A.lu().solve(F);
			std::cout << F <<"\n" << A << "\n" << U;


		}
	};
}



