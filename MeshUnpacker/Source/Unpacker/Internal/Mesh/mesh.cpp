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
			order.push_back(std::make_pair(layout.attributesLayout[i].attribute, layout.attributesLayout[i].type));
		}
	}
	for (int i = 0; i < layout.attributesCount; ++i) {
		if (layout.attributesLayout[i].bufferIndex == Buffer::Buffer_1) {
			order.push_back(std::make_pair(layout.attributesLayout[i].attribute, layout.attributesLayout[i].type));
		}
	}
}

void MESH_UNPACKER::INTERNAL::MESH::LODBuffer::populate(MeshInfoSection& meshInfoSection, MeshDataSection& meshDataSection, int lodNumber, int mesh_counted) {
	for (int mesh = 0; mesh < meshInfoSection.lodInfos[lodNumber].meshCount; ++mesh) {
		auto current_layer_index = meshInfoSection.meshInfos[mesh_counted + mesh].layerIndex;
		const auto& layout = meshInfoSection.bufferLayoutSection.vertexBufferLayouts[current_layer_index];

		//VERTEX* v = (VERTEX*)meshDataSection.vertexDataSections[lodNumber];
		//subMeshesVertexContainer.push_back(std::vector<VERTEX>{}); // To allocate the actual vertex container
		//for (int vertexCounter = 0; vertexCounter < meshInfoSection.meshInfos[mesh_counted + mesh].verticesCount; ++vertexCounter) {
		//	subMeshesVertexContainer[mesh].push_back(v[vertexCounter]);
		//}
		//// UV loop here...
		//TYPES::Face* f = (TYPES::Face*)meshDataSection.faceDataSections[lodNumber];
		//subMeshesFaceContainer.push_back(std::vector<TYPES::Face>{});
		//for (int faceCounter = 0; faceCounter < meshInfoSection.meshInfos[mesh_counted + mesh].faceIndicesCount / 3; ++faceCounter) {
		//	subMeshesFaceContainer[mesh].push_back(f[faceCounter]);
		//}
		//byte* s = meshDataSection.vertexGroupDataSections[lodNumber];
		//subMeshesSkinContainer.push_back(std::vector<byte>{});
		//for (int skinCounter = 0; skinCounter < meshInfoSection.meshInfos[mesh_counted + mesh].vertexGroupsCount; ++skinCounter) {
		//	subMeshesSkinContainer[mesh].push_back(s[skinCounter]);
		//}
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

	std::ifstream file(mesh_file_name, std::ios::binary);

	file.read((char*)&mesh->header, sizeof(MESH::Header));

	expect(mesh->header.magic_u, 0x48534D4Du);
	expect(mesh->header.version, 0x11u);

	mesh->meshDescSection.populate(file);
	mesh->meshInfoSection.populate(file);
	mesh->meshDataSection.populate(file, mesh->meshDescSection);

	for (int i = 0; i < mesh->meshInfoSection.bufferLayoutCount; ++i) {
		bufferLayouts.push_back(INTERNAL::MESH::BufferLayout{});
		bufferLayouts[i].populate(mesh->meshInfoSection, i);
	}

	int mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < mesh->meshInfoSection.lodCount; ++lodCounter) {
		mesh->lodBuffers.push_back(MESH::LODBuffer{});
		mesh->lodBuffers[lodCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, lodCounter, mesh_counted);
		mesh_counted += mesh->meshInfoSection.lodInfos[lodCounter].meshCount;
	}
}

std::shared_ptr<MESH_UNPACKER::Mesh> MESH_UNPACKER::MeshLoader::getMesh() {
	return mesh;
}


