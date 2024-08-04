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

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::ModelLodSettings::populate(std::ifstream& mesh) {
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

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::ModelLodSettings::~ModelLodSettings() {
	delete[] unknownByte1, unknownByte2, unknownByte3, unknownByte4;
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::MeshInfoSection(std::ifstream& mesh) {
	populate(mesh);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::populate(std::ifstream& mesh) {
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

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::~MeshInfoSection() {
	delete[] modelIDs, matAssignments, meshInfos, subMeshInfos, lodSettings,
		lodSettings, lodThresholds, lodConnections, modelLodSettings, bones;
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

	for (int meshCounter = 0; meshCounter < mesh->meshInfoSection.meshCount; ++meshCounter) {
		Mesh::VertexType vt = determine_vertexType(mesh->meshDataSection.vertDataSections[meshCounter]);
		if (vt == Mesh::VertexType::Vertex16) {
			mesh->meshBuffersV16.push_back(MESH::MeshBuffer<MESH::TYPES::Vertex16>());
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

std::shared_ptr<MESH_UNPACKER::Mesh> MESH_UNPACKER::MeshLoader::getMesh() {
	return mesh;
}
