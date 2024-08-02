#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include "half.h"

using ulong = unsigned int;
using ushort = unsigned short;
using byte = unsigned char;
using half_float::half;

#define read_ulong(dst, hndl) hndl.read((char*)&dst, sizeof(ulong))
#define error(msg) MessageBoxA(NULL, msg, "ERROR", MB_ICONERROR);\
exit(-1)

ulong sub_file_offset = 0;
template<typename T>
T read_virtual_file(byte* sub_file) {
	T tmp{};
	tmp = *(T*)(sub_file + sub_file_offset);
	sub_file_offset += sizeof(T);
	return tmp;
}

namespace Types {
	struct Bone {
		ulong boneID;
		ushort boneIndex;
		ushort boneChild;
		float a11, a12, a13, a14,
			  a21, a22, a23, a24;
	};

	struct Vertex16 {
		half x, y, z;
		ushort checkVert; // 15360
		byte weight1, weight2, weight3, weight4, bone1, bone2, bone3, bone4;
	};

	struct Vertex24 {
		float x, y, z, checkVert; // 1.0
		byte weight1, weight2, weight3, weight4, bone1, bone2, bone3, bone4;
	};

	struct Face {
		ushort index1, index2, index3;
	};
}

struct Header {
	char magic[4]; // or ulong
	ulong version;

	ulong meshDescSectionSize;
	ulong meshInfoSectionSize;
	ulong meshDataSectionSize;
};

struct MeshDescSection {
	ulong sectionID; // 0xEBAEC3FA
	ulong matIndexCount;
	ulong sectionDataCount;

	long* unkMatIndices = nullptr;
	long* meshMatIndices = nullptr;

	ulong* vertSectionSizes = nullptr;
	ulong* faceSectionSizes = nullptr;
	ulong* skinSectionSizes = nullptr;

	MeshDescSection() = default;

	MeshDescSection(std::ifstream& mesh) {
		populate(mesh);
	}


	void populate(std::ifstream& mesh) {
		mesh.read((char*)this, sizeof(ulong) * 3); // parse first three attributes
		
		unkMatIndices = new long[matIndexCount];
		mesh.read((char*)unkMatIndices, sizeof(long) * matIndexCount);

		meshMatIndices = new long[matIndexCount];
		mesh.read((char*)meshMatIndices, sizeof(long) * matIndexCount);

		vertSectionSizes = new ulong[sectionDataCount];
		mesh.read((char*)vertSectionSizes, sizeof(ulong) * sectionDataCount);

		faceSectionSizes = new ulong[sectionDataCount];
		mesh.read((char*)faceSectionSizes, sizeof(ulong) * sectionDataCount);

		skinSectionSizes = new ulong[sectionDataCount];
		mesh.read((char*)skinSectionSizes, sizeof(ulong) * sectionDataCount);
	}

	~MeshDescSection() {
		delete[] unkMatIndices, meshMatIndices, vertSectionSizes, faceSectionSizes, 
			skinSectionSizes;
	}
};

struct MeshInfoSection {
	struct MatAssignment {
		long model;
		long mesh;
		long mat;
		long unk;
	};

	struct MeshInfo {
		ulong vertSecSize;
		ulong faceSecSize;
		ulong skinSecSize;
		ulong subMeshCount;
		
		ushort faceCount[16];
	};
	
	struct SubMeshInfo {
		enum UVFormat : ulong{
			UVArray52,
			UVArray56,
			UVArray48,
			UVArray44
		};
		
		ulong offset;
		ulong vertStart;
		ulong vertCount;
		ulong faceIndicesStart;
		ulong faceIndicesCount;
		ulong skinStart;
		ulong skinCount;
		ulong unknown7;
		ulong unknown8;

		UVFormat uvFormat;
	};

	struct ModelLodSettings {
		ulong unknownCount;
		byte* unknownByte1 = nullptr;
		byte* unknownByte2 = nullptr;
		byte* unknownByte3 = nullptr;
		byte* unknownByte4 = nullptr;

		void populate(std::ifstream& mesh) {
			mesh.read((char*)&unknownCount, sizeof(ulong));

			unknownByte1 = new byte[unknownCount];
			mesh.read((char*)unknownByte1, sizeof(byte) * unknownCount);

			unknownByte2 = new byte[unknownCount];
			mesh.read((char*)unknownByte2, sizeof(byte) * unknownCount);

			unknownByte3 = new byte[unknownCount];
			mesh.read((char*)unknownByte3, sizeof(byte) * unknownCount);

			unknownByte4 = new byte[unknownCount];
			mesh.read((char*)unknownByte4, sizeof(byte) * unknownCount);
		}

		~ModelLodSettings() {
			delete[] unknownByte1, unknownByte2, unknownByte3, unknownByte4;
		}
	};

	ulong sectionID; // 0x1A1541BC

	ulong modelCount;
	ulong matAssignmentCount;
	ulong meshCount;
	ulong subMeshCount;

	ulong unknownULong1;

	ulong lodSettingsCount;
	ulong meshLodCount;
	ulong shadowLodCount;

	ulong modelLodCount;
	ulong ulongReadCount;

	float unknownFloat;

	float unknown1[4];

	ulong boneCount;
	ulong boneSectionSize;

	ulong* modelIDs;

	MatAssignment* matAssignments = nullptr;
	MeshInfo* meshInfos = nullptr;
	SubMeshInfo* subMeshInfos = nullptr;

	ulong lodSectionID; // 0x8E3E068E
	ulong* lodSettings = nullptr;
	float* lodThresholds = nullptr;
	ulong* lodConnections = nullptr;

	ulong modelLodSectionID; // 0x37D749A6

	ModelLodSettings* modelLodSettings = nullptr;

	ulong boneSectionID; // 0x93D9A424
	Types::Bone* bones = nullptr;

	MeshInfoSection() = default;

	MeshInfoSection(std::ifstream& mesh) {
		populate(mesh);
	}

	void populate(std::ifstream& mesh) {
		mesh.read((char*)this, 72); // parse until modelIDs array

		modelIDs = new ulong[modelCount];
		mesh.read((char*)modelIDs, sizeof(ulong) * modelCount);

		matAssignments = new MatAssignment[matAssignmentCount];
		mesh.read((char*)matAssignments, sizeof(MatAssignment) * matAssignmentCount);

		meshInfos = new MeshInfo[meshCount];
		mesh.read((char*)meshInfos, sizeof(MeshInfo) * meshCount);

		subMeshInfos = new SubMeshInfo[subMeshCount];
		mesh.read((char*)subMeshInfos, sizeof(SubMeshInfo) * subMeshCount);

		read_ulong(lodSectionID, mesh);

		lodSettings = new ulong[lodSettingsCount];
		mesh.read((char*)lodSettings, sizeof(ulong) * lodSettingsCount);

		lodThresholds = new float[meshLodCount + shadowLodCount];
		mesh.read((char*)lodThresholds, sizeof(float) * (meshLodCount + shadowLodCount));

		lodConnections = new ulong[meshLodCount + shadowLodCount];
		mesh.read((char*)lodConnections, sizeof(ulong) * (meshLodCount + shadowLodCount));

		read_ulong(modelLodSectionID, mesh);

		modelLodSettings = new ModelLodSettings[modelLodCount];
		for (int i = 0; i < modelLodCount; ++i)
			modelLodSettings[i].populate(mesh);

		read_ulong(boneSectionID, mesh);
		
		bones = new Types::Bone[boneCount];
		mesh.read((char*)bones, sizeof(Types::Bone) * boneCount);
	}

	~MeshInfoSection() {
		delete[] modelIDs, matAssignments, meshInfos, subMeshInfos, lodSettings,
			lodSettings, lodThresholds, lodConnections, modelLodSettings, bones;
	}
};

struct MeshDataSection {

	ulong sectionID; // 0x95DBDB69
	
	std::vector<byte*> vertDataSections;
	std::vector<byte*> faceDataSections;
	std::vector<byte*> boneDataSections;

	MeshDataSection() = default;

	MeshDataSection(std::ifstream& mesh, MeshDescSection& meshDescSection) {
		populate(mesh, meshDescSection);
	}

	void populate(std::ifstream& mesh, MeshDescSection& meshDescSection) {
		read_ulong(sectionID, mesh);

		for (int i = 0; i < meshDescSection.sectionDataCount; ++i) {
			byte* tmp_buf = new byte[meshDescSection.vertSectionSizes[i]];
			mesh.read((char*)tmp_buf, meshDescSection.vertSectionSizes[i]);
			vertDataSections.push_back(tmp_buf);
		}
		
		for (int i = 0; i < meshDescSection.sectionDataCount; ++i) {
			byte* tmp_buf = new byte[meshDescSection.faceSectionSizes[i]];
			mesh.read((char*)tmp_buf, meshDescSection.faceSectionSizes[i]);
			faceDataSections.push_back(tmp_buf);
		}

		for (int i = 0; i < meshDescSection.sectionDataCount; ++i) {
			byte* tmp_buf = new byte[meshDescSection.skinSectionSizes[i]];
			mesh.read((char*)tmp_buf, meshDescSection.skinSectionSizes[i]);
			boneDataSections.push_back(tmp_buf);
		}
	}

	~MeshDataSection() {
		for (auto& iter : vertDataSections)
			delete[] iter;
		for (auto& iter : faceDataSections)
			delete[] iter;
		for (auto& iter : boneDataSections)
			delete[] iter;
	}
};

template<typename VERTEX> 
	requires std::is_same_v<VERTEX, Types::Vertex16> || std::is_same_v<VERTEX, Types::Vertex24>
struct MeshBuffer {
	std::vector<std::vector<VERTEX>> subMeshesVertexContainer;
	std::vector<std::vector<byte>> subMeshesUVContainer;
	std::vector<std::vector<Types::Face>> subMeshesFaceContainer;
	std::vector<std::vector<byte>> subMeshSkinContainer;

	static bool checkVert(VERTEX vertex) {
		if constexpr (std::is_same_v<VERTEX, Types::Vertex16>) {
			Types::Vertex16* vertex16 = &vertex;
			if (vertex16->checkVert == 15360)
				return true;
			return false;
		}
		else {
			Types::Vertex24* vertex24 = &vertex;
			if (vertex24->checkVert == 1.0f)
				return true;
			return false;
		}
	}

	void populate(MeshInfoSection& meshInfoSection, MeshDataSection& meshDataSection, int meshNumber) {
		for (int subMesh = 0; subMesh < meshInfoSection.meshInfos[meshNumber].subMeshCount; ++subMesh) {
			VERTEX* v = (VERTEX*)meshDataSection.vertDataSections[meshNumber];
			subMeshesVertexContainer.push_back(std::vector<VERTEX>{}); // To allocate the actual vertex container
			for (int vertexCounter = 0; vertexCounter < meshInfoSection.subMeshInfos[subMesh].vertCount; ++vertexCounter) {
				if (!checkVert(v[vertexCounter])) {
					error("CheckVert didnt match!");
				}
				subMeshesVertexContainer[subMesh].push_back(v[vertexCounter]);
			}
			// UV loop here...
			Types::Face* f = (Types::Face*)meshDataSection.faceDataSections[meshNumber];
			subMeshesFaceContainer.push_back(std::vector<Types::Face>{});
			for (int faceCounter = 0; faceCounter < meshInfoSection.subMeshInfos[subMesh].faceIndicesCount / 3; ++faceCounter) {
				subMeshesFaceContainer[subMesh].push_back(f[faceCounter]);
			}
			// Skin loop here...
		}
	}

	void to_obj(const std::string& file) {
		std::ofstream out(file);
		for (auto& iter : subMeshesVertexContainer[0]) {
			out << "v " << iter.x << " " << iter.y << " " << iter.z << "\n";
		}
		for (auto& iter : subMeshesFaceContainer[0]) {
			out << "f " << iter.index1 + 1 << "/ " << iter.index2 + 1 << "/ " << iter.index3 + 1 << "/\n";
		}
	}
};

int main() {
	std::ifstream mesh("F:\\Sow\\unpacked-game\\characters\\modelweapons\\player\\1h_blunt\\celebrimborhammer\\player_celebrimborhammer.mesh", std::ios::binary);
	
	Header header{};
	mesh.read((char*)&header, sizeof(Header)); // No pointers -> directly parsed

	MeshDescSection meshDescSection(mesh);
	MeshInfoSection meshInfoSection(mesh);
	MeshDataSection meshDataSection(mesh, meshDescSection);
	
	MeshBuffer<Types::Vertex16> meshBuffer;

	for (int mesh = 0; mesh < meshInfoSection.meshCount; ++mesh) {
		meshBuffer.populate(meshInfoSection, meshDataSection, mesh);
	}
	meshBuffer.to_obj("F:\\out2.obj");

}