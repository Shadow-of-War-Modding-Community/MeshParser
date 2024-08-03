#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <optional>
#include <memory>
#include "half.h"

namespace MESH_UNPACKER {
	namespace INTERNAL {
		using ulong = unsigned int;
		using ushort = unsigned short;
		using byte = unsigned char;
		using half_float::half;

#define read_ulong(dst, hndl) hndl.read((char*)&dst, sizeof(ulong))
#define error(msg) MessageBoxA(NULL, msg, "MeshLoader-ERROR", MB_ICONERROR);\
exit(-1)

		template<typename T>
			requires std::is_integral_v<T> || std::is_floating_point_v<T>
		inline void expect(T var, T val) {
			if (var != val) {
				error(("Variable contains unexpected value.\nExpected: " + std::to_string(val) + "\nFound: " + std::to_string(var)).c_str());
			}
		}
		namespace MESH {
			namespace TYPES {
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
				union {
					char magic_c[4]; // MMSH
					ulong magic_u; // 0x48534D4D
				};
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

					expect(sectionID, 0xEBAEC3FAu);

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
					enum UVFormat : ulong {
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
				TYPES::Bone* bones = nullptr;

				MeshInfoSection() = default;

				MeshInfoSection(std::ifstream& mesh) {
					populate(mesh);
				}

				void populate(std::ifstream& mesh) {
					mesh.read((char*)this, 72); // parse until modelIDs array

					expect(sectionID, 0x1A1541BCu);

					modelIDs = new ulong[modelCount];
					mesh.read((char*)modelIDs, sizeof(ulong) * modelCount);

					matAssignments = new MatAssignment[matAssignmentCount];
					mesh.read((char*)matAssignments, sizeof(MatAssignment) * matAssignmentCount);

					meshInfos = new MeshInfo[meshCount];
					mesh.read((char*)meshInfos, sizeof(MeshInfo) * meshCount);

					subMeshInfos = new SubMeshInfo[subMeshCount];
					mesh.read((char*)subMeshInfos, sizeof(SubMeshInfo) * subMeshCount);

					read_ulong(lodSectionID, mesh);

					expect(lodSectionID, 0x8E3E068Eu);

					lodSettings = new ulong[lodSettingsCount];
					mesh.read((char*)lodSettings, sizeof(ulong) * lodSettingsCount);

					lodThresholds = new float[meshLodCount + shadowLodCount];
					mesh.read((char*)lodThresholds, sizeof(float) * (meshLodCount + shadowLodCount));

					lodConnections = new ulong[meshLodCount + shadowLodCount];
					mesh.read((char*)lodConnections, sizeof(ulong) * (meshLodCount + shadowLodCount));

					read_ulong(modelLodSectionID, mesh);

					expect(modelLodSectionID, 0x37D749A6u);

					modelLodSettings = new ModelLodSettings[modelLodCount];
					for (int i = 0; i < modelLodCount; ++i)
						modelLodSettings[i].populate(mesh);

					read_ulong(boneSectionID, mesh);

					expect(boneSectionID, 0x93D9A424u);

					bones = new TYPES::Bone[boneCount];
					mesh.read((char*)bones, sizeof(TYPES::Bone) * boneCount);
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
				std::vector<byte*> skinDataSections;

				MeshDataSection() = default;

				MeshDataSection(std::ifstream& mesh, MeshDescSection& meshDescSection) {
					populate(mesh, meshDescSection);
				}

				void populate(std::ifstream& mesh, MeshDescSection& meshDescSection) {
					read_ulong(sectionID, mesh);

					expect(sectionID, 0x95DBDB69u);

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
						skinDataSections.push_back(tmp_buf);
					}
				}

				~MeshDataSection() {
					for (auto& iter : vertDataSections)
						delete[] iter;
					for (auto& iter : faceDataSections)
						delete[] iter;
					for (auto& iter : skinDataSections)
						delete[] iter;
				}
			};

			template<typename VERTEX>
				requires std::is_same_v<VERTEX, TYPES::Vertex16> || std::is_same_v<VERTEX, TYPES::Vertex24>
			class MeshBuffer {
				static void checkVert(VERTEX vertex) {
					if constexpr (std::is_same_v<VERTEX, TYPES::Vertex16>)
						expect(((TYPES::Vertex16*)&vertex)->checkVert, ushort(15360));
					else
						expect(((TYPES::Vertex24*)&vertex)->checkVert, 1.0f);
				}

			public:
				std::vector<std::vector<VERTEX>> subMeshesVertexContainer;
				std::vector<std::vector<byte>> subMeshesUVContainer;
				std::vector<std::vector<TYPES::Face>> subMeshesFaceContainer;
				std::vector<std::vector<byte>> subMeshesSkinContainer;

				void populate(MeshInfoSection& meshInfoSection, MeshDataSection& meshDataSection, int meshNumber) {
					for (int subMesh = 0; subMesh < meshInfoSection.meshInfos[meshNumber].subMeshCount; ++subMesh) {
						VERTEX* v = (VERTEX*)meshDataSection.vertDataSections[meshNumber];
						subMeshesVertexContainer.push_back(std::vector<VERTEX>{}); // To allocate the actual vertex container
						for (int vertexCounter = 0; vertexCounter < meshInfoSection.subMeshInfos[subMesh].vertCount; ++vertexCounter) {
							checkVert(v[vertexCounter]);
							subMeshesVertexContainer[subMesh].push_back(v[vertexCounter]);
						}
						// UV loop here...
						TYPES::Face* f = (TYPES::Face*)meshDataSection.faceDataSections[meshNumber];
						subMeshesFaceContainer.push_back(std::vector<TYPES::Face>{});
						for (int faceCounter = 0; faceCounter < meshInfoSection.subMeshInfos[subMesh].faceIndicesCount / 3; ++faceCounter) {
							subMeshesFaceContainer[subMesh].push_back(f[faceCounter]);
						}
						byte* s = meshDataSection.skinDataSections[meshNumber];
						subMeshesSkinContainer.push_back(std::vector<byte>{});
						for (int skinCounter = 0; skinCounter < meshInfoSection.subMeshInfos[subMesh].skinCount; ++skinCounter) {
							subMeshesSkinContainer[subMesh].push_back(s[skinCounter]);
						}
					}
				}

				void to_obj(const std::string& file) {
					//std::ofstream out(file);
					//for (auto& iter : subMeshesVertexContainer[0]) {
					//	out << "v " << iter.x << " " << iter.y << " " << iter.z << "\n";
					//}
					//for (auto& iter : subMeshesFaceContainer[0]) {
					//	out << "f " << iter.index1 + 1 << "/ " << iter.index2 + 1 << "/ " << iter.index3 + 1 << "/\n";
					//}
				}
			};
		}
		namespace SKEL {
			namespace TYPES {
#pragma pack(push, 1)
				struct Bone {
					ulong boneNameOffset;
					bool boneActive;
					float c21,c22, c23, c11, c12, c13, c14;
					ulong childCount;
					std::string boneName;

					void populate(std::ifstream& skel, std::string& name) {
						skel.read((char*)this, 40);
						boneName = name;
					}
				};
#pragma pack(pop)
			}

			struct Header {
#pragma pack(push, 1) // Tells the compiler to align the members in memory without any padding. Else the struct will be read wrong!
				struct UnknownMeshDataBlock {
					bool unknownBool1;
					bool unknownBool2;
					float lodThreshold;
					ulong unknownUInt;
					long unknownInt;
				};
#pragma pack(pop)
				struct UnknownMeshData {
					ulong unknownMeshDataBlockCount;
					UnknownMeshDataBlock* unknownMeshDataBlock = nullptr;

					void populate(std::ifstream& skel) {
						read_ulong(unknownMeshDataBlockCount, skel);

						unknownMeshDataBlock = new UnknownMeshDataBlock[unknownMeshDataBlockCount];
						skel.read((char*)unknownMeshDataBlock, sizeof(UnknownMeshDataBlock) * unknownMeshDataBlockCount);
					}

					~UnknownMeshData() {
						delete[] unknownMeshDataBlock;
					}
				};
				
				union {
					char magic_c[4]; // SKEL
					ulong magic_u;
				};
				ulong version;

				char unk1[4]; // ID?

				ulong boneNameSecSize;

				ulong unk2;
				ulong boneCount1;

				char unk3[4]; // ID?
				long unk4; // Animation count?

				ulong meshCount; // Unique Mesh count?
				ulong unk6; // FX count?
				ulong boneCount2;

				long unk7;
				long unk8;
				long unk9;
				long unk10;
				long unk11; // ID?

				ulong unk12;

				float unk13; // SectionID?
				float unk14[3]; // Transforms?
				float unk15[3]; // Transforms?

				UnknownMeshData* unknownMeshData = nullptr;

				Header() = default;

				Header(std::ifstream& skel) {
					populate(skel);
				}

				void populate(std::ifstream& skel) {
					skel.read((char*)this, 96);

					unknownMeshData = new UnknownMeshData[meshCount];
					for (int i = 0; i < meshCount; ++i)
						unknownMeshData[i].populate(skel);
				}

				~Header() {
					delete[] unknownMeshData;
				}
			};

			struct BoneSection {
				ulong sectionID; // 0xDCDCFF00

				std::vector<std::string> boneNames;

				TYPES::Bone* bones = nullptr;

				void populate(std::ifstream& skel, Header& header) {
					
					auto getNextString = [&]() -> std::string {
						std::string readString;
						for (;;) {
							char tmp;
							skel.read(&tmp, 1);
							if (tmp == '\0') {
								readString += tmp;
								return readString;
							}
							readString += tmp;
						}
					};
					
					auto backup = skel.tellg();

					read_ulong(sectionID, skel);

					expect(sectionID, 0xDCDCFF00);

					char useless;
					skel.read(&useless, 1);

					for (int boneCounter = 0; boneCounter < header.boneCount1; ++boneCounter)
						boneNames.push_back(getNextString());

					skel.seekg((ulong)backup + header.boneNameSecSize + 3, std::ios_base::beg); // Seek here is absolutely not the typical way to go...

					std::cout << std::hex << skel.tellg();

					bones = new TYPES::Bone[header.boneCount1];

					for (int boneCounter = 0; boneCounter < header.boneCount1; ++boneCounter)
						bones[boneCounter].populate(skel, boneNames[boneCounter]);
				}
			};
		}
	}

	struct Mesh {
		enum class VertexType {
			Vertex16,
			Vertex24,
			Unknown
		};

		INTERNAL::MESH::Header header{};
		INTERNAL::MESH::MeshDescSection meshDescSection{};
		INTERNAL::MESH::MeshInfoSection meshInfoSection{};
		INTERNAL::MESH::MeshDataSection meshDataSection{};

		std::vector<INTERNAL::MESH::MeshBuffer<INTERNAL::MESH::TYPES::Vertex16>> meshBuffersV16{};
		std::vector<INTERNAL::MESH::MeshBuffer<INTERNAL::MESH::TYPES::Vertex24>> meshBuffersV24{};

		std::vector<VertexType> vertexTypes;
	};
	
	class MeshLoader {
		bool load_once = false;
		
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		
		Mesh::VertexType determine_vertexType(byte* vertDataBuffer) {
			using namespace INTERNAL::MESH::TYPES;
			Vertex16* v16 = (Vertex16*)vertDataBuffer;
			if (v16->checkVert == 15360)
				return Mesh::VertexType::Vertex16;
			Vertex24* v24 = (Vertex24*)vertDataBuffer;
			if (v24->checkVert == 1.0f)
				return Mesh::VertexType::Vertex24;
			return Mesh::VertexType::Unknown;
		}

	public:

		MeshLoader() = default;

		MeshLoader(const std::string& mesh_file_name, const std::string& skel_file_name = "") {
			load(mesh_file_name, skel_file_name);
		}

		void load(const std::string& mesh_file_name, const std::string& skel_file_name = "") {
			using namespace INTERNAL;
			
			if (load_once) {
				error("MeshLoader can only load one file once!");
			}
			load_once = true;

			std::ifstream file(mesh_file_name, std::ios::binary);

			file.read((char*)&mesh->header, sizeof(MESH::Header));

			expect(mesh->header.magic_u, 0x48534D4Du);
			expect(mesh->header.version, 0x11u);

			mesh->meshDescSection.populate(file);
			mesh->meshInfoSection.populate(file);
			mesh->meshDataSection.populate(file, mesh->meshDescSection);

			for (int meshCounter = 0; meshCounter < mesh->meshInfoSection.meshCount; ++meshCounter) {
				Mesh::VertexType vt = determine_vertexType(mesh->meshDataSection.vertDataSections[meshCounter]);
				if (vt == Mesh::VertexType::Vertex16) {
					mesh->meshBuffersV16.push_back(MESH::MeshBuffer<MESH::TYPES::Vertex16>{});
					mesh->meshBuffersV16[meshCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, meshCounter);
				}
				else if (vt == Mesh::VertexType::Vertex24) {
					mesh->meshBuffersV24.push_back(MESH::MeshBuffer<MESH::TYPES::Vertex24>{});
					mesh->meshBuffersV24[meshCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, meshCounter);
				}
				else {
					error("MeshLoader can't deduce Vertex type!");
				}
				mesh->vertexTypes.push_back(vt);
			}
		}

		std::shared_ptr<Mesh> getMesh() {
			return mesh;
		}

	};
}