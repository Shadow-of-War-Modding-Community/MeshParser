#include "mesh.h"

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::LodThresholds::populate(std::ifstream& mesh, ulong meshLodCount, ulong shadowLodCount) {
	meshLodThresholds = new float[meshLodCount];
	mesh.read((char*)meshLodThresholds, sizeof(float) * meshLodCount);

	shadowLodThresholds = new float[shadowLodCount];
	mesh.read((char*)shadowLodThresholds, sizeof(float) * shadowLodCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::LodThresholds::~LodThresholds() {
	delete[] meshLodThresholds, shadowLodThresholds;
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::LodConnections::populate(std::ifstream& mesh, ulong meshLodCount, ulong shadowLodCount) {
	meshLodConnections = new ulong[meshLodCount];
	mesh.read((char*)meshLodConnections, sizeof(ulong) * meshLodCount);

	shadowLodConnections = new ulong[shadowLodCount];
	mesh.read((char*)shadowLodConnections, sizeof(ulong) * shadowLodCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::LodConnections::~LodConnections() {
	delete[] meshLodConnections, shadowLodConnections;
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::populate(std::ifstream& mesh, ulong lodSettingsCount, ulong meshLodCount, ulong shadowLodCount) {
	read_ulong(sectionID, mesh);

	expect(sectionID, 0x8E3E068Eu);

	lodSettings = new ulong[lodSettingsCount];
	mesh.read((char*)lodSettings, sizeof(ulong) * lodSettingsCount);
	
	lodThresholds.populate(mesh, meshLodCount, shadowLodCount);
	lodConnections.populate(mesh, meshLodCount, shadowLodCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::LodSection::~LodSection() {
	delete[] lodSettings;
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BufferLayoutSection::VertexBufferLayout::populate(std::ifstream& mesh) {
	read_ulong(attributesCount, mesh);

	attributesLayout = new AttributeLayout[attributesCount];
	mesh.read((char*)attributesLayout, sizeof(AttributeLayout) * attributesCount);

}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BufferLayoutSection::VertexBufferLayout::~VertexBufferLayout() {
	delete[] attributesLayout;
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BufferLayoutSection::populate(std::ifstream& mesh, ulong bufferLayoutCount) {
	read_ulong(sectionID, mesh);

	expect(sectionID, 0x37D749A6u);

	vertexBufferLayouts = new VertexBufferLayout[bufferLayoutCount];

	for (int i = 0; i < bufferLayoutCount; ++i)
		vertexBufferLayouts[i].populate(mesh);

}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BufferLayoutSection::~BufferLayoutSection() {
	delete[] vertexBufferLayouts;
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BoneSection::populate(std::ifstream& mesh, ulong boneCount) {
	read_ulong(sectionID, mesh);

	expect(sectionID, 0x93D9A424u);

	bones = new TYPES::Bone[boneCount];
	mesh.read((char*)bones, sizeof(TYPES::Bone) * boneCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::BoneSection::~BoneSection() {
	delete[] bones;
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::MeshInfoSection(std::ifstream& mesh) {
	populate(mesh);
}

void MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::populate(std::ifstream& mesh) {
	mesh.read((char*)this, 72); // parse until lodGroupIDs array

	expect(sectionID, 0x1A1541BCu);

	lodGroupIDs = new ulong[lodGroupCount];
	mesh.read((char*)lodGroupIDs, sizeof(ulong) * lodGroupCount);

	connections = new Connection[connectionCount];
	mesh.read((char*)connections, sizeof(Connection) * connectionCount);

	lodInfos = new LodInfo[lodCount];
	mesh.read((char*)lodInfos, sizeof(LodInfo) * lodCount);

	meshInfos = new MeshInfo[meshCount];
	mesh.read((char*)meshInfos, sizeof(MeshInfo) * meshCount);

	lodSection.populate(mesh, lodSettingsCount, meshLodCount, shadowLodCount);

	bufferLayoutSection.populate(mesh, bufferLayoutCount);

	boneSection.populate(mesh, boneCount);
}

MESH_UNPACKER::INTERNAL::MESH::MeshInfoSection::~MeshInfoSection() {
	delete[] lodGroupIDs, connections, lodInfos, meshInfos;
}