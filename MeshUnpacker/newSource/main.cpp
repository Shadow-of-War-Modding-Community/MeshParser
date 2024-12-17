#include "Parser/includes-types.h"

int main() {
	using namespace PARSER;

	auto mesh = Parser::Import("F:\\sauron_flangedmace.mesh");
	
	auto mesh2 = Parser::Import("F:\\sf.fbx", &mesh);
	
	Parser::Export(mesh2, "F:\\sf.mesh", "mesh");

	//auto mesh3 = Parser::Import("F:\\sf.mesh");
	//
	//Parser::Export(mesh3, "F:\\out.fbx", "fbx");
}