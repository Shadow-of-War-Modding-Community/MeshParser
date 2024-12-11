#include "Parser/includes-types.h"

int main() {
	PARSER::Mesh mesh;
	mesh.import_mesh("F:\\balrog_base.mesh");
	mesh.export_custom("F:\\balrog_base.fbx", "fbxa");

	PARSER::Mesh mesh2;
	mesh2.import_custom("F:\\balrog_base.fbx", mesh);

}