#pragma once

#include "../includes-types.h"

namespace PARSER {

	using namespace TYPES;

	namespace INTERNAL {
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

			std::vector<long> unkMatIndices;
			std::vector<long> materialIndices;

			std::vector<ulong> vertexDataSectionSizes;
			std::vector<ulong> faceDataSectionSizes;
			std::vector<ulong> vertexGroupDataSectionSizes;
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
				ulong vertexGroupDataSize;
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
					std::vector<float> meshLodThresholds;
					std::vector<float> shadowLodThresholds;
				};

				struct LodConnections {
					std::vector<ulong> meshLodConnections;
					std::vector<ulong> shadowLodConnections;
				};

				ulong sectionID;
				std::vector<ulong> lodSettings;
				LodThresholds lodThresholds;
				LodConnections lodConnections;

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
							VECTOR2S16 = 9,
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
					std::vector<AttributeLayout> attributesLayouts;
				};

				ulong sectionID;
				std::vector<VertexBufferLayout> vertexBufferLayouts;

			};

			struct BoneSection {
				ulong sectionID;
				std::vector<MESH::Bone> bones;
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

			std::vector<ulong> lodGroupIDs;

			std::vector<Connection> connections;

			std::vector<LodInfo> lodInfos;
			std::vector<MeshInfo> meshInfos;

			LodSection lodSection;

			BufferLayoutSection bufferLayoutSection;

			BoneSection boneSection;
		};

		struct MeshDataSection {
			ulong sectionID; // 0x95DBDB69

			std::vector<std::vector<byte>> vertexDataSections;
			std::vector<std::vector<byte>> faceDataSections;
			std::vector<std::vector<byte>> vertexGroupDataSections;
		};
	}

	struct BufferLayout {
		ulong size;
		std::vector<INTERNAL::MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout> order;
	};

	struct LODContainer {
		std::vector<std::vector<MESH::VertexAttribute>> meshVertexAttributeContainers;
		std::vector<std::vector<MESH::Face>> meshFaceContainers;
		std::vector<std::vector<byte>> meshVertexGroupContainers;
	};

	class Mesh {
		INTERNAL::Header header{};
		INTERNAL::MeshDescSection meshDescSection{};
		INTERNAL::MeshInfoSection meshInfoSection{};
		INTERNAL::MeshDataSection meshDataSection{};

		std::vector<BufferLayout> bufferLayouts;

		std::vector<LODContainer> lodContainers;

		std::unique_ptr<aiScene> scene;

		void mesh_to_internal(const std::string& mesh_file);

		void internal_to_lodContainers();

		void lodContainers_to_assimp();


		void assimp_to_mesh();

	public:

		void import_mesh(const std::string& mesh_file);

		void export_custom(const std::string& file, const std::string& format);
	};
}