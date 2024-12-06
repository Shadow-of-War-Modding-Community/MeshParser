#include "../Internal/includes-types-defs.h"

void export_assimp(std::shared_ptr<aiScene> scene, const char* format, const char* name) {
	Assimp::Exporter exporter;
	if (exporter.Export(scene.get(), format, name) != AI_SUCCESS) {
		error(exporter.GetErrorString());
	}
}

// Will only use the original buffers. LODBuffers will be ignored!
void export_mesh(std::shared_ptr<MESH_UNPACKER::Mesh> mesh, const char* name) {	
	using namespace MESH_UNPACKER::INTERNAL;
	std::ofstream file(name, std::ios::binary);

	auto write_ulong = [&](ulong var) {
		file.write((char*)&var, sizeof(ulong));
	};

	auto write_float = [&](float var) {
		file.write((char*)&var, sizeof(float));
	};
	
	// Write Header
	file.write((char*)&mesh->header, sizeof(MESH::Header));

	// Write DescSection
	auto& descSection = mesh->meshDescSection;
	write_ulong(descSection.sectionID);
	write_ulong(descSection.materialIndicesCount);
	write_ulong(descSection.dataSectionCount);

	file.write((char*)descSection.unkMatIndices, sizeof(ulong) * descSection.materialIndicesCount);
	file.write((char*)descSection.materialIndices, sizeof(ulong) * descSection.materialIndicesCount);
	file.write((char*)descSection.vertexDataSectionSizes, sizeof(ulong) * descSection.dataSectionCount);
	file.write((char*)descSection.faceDataSectionSizes, sizeof(ulong) * descSection.dataSectionCount);
	file.write((char*)descSection.vertexGroupDataSectionSizes, sizeof(ulong) * descSection.dataSectionCount);

	// Write InfoSection
	auto& infoSection = mesh->meshInfoSection;
	file.write((char*)&infoSection, 72);

	file.write((char*)infoSection.lodGroupIDs, sizeof(ulong) * infoSection.lodGroupCount);
	file.write((char*)infoSection.connections, sizeof(MESH::MeshInfoSection::Connection) * infoSection.connectionCount);
	file.write((char*)infoSection.lodInfos, sizeof(MESH::MeshInfoSection::LodInfo) * infoSection.lodCount);
	file.write((char*)infoSection.meshInfos, sizeof(MESH::MeshInfoSection::MeshInfo) * infoSection.meshCount);

	// Lod Section
	write_ulong(infoSection.lodSection.sectionID);

	file.write((char*)infoSection.lodSection.lodSettings, sizeof(ulong) * infoSection.lodSettingsCount);
	file.write((char*)infoSection.lodSection.lodThresholds.meshLodThresholds, sizeof(float) * infoSection.meshLodCount);
	file.write((char*)infoSection.lodSection.lodThresholds.shadowLodThresholds, sizeof(float) * infoSection.shadowLodCount);
	file.write((char*)infoSection.lodSection.lodConnections.meshLodConnections, sizeof(ulong) * infoSection.meshLodCount);
	file.write((char*)infoSection.lodSection.lodConnections.shadowLodConnections, sizeof(ulong) * infoSection.shadowLodCount);

	// Buffer Layout Section
	write_ulong(infoSection.bufferLayoutSection.sectionID);

	for (int i = 0; i < infoSection.bufferLayoutCount; ++i) {
		write_ulong(infoSection.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
		file.write((char*)infoSection.bufferLayoutSection.vertexBufferLayouts[i].attributesLayout,
			sizeof(MESH::MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout) *
			infoSection.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
	}

	// Bone Section
	write_ulong(infoSection.boneSection.sectionID);
	file.write((char*)infoSection.boneSection.bones, sizeof(MESH::TYPES::Bone) * infoSection.boneCount);
	
	// Write DataSection
	auto& dataSection = mesh->meshDataSection;
	write_ulong(dataSection.sectionID);

	for (int i = 0; i < descSection.dataSectionCount; ++i) {
		file.write((char*)dataSection.vertexDataSections[i].data(), descSection.vertexDataSectionSizes[i]);
	}
	for (int i = 0; i < descSection.dataSectionCount; ++i) {
		file.write((char*)dataSection.faceDataSections[i].data(), descSection.faceDataSectionSizes[i]);
	}
	for (int i = 0; i < descSection.dataSectionCount; ++i) {
		file.write((char*)dataSection.vertexGroupDataSections[i].data(), descSection.vertexGroupDataSectionSizes[i]);
	}
	file.close();
}