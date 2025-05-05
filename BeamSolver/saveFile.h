#pragma once
#include "VBeams.h" 
#include <unordered_map>
#include <fstream>

//std::ofstream out(fname, std::ios_base::binary);
namespace Saving {
	template <typename T>
	static inline void saveVar(std::ofstream& out,  T var) {
		out.write(reinterpret_cast<char*>(&var), sizeof(T));
	}

	template <typename T>
	static inline T readVar(std::ifstream& in) {
		T var;
		in.read(reinterpret_cast<char*>(&var), sizeof(T));
		return var;
	}


	static inline void saveNode(std::ofstream& out, const Beams::Node& node) {
		saveVar(out, node.x);
		saveVar(out, node.y);
		saveVar(out, node.z);
	}

	static inline void readNode(std::ifstream& in, Beams::Model& model) {
		float x = readVar<double>(in);
		float y = readVar<double>(in);
		float z = readVar<double>(in);

		model.addNode(Vector3{ x,y,z });
	}


	static inline void saveSection(std::ofstream& out, const Beams::Section& sec) {
		saveVar(out, sec.Area);
		saveVar(out, sec.Modulus);
		saveVar(out, sec.G);
		saveVar(out, sec.Ixx);
		saveVar(out, sec.Iyy);
		saveVar(out, sec.Izz);
	}

	static inline void readSection(std::ifstream& in, Beams::Model& model) {
		float Area = readVar<double>(in);
		float Modulus = readVar<double>(in);
		float G = readVar<double>(in);
		float Ixx = readVar<double>(in);
		float Iyy = readVar<double>(in);
		float Izz = readVar<double>(in);
		model.addSection(Area, Modulus, G, Ixx, Iyy, Izz);
	}




	static inline void saveElement(std::ofstream& out, const Beams::vBeam& el, std::unordered_map<size_t, size_t>& nodeOrder, std::unordered_map<size_t, size_t>& sectionOrder) {
		saveVar(out, nodeOrder[el.node1Pos]);
		saveVar(out, nodeOrder[el.node2Pos]);
		saveVar(out, nodeOrder[el.node3Pos]);
		saveVar(out, sectionOrder[el.getSectionId()]);
	}


	static inline void readElement(std::ifstream& in, Beams::Model& model) {
		size_t node1 = readVar<size_t>(in);
		size_t node2 = readVar<size_t>(in);
		size_t node3 = readVar<size_t>(in);
		size_t getSe = readVar<size_t>(in);
		model.addElement(node1,node2,node3,getSe);
	}


	static inline void saveForce(std::ofstream& out, std::array<double, 6> force, size_t nodeOrderPos) {
		saveVar(out, nodeOrderPos);
		saveVar(out, (float)force[0]);
		saveVar(out, (float)force[1]);
		saveVar(out, (float)force[2]);
		saveVar(out, (float)force[3]);
		saveVar(out, (float)force[4]);
		saveVar(out, (float)force[5]);
	};

	static inline void readForce(std::ifstream& in, Beams::Model& model) {
		size_t nodePos = readVar<size_t>(in);
		model.addForce(nodePos, 0, readVar<float>(in));
		model.addForce(nodePos, 1, readVar<float>(in));
		model.addForce(nodePos, 2, readVar<float>(in));
		model.addForce(nodePos, 3, readVar<float>(in));
		model.addForce(nodePos, 4, readVar<float>(in));
		model.addForce(nodePos, 5, readVar<float>(in));
	}




	void saveModel(Beams::Model& model, std::string fname) {
		std::ofstream out(fname, std::ios_base::binary);

		const Beams::NodeContainer& nodes = model.getNodes();
		saveVar(out, nodes.size());
		std::unordered_map<size_t, size_t> nodePos_to_SaveOrder;
		size_t counter = 0;
		for (auto& node : nodes) {
			saveNode(out, node);
			nodePos_to_SaveOrder[node.pos] = counter;
			counter++;
		}

		const std::map<size_t, Beams::Section>& sections = model.getSections();
		saveVar(out, sections.size());
		std::unordered_map<size_t, size_t> secId_to_SaveOrder;
		counter = 0;
		for (auto& sec : sections) {
			saveSection(out, sec.second);
			secId_to_SaveOrder[sec.first] = counter;
			counter++;
		}

		const std::vector<Beams::vBeam>& elements = model.getElements();
		saveVar(out, elements.size());
		for (auto& element : elements) {
			saveElement(out, element, nodePos_to_SaveOrder, secId_to_SaveOrder);
		}

		const auto& forces = model.getForces();
		saveVar(out, forces.size());
		for (auto& forcePair : forces) {
			saveForce(out, forcePair.second, nodePos_to_SaveOrder[forcePair.first]);
		}

		const auto& BCs = model.getBCfixed();
		saveVar(out, BCs.size());
		for (auto& BC : BCs) {
			saveVar(out, nodePos_to_SaveOrder[BC]);
		}

		out.close();
	}

	void loadModel(Beams::Model& model, std::string fname) {
		model.clear();
		std::ifstream in(fname, std::ios_base::binary);

		size_t nodeSize = readVar<size_t>(in);
		for (size_t i = 0; i < nodeSize; i++) {
			readNode(in, model);
		}

		size_t sectionSize = readVar<size_t>(in);
		for (size_t i = 0; i < sectionSize; i++) {
			readSection(in, model);
		}

		size_t elSize = readVar<size_t>(in);
		for (size_t i = 0; i < elSize; i++) {
			readElement(in, model);
		}

		size_t forceSize = readVar<size_t>(in);
		for (size_t i = 0; i < forceSize; i++) {
			readForce(in, model);
		}

		size_t BCSize = readVar<size_t>(in);
		for (size_t i = 0; i < BCSize; i++) {
			model.addBCfixed(readVar<size_t>(in));
		}

	}

}