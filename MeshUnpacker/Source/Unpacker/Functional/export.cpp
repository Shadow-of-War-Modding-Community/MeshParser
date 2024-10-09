#include "../Internal/includes-types-defs.h"

void export_assimp(std::shared_ptr<aiScene> scene, const char* format, const char* name) {
	Assimp::Exporter exporter;
	if (exporter.Export(scene.get(), format, name) != AI_SUCCESS) {
		error(exporter.GetErrorString());
	}
}

// Will only use the original buffers. LODBuffers will be ignored!
void export_mesh(std::shared_ptr<MESH_UNPACKER::Mesh> mesh, const char* name) {

}