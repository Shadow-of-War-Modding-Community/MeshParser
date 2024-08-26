#include "mesh.h"

MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::MeshDescSection(std::ifstream& mesh) {
	populate(mesh);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::populate(std::ifstream& mesh) {
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

MESH_UNPACKER::INTERNAL::MESH::MeshDescSection::~MeshDescSection() {
	delete[] unkMatIndices, meshMatIndices, vertSectionSizes, faceSectionSizes,
		skinSectionSizes;
}

MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::MeshDataSection(std::ifstream& mesh, MeshDescSection& meshDescSection) {
	populate(mesh, meshDescSection);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::populate(std::ifstream& mesh, MeshDescSection& meshDescSection) {
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

MESH_UNPACKER::INTERNAL::MESH::MeshDataSection::~MeshDataSection() {
	for (auto& iter : vertDataSections)
		delete[] iter;
	for (auto& iter : faceDataSections)
		delete[] iter;
	for (auto& iter : skinDataSections)
		delete[] iter;
}

MESH_UNPACKER::Mesh::VertexType MESH_UNPACKER::MeshLoader::determine_vertexType(byte* vertDataBuffer) {
	using namespace INTERNAL::MESH::TYPES;
	Vertex16* v16 = (Vertex16*)vertDataBuffer;
	if (v16->checkVert == 15360)
		return Mesh::VertexType::Vertex16;
	Vertex24* v24 = (Vertex24*)vertDataBuffer;
	if (v24->checkVert == 1.0f)
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
		Mesh::VertexType vt = determine_vertexType(mesh->meshDataSection.vertDataSections[lodCounter]);
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


