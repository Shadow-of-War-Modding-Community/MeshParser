#include "Parser/includes-types.h"

int main() {
	using namespace PARSER;

	auto mesh = Parser::Import("F:\\sauron_flangedmace.mesh");
	
	auto mesh2 = Parser::Import("F:\\tire.fbx", &mesh);
	
	Parser::Export(mesh2, "F:\\out.mesh", "mesh");

	//auto mesh = Parser::Import("F:\\out.mesh");
	//
	//Parser::Export(mesh, "F:\\out2.fbx", "fbx");

}