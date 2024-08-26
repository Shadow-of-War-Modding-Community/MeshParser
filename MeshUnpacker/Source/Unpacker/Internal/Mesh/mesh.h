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
			struct Connection {
				long lodGoup;
				long lod;
				long material;
				long unk;
			};

			struct LodInfo {
				ulong vertexDataSize;
				ulong faceDataSize;
				ulong VertexGroupDataSize;
				ulong meshCount;
				ushort faceCount[16];
			};

			struct MeshInfo {
				ulong dataOffset;
				ulong verticesOffset;
				ulong verticesCount;
				ulong faceIndicesOffset;
				ulong faceIndicesCount;
				ulong vertexGroupsOffset;
				ulong vertexGroupsCount;
				ulong unknown7;
				ulong unknown8;
				ulong layerIndex;
			};

			struct LodSection {
				struct LodThresholds {
					float* meshLodThresholds = nullptr;
					float* shadowLodThresholds = nullptr;

					void populate(std::ifstream&, ulong, ulong);

					~LodThresholds();
				};

				struct LodConnections {
					ulong* meshLodConnections = nullptr;
					ulong* shadowLodConnections = nullptr;

					void populate(std::ifstream&, ulong, ulong);

					~LodConnections();
				};

				ulong sectionID;
				ulong* lodSettings = nullptr;
				LodThresholds lodThresholds;
				LodConnections lodConnections;

				void populate(std::ifstream&, ulong, ulong, ulong);

				~LodSection();

			};

			struct BufferLayoutSection {
				struct VertexBufferLayout {
					struct ElementLayout {
						enum class Type : byte {
							UNK0 = 0,
							UNK1 = 1,
							VECTOR3F32 = 2,
							VECTOR4F32 = 3,
							UINT8 = 4,
							UNK5 = 5,
							UNK6 = 6,
							UNK7 = 7,
							UNK8 = 8,
							UINT16 = 9,
							UNK10 = 10,
							UNK11 = 11,
							VECTOR4F16 = 12
						};

						enum class Element : byte {
							POSITION = 0,
							NORMAL = 1,
							TANGENT = 2,
							BITANGENT = 3,
							TEXCOORD = 4,
							COLOR = 5,
							WEIGHT = 6,
							VERTEXGROUP = 7,
							UNK8 = 8,
							UNK9 = 9,
							UNK10 = 10,
							UNK11 = 11,
							UNK12 = 12,
						};

						byte bufferIndex;
						Type type;
						Element element;
						byte channel;
					};

					ulong elementCount;
					ElementLayout* elementsLayout = nullptr;

					void populate(std::ifstream&);

					~VertexBufferLayout();
				};
				
				ulong sectionID;
				VertexBufferLayout* vertexBufferLayouts = nullptr;

				void populate(std::ifstream&, ulong);

				~BufferLayoutSection();
			};

			struct BoneSection {
				ulong sectionID;
				TYPES::Bone* bones = nullptr;

				void populate(std::ifstream&, ulong);

				~BoneSection();
			};

			ulong sectionID; // 0x1A1541BC

			ulong lodGroupCount;
			ulong connectionCount;
			ulong lodCount;
			ulong meshCount;

			ulong unknownULong1;

			ulong lodSettingsCount;
			ulong meshLodCount;
			ulong shadowLodCount;

			ulong bufferLayoutCount;
			ulong ulongReadCount;

			float unknownFloat;

			float unknown1[4];

			ulong boneCount;
			ulong boneSectionSize;

			ulong* lodGroupIDs = nullptr;

			Connection* connections = nullptr;

			LodInfo* lodInfos = nullptr;
			MeshInfo* meshInfos = nullptr;

			LodSection lodSection;

			BufferLayoutSection bufferLayoutSection;

			BoneSection boneSection;

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

			void populate(MeshInfoSection& meshInfoSection, MeshDataSection& meshDataSection, int lodNumber) {
				for (int mesh = 0; mesh < meshInfoSection.lodInfos[lodNumber].meshCount; ++mesh) {
					VERTEX* v = (VERTEX*)meshDataSection.vertDataSections[lodNumber];
					subMeshesVertexContainer.push_back(std::vector<VERTEX>{}); // To allocate the actual vertex container
					for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh].verticesCount; ++vertexCounter) {
						checkVert(v[vertexCounter]);
						subMeshesVertexContainer[mesh].push_back(v[vertexCounter]);
					}
					// UV loop here...
					TYPES::Face* f = (TYPES::Face*)meshDataSection.faceDataSections[lodNumber];
					subMeshesFaceContainer.push_back(std::vector<TYPES::Face>{});
					for (int faceCounter = 0; faceCounter < meshInfoSection.meshInfos[mesh].faceIndicesCount / 3; ++faceCounter) {
						subMeshesFaceContainer[mesh].push_back(f[faceCounter]);
					}
					byte* s = meshDataSection.skinDataSections[lodNumber];
					subMeshesSkinContainer.push_back(std::vector<byte>{});
					for (int skinCounter = 0; skinCounter < meshInfoSection.meshInfos[mesh].vertexGroupsCount; ++skinCounter) {
						subMeshesSkinContainer[mesh].push_back(s[skinCounter]);
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