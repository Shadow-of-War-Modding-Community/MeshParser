#pragma once
#include "../includes-types-defs.h"

//FRWDDEC
namespace MESH_UNPACKER::INTERNAL::SKEL {
	struct Skeleton;
}


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
			ulong materialIndicesCount;
			ulong dataSectionCount;

			long* unkMatIndices = nullptr;
			long* materialIndices = nullptr;

			ulong* vertexDataSectionSizes = nullptr;
			ulong* faceDataSectionSizes = nullptr;
			ulong* vertexGroupDataSectionSizes = nullptr;

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
					struct AttributeLayout {
						enum class Buffer : byte {
							Buffer_0 = 0,
							Buffer_1 = 1
						};

						enum class Type : byte {
							TYPE0 = 0,
							VECTOR2F32 = 1,
							VECTOR3F32 = 2,
							VECTOR4F32 = 3,
							VECTOR4U8 = 4,
							TYPE5 = 5,
							TYPE6 = 6,
							TYPE7 = 7,
							TYPE8 = 8,
							VECTOR216 = 9,
							TYPE10 = 10,
							TYPE11 = 11,
							VECTOR4F16 = 12,
						};

						enum class Attribute : byte {
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
							UNK12 = 12
						};

						enum class Channel : byte {
							Channel_0 = 0,
							Channel_1 = 1
						};

						Buffer bufferIndex;
						Type type;
						Attribute attribute;
						Channel channel;
					};

					ulong attributesCount;
					AttributeLayout* attributesLayout = nullptr;

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

			std::vector<byte*> vertexDataSections;
			std::vector<byte*> faceDataSections;
			std::vector<byte*> vertexGroupDataSections;

			MeshDataSection() = default;

			MeshDataSection(std::ifstream&, MeshDescSection&);

			void populate(std::ifstream&, MeshDescSection&);

			~MeshDataSection();
		};

		struct BufferLayout {
			std::vector<MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout> order;

			void populate(MeshInfoSection&, int);
		};

		struct LODBuffer {
			std::vector<std::vector<TYPES::VertexAttribute>> meshVertexAttributeContainers;
			std::vector<std::vector<TYPES::Face>> meshFaceContainers;
			std::vector<std::vector<byte>> meshVertexGroupContainers;

			void populate(MeshInfoSection&, MeshDataSection&, const std::vector<INTERNAL::MESH::BufferLayout>&, int, int);
		};
	}

	struct Mesh {
		bool has_skeleton = false;

		INTERNAL::MESH::Header header{};
		INTERNAL::MESH::MeshDescSection meshDescSection{};
		INTERNAL::MESH::MeshInfoSection meshInfoSection{};
		INTERNAL::MESH::MeshDataSection meshDataSection{};

		INTERNAL::SKEL::Skeleton skeleton{};

		std::vector<INTERNAL::MESH::LODBuffer> lodBuffers;
	};

	class MeshLoader {
		bool load_once = false;

		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

		std::vector<INTERNAL::MESH::BufferLayout> bufferLayouts;

	public:

		MeshLoader() = default;

		MeshLoader(const std::string&, const std::string& = "");

		void load(const std::string&, const std::string& = "");

		std::shared_ptr<Mesh> getMesh();

	};
}