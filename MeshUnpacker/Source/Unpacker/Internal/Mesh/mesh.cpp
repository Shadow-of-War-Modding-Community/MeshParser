#include "mesh.h"

MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::MeshDescSection(std::ifstream& mesh) {
	populate(mesh);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::populate(std::ifstream& mesh) {
	mesh.read((char*)this, sizeof(ulong) * 3); // parse first three attributes

	expect(sectionID, 0xEBAEC3FAu);

	unkMatIndices = new long[materialIndicesCount];
	mesh.read((char*)unkMatIndices, sizeof(long) * materialIndicesCount);

	materialIndices = new long[materialIndicesCount];
	mesh.read((char*)materialIndices, sizeof(long) * materialIndicesCount);

	vertexDataSectionSizes = new ulong[dataSectionCount];
	mesh.read((char*)vertexDataSectionSizes, sizeof(ulong) * dataSectionCount);

	faceDataSectionSizes = new ulong[dataSectionCount];
	mesh.read((char*)faceDataSectionSizes, sizeof(ulong) * dataSectionCount);

	vertexGroupDataSectionSizes = new ulong[dataSectionCount];
	mesh.read((char*)vertexGroupDataSectionSizes, sizeof(ulong) * dataSectionCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::~MeshDescSection() {
	delete[] unkMatIndices, materialIndices, vertexDataSectionSizes, faceDataSectionSizes,
		vertexGroupDataSectionSizes;
}

MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::MeshDataSection(std::ifstream& mesh, MeshDescSection& meshDescSection) {
	populate(mesh, meshDescSection);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::populate(std::ifstream& mesh, MeshDescSection& meshDescSection) {
	read_ulong(sectionID, mesh);

	expect(sectionID, 0x95DBDB69u);

	for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
		byte* tmp_buf = new byte[meshDescSection.vertexDataSectionSizes[i]];
		mesh.read((char*)tmp_buf, meshDescSection.vertexDataSectionSizes[i]);
		vertexDataSections.push_back(tmp_buf);
	}

	for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
		byte* tmp_buf = new byte[meshDescSection.faceDataSectionSizes[i]];
		mesh.read((char*)tmp_buf, meshDescSection.faceDataSectionSizes[i]);
		faceDataSections.push_back(tmp_buf);
	}

	for (int i = 0; i < meshDescSection.dataSectionCount; ++i) {
		byte* tmp_buf = new byte[meshDescSection.vertexGroupDataSectionSizes[i]];
		mesh.read((char*)tmp_buf, meshDescSection.vertexGroupDataSectionSizes[i]);
		vertexGroupDataSections.push_back(tmp_buf);
	}
}

MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::~MeshDataSection() {
	for (auto& iter : vertexDataSections)
		delete[] iter;
	for (auto& iter : faceDataSections)
		delete[] iter;
	for (auto& iter : vertexGroupDataSections)
		delete[] iter;
}

void MESH_UNPACKER::INTERNAL::MESH::BufferLayout::populate(MeshInfoSection& meshInfoSection, int index) {
	using Buffer = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout::Buffer;
	
	const auto& layout = meshInfoSection.bufferLayoutSection.vertexBufferLayouts[index];
	for (int i = 0; i < layout.attributesCount; ++i) {
		if (layout.attributesLayout[i].bufferIndex == Buffer::Buffer_0) {
			order.push_back(layout.attributesLayout[i]);
		}
	}
	for (int i = 0; i < layout.attributesCount; ++i) {
		if (layout.attributesLayout[i].bufferIndex == Buffer::Buffer_1) {
			order.push_back(layout.attributesLayout[i]);
		}
	}
}

void MESH_UNPACKER::INTERNAL::MESH::LODBuffer::populate(MeshInfoSection& meshInfoSection, MeshDataSection& meshDataSection, const std::vector<INTERNAL::MESH::BufferLayout>& bufferLayouts,
	int lodNumber, int mesh_counted) {


	using AttributeLayout = MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout;
	using Attribute = AttributeLayout::Attribute;
	using Type = AttributeLayout::Type;
	using Buffer = AttributeLayout::Buffer;

	int virtual_vertex_buffer_offset = 0, virtual_face_buffer_offset = 0, virtual_vertexGroup_buffer_offset = 0;


	auto read_attribute = [&](byte* vertexbuffer, const AttributeLayout& attributeLayout, TYPES::VertexAttribute& va, const Buffer& targetBuffer) 
	{
			if (attributeLayout.bufferIndex != targetBuffer)
				return;
			switch (attributeLayout.attribute) {
				case Attribute::POSITION:
				{
					if (attributeLayout.type == Type::VECTOR4F16) {
						TYPES::Position16* position = (TYPES::Position16*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.position.x = (float)position->x;
						va.position.y = (float)position->y;
						va.position.z = (float)position->z;
						va.position.w = (float)position->w;
						virtual_vertex_buffer_offset += sizeof(TYPES::Position16);
					}
					else if (attributeLayout.type == Type::VECTOR4F32) {
						TYPES::Position32* position = (TYPES::Position32*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.position = *position;
						virtual_vertex_buffer_offset += sizeof(TYPES::Position32);
					}
					else {
						error("Unknown Type for POSITION");
					}
					break;
				}
				case Attribute::WEIGHT:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						TYPES::Weights* weights = (TYPES::Weights*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.weights = *weights;
						virtual_vertex_buffer_offset += sizeof(TYPES::Weights);
					}
					else {
						error("Unknown Type for WEIGHT");
					}
					break;
				}
				case Attribute::VERTEXGROUP:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						TYPES::VertexGroups* vertexGroups = (TYPES::VertexGroups*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.vertexGroups = *vertexGroups;
						virtual_vertex_buffer_offset += sizeof(TYPES::VertexGroups);
					}
					else {
						error("Unknown Type for VERTEXGROUP");
					}
					break;
				}
				case Attribute::TEXCOORD:
				{
					if (attributeLayout.type == Type::VECTOR2F32) {
						TYPES::UV* uv = (TYPES::UV*)(vertexbuffer + virtual_vertex_buffer_offset);
						TYPES::UV tmpUV = *uv;
						va.uvs.push_back(std::make_pair(tmpUV, false));
						virtual_vertex_buffer_offset += sizeof(TYPES::UV);
					}
					else if (attributeLayout.type == Type::VECTOR216) {
						short* uv = (short*)(vertexbuffer + virtual_vertex_buffer_offset);
						TYPES::UV tmpUV;
						tmpUV.u = uv[0];
						tmpUV.v = uv[1];
						va.uvs.push_back(std::make_pair(tmpUV, true));
						virtual_vertex_buffer_offset += sizeof(short) * 2;
					}
					else {
						error("Unknown Type for TEXCOORD");
					}
					break;
				}
				case Attribute::NORMAL:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						TYPES::Normal* normal = (TYPES::Normal*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.normal = *normal;
						virtual_vertex_buffer_offset += sizeof(TYPES::Normal);
					}
					else {
						error("Unknown Type for NORMAL");
					}
					break;
				}
				case Attribute::TANGENT:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						TYPES::Tangent* tangent = (TYPES::Tangent*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.tangent = *tangent;
						virtual_vertex_buffer_offset += sizeof(TYPES::Tangent);
					}
					else {
						error("Unknown Type for TANGENT");
					}
					break;
				}
				case Attribute::BITANGENT:
				{
					if (attributeLayout.type == Type::VECTOR3F32) {
						TYPES::Bitangent* bitangent = (TYPES::Bitangent*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.bitangent = *bitangent;
						virtual_vertex_buffer_offset += sizeof(TYPES::Bitangent);
					}
					else {
						error("Unknown Type for BITANGENT");
					}
					break;
				}
				case Attribute::COLOR:
				{
					if (attributeLayout.type == Type::VECTOR4U8) {
						TYPES::Color* color = (TYPES::Color*)(vertexbuffer + virtual_vertex_buffer_offset);
						va.color = *color;
						virtual_vertex_buffer_offset += sizeof(TYPES::Color);
					}
					else {
						error("Unknown Type for COLOR");
					}
					break;
				}
			}
	};
	
	for (int mesh = 0; mesh < meshInfoSection.lodInfos[lodNumber].meshCount; ++mesh) {
		auto current_layer_index = meshInfoSection.meshInfos[mesh_counted + mesh].layerIndex;
		meshVertexAttributeContainers.push_back(std::vector<TYPES::VertexAttribute>{});	
		for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh_counted + mesh].verticesCount; ++vertexCounter) {
			TYPES::VertexAttribute vertexAttribute{};
			for (auto& attribute : bufferLayouts[current_layer_index].order) {
				read_attribute(meshDataSection.vertexDataSections[lodNumber], attribute, vertexAttribute, Buffer::Buffer_0);
			}
			meshVertexAttributeContainers[mesh].push_back(vertexAttribute);
		}
		for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh_counted + mesh].verticesCount; ++vertexCounter) {
			for (auto& attribute : bufferLayouts[current_layer_index].order) {
				read_attribute(meshDataSection.vertexDataSections[lodNumber], attribute, meshVertexAttributeContainers[mesh][vertexCounter],
					Buffer::Buffer_1);
			}
		}
		TYPES::Face* face = (TYPES::Face*)(meshDataSection.faceDataSections[lodNumber] + virtual_face_buffer_offset);
		meshFaceContainers.push_back(std::vector<TYPES::Face>{});
		for (int faceCounter = 0; faceCounter < meshInfoSection.meshInfos[mesh_counted + mesh].faceIndicesCount / 3; ++faceCounter) {
			meshFaceContainers[mesh].push_back(face[faceCounter]);
			virtual_face_buffer_offset += sizeof(TYPES::Face);
		}
		byte* vertexGroupIndex = (byte*)(meshDataSection.vertexGroupDataSections[lodNumber] + virtual_vertexGroup_buffer_offset);
		meshVertexGroupContainers.push_back(std::vector<byte>{});
		for (int vGCounter = 0; vGCounter < meshInfoSection.meshInfos[mesh_counted + mesh].vertexGroupsCount; ++vGCounter) {
			meshVertexGroupContainers[mesh].push_back(vertexGroupIndex[vGCounter]);
			virtual_vertexGroup_buffer_offset += sizeof(byte);
		}
	}
}

MESH_UNPACKER::MeshLoader::MeshLoader(const std::string& mesh_file_name, const std::string& skel_file_name) {
	load(mesh_file_name, skel_file_name);
}

void MESH_UNPACKER::MeshLoader::load(const std::string& mesh_file_name, const std::string& skel_file_name) {
	using namespace INTERNAL;

	if (load_once) {
		error("MeshLoader can only load one file once!");
	}
	load_once = true;

	if (skel_file_name != "") {
		std::ifstream skel_file(skel_file_name, std::ios::binary);
		mesh->skeleton.header.populate(skel_file);
		mesh->skeleton.boneSection.populate(skel_file, mesh->skeleton.header);
		mesh->has_skeleton = true;
	}

	std::ifstream mesh_file(mesh_file_name, std::ios::binary);

	mesh_file.read((char*)&mesh->header, sizeof(MESH::Header));

	expect(mesh->header.magic_u, 0x48534D4Du);
	expect(mesh->header.version, 0x11u);

	mesh->meshDescSection.populate(mesh_file);
	mesh->meshInfoSection.populate(mesh_file);
	mesh->meshDataSection.populate(mesh_file, mesh->meshDescSection);

	for (int i = 0; i < mesh->meshInfoSection.bufferLayoutCount; ++i) {
		bufferLayouts.push_back(INTERNAL::MESH::BufferLayout{});
		bufferLayouts[i].populate(mesh->meshInfoSection, i);
	}

	int mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < mesh->meshInfoSection.lodCount; ++lodCounter) {
		mesh->lodBuffers.push_back(MESH::LODBuffer{});
		mesh->lodBuffers[lodCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, bufferLayouts, lodCounter, mesh_counted);
		mesh_counted += mesh->meshInfoSection.lodInfos[lodCounter].meshCount;
	}
}

std::shared_ptr<MESH_UNPACKER::Mesh> MESH_UNPACKER::MeshLoader::getMesh() {
	return mesh;
}


