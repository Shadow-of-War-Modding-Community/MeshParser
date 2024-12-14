#include "mesh.h"

extern PARSER::Mesh mesh_to_internal(const std::string& mesh_file);
extern void internal_to_lodContainers(PARSER::Mesh& mesh);
extern void lodContainers_to_assimp(PARSER::Mesh& mesh);

extern void assimp_to_lodContainers(PARSER::Mesh& mesh);
extern void lodContainers_to_internal(PARSER::Mesh& mesh);

void export_mesh(const PARSER::Mesh& mesh, const std::string& path) {
    using namespace PARSER;
    using namespace INTERNAL;
    using namespace MESH;


    std::ofstream mesh_io(path, std::ios::binary);

    auto write_ulong = [&](ulong var) {
        mesh_io.write((char*)&var, sizeof(ulong));
        };

    auto write_float = [&](float var) {
        mesh_io.write((char*)&var, sizeof(float));
        };

    {

        mesh_io.write((char*)&mesh.header, sizeof(Header));
    }

    {
        auto& mDS = mesh.meshDescSection;
        mesh_io.write((char*)&mDS, sizeof(ulong) * 3);

        mesh_io.write((char*)mDS.unkMatIndices.data(), sizeof(long) * mDS.materialIndicesCount);
        mesh_io.write((char*)mDS.materialIndices.data(), sizeof(long) * mDS.materialIndicesCount);
        mesh_io.write((char*)mDS.vertexDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
        mesh_io.write((char*)mDS.faceDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
        mesh_io.write((char*)mDS.vertexGroupDataSectionSizes.data(), sizeof(long) * mDS.dataSectionCount);
    }

    {
        auto& mIS = mesh.meshInfoSection;
        mesh_io.write((char*)&mIS, 72);

        mesh_io.write((char*)mIS.lodGroupIDs.data(), sizeof(ulong) * mIS.lodGroupCount);
        mesh_io.write((char*)mIS.connections.data(), sizeof(MeshInfoSection::Connection) * mIS.connectionCount);
        mesh_io.write((char*)mIS.lodInfos.data(), sizeof(MeshInfoSection::LodInfo) * mIS.lodCount);
        mesh_io.write((char*)mIS.meshInfos.data(), sizeof(MeshInfoSection::MeshInfo) * mIS.meshCount);


        write_ulong(mIS.lodSection.sectionID);

        mesh_io.write((char*)mIS.lodSection.lodSettings.data(), sizeof(ulong) * mIS.lodSettingsCount);
        mesh_io.write((char*)mIS.lodSection.lodThresholds.meshLodThresholds.data(), sizeof(float) * mIS.meshLodCount);
        mesh_io.write((char*)mIS.lodSection.lodThresholds.shadowLodThresholds.data(), sizeof(float) * mIS.shadowLodCount);
        mesh_io.write((char*)mIS.lodSection.lodConnections.meshLodConnections.data(), sizeof(ulong) * mIS.meshLodCount);
        mesh_io.write((char*)mIS.lodSection.lodConnections.shadowLodConnections.data(), sizeof(ulong) * mIS.shadowLodCount);


        write_ulong(mIS.bufferLayoutSection.sectionID);

        for (int i = 0; i < mIS.bufferLayoutCount; ++i) {
            write_ulong(mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
            mesh_io.write((char*)mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesLayouts.data(),
                sizeof(MeshInfoSection::BufferLayoutSection::VertexBufferLayout::AttributeLayout) * mIS.bufferLayoutSection.vertexBufferLayouts[i].attributesCount);
        }


        write_ulong(mIS.boneSection.sectionID);

        mesh_io.write((char*)mIS.boneSection.bones.data(), sizeof(Bone) * mIS.boneCount);
    }

    {
        auto& mDS = mesh.meshDataSection;
        write_ulong(mDS.sectionID);

        for (int i = 0; i < mesh.meshDescSection.dataSectionCount; ++i) {
            mesh_io.write((char*)mesh.meshDataSection.vertexDataSections[i].data(), mesh.meshDescSection.vertexDataSectionSizes[i]);
        }

        for (int i = 0; i < mesh.meshDescSection.dataSectionCount; ++i) {
            mesh_io.write((char*)mesh.meshDataSection.faceDataSections[i].data(), mesh.meshDescSection.faceDataSectionSizes[i]);
        }

        for (int i = 0; i < mesh.meshDescSection.dataSectionCount; ++i) {
            mesh_io.write((char*)mesh.meshDataSection.vertexGroupDataSections[i].data(), mesh.meshDescSection.vertexGroupDataSectionSizes[i]);
        }
    }


    mesh_io.close();
}

PARSER::Mesh PARSER::Parser::Import(const std::string& file)
{
	if (file.ends_with(".mesh")) {
		auto mesh = mesh_to_internal(file);
		internal_to_lodContainers(mesh);
		lodContainers_to_assimp(mesh);
		return mesh;
	}
	Mesh mesh;
	Assimp::Importer importer;
	mesh.assimp_scene = (aiScene*)importer.ReadFile(file, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	assimp_to_lodContainers(mesh);
	lodContainers_to_internal(mesh);
	return mesh;
}

void PARSER::Parser::Export(const Mesh& mesh, const std::string& path, const std::string& format)
{
	if (format == "mesh") {
		export_mesh(mesh, path);
		return;
	}
	Assimp::Exporter exporter;
	if (exporter.Export(mesh.assimp_scene, format, path) != AI_SUCCESS) {
		error(exporter.GetErrorString());
	}
}