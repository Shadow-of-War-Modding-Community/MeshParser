#include "Parser/includes-types.h"

int main() {
	using namespace PARSER;

	//auto mesh = Parser::Import("F:\\sauron_flangedmace.mesh");
	//
	//auto mesh2 = Parser::Import("F:\\out.fbx", &mesh);
	//
	//Parser::Export(mesh2, "F:\\out.mesh", "mesh");
	//
	//mesh = Parser::Import("F:\\out.mesh");
	//
	//Parser::Export(mesh, "F:\\final.fbx", "fbx");

	//auto mesh = Parser::Import("F:\\untitled.fbx");
	
	//Parser::Export(mesh, "F:\\out.mesh", "mesh");

	auto mesh = Parser::Import("F:\\out.mesh");
	
	Parser::Export(mesh, "F:\\test.fbx", "fbx");
}