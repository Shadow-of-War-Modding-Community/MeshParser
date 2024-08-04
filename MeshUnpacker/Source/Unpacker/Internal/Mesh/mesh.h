#pragma once
#include "../includes-types-defs.h"

namespace MESH_UNPACKER {
	namespace INTERNAL::MESH {
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

			MeshDescSection(std::ifstream&);

			void populate(std::ifstream&);

			~MeshDescSection();
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

				void populate(std::ifstream&);

				~ModelLodSettings();
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

			MeshInfoSection(std::ifstream&);

			void populate(std::ifstream&);

			~MeshInfoSection();
		};

		struct MeshDataSection {
			ulong sectionID; // 0x95DBDB69

			std::vector<byte*> vertDataSections;
			std::vector<byte*> faceDataSections;
			std::vector<byte*> skinDataSections;

			MeshDataSection() = default;

			MeshDataSection(std::ifstream&, MeshDescSection&);

			void populate(std::ifstream&, MeshDescSection&);

			~MeshDataSection();
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

		};
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

		Mesh::VertexType determine_vertexType(byte*);

	public:

		MeshLoader() = default;

		MeshLoader(const std::string&, const std::string& = "");

		void load(const std::string&, const std::string& = "");

		std::shared_ptr<Mesh> getMesh();

	};
}