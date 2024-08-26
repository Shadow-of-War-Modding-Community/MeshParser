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

MESH_UNPACKER::Mesh::VertexType MESH_UNPACKER::MeshLoader::determine_vertexType(byte* vertDataBuffer) {
	using namespace INTERNAL::MESH::TYPES;
	Vertex16* v16 = (Vertex16*)vertDataBuffer;
	if ((float)v16->positions.w == 1.0f)
		return Mesh::VertexType::Vertex16;
	Vertex24* v24 = (Vertex24*)vertDataBuffer;
	if (v24->positions.w == 1.0f)
		return Mesh::VertexType::Vertex24;
	return Mesh::VertexType::Unknown;
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

	int mesh_counted = 0;
	for (int lodCounter = 0; lodCounter < mesh->meshInfoSection.lodCount; ++lodCounter) {
		Mesh::VertexType vt = determine_vertexType(mesh->meshDataSection.vertexDataSections[lodCounter]);
		if (vt == Mesh::VertexType::Vertex16) {
			mesh->meshBuffersV16.push_back(MESH::MeshBuffer<MESH::TYPES::Vertex16>());
			mesh->meshBuffersV16[lodCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, lodCounter, mesh_counted);
		}
		else if (vt == Mesh::VertexType::Vertex24) {
			mesh->meshBuffersV24.push_back(MESH::MeshBuffer<MESH::TYPES::Vertex24>{});
			mesh->meshBuffersV24[lodCounter].populate(mesh->meshInfoSection, mesh->meshDataSection, lodCounter, mesh_counted);
		}
		else {
			error("MeshLoader can't deduce Vertex type!");
		}
		mesh_counted += mesh->meshInfoSection.lodInfos[lodCounter].meshCount;
		mesh->vertexTypes.push_back(vt);
	}
}

std::shared_ptr<MESH_UNPACKER::Mesh> MESH_UNPACKER::MeshLoader::getMesh() {
	return mesh;
}


