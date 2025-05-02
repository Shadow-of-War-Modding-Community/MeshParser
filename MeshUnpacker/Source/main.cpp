#include "Parser/includes-types.h"

int main() {
	using namespace PARSER;

	auto mesh = Parser::Import("./Tests/player_rangerswordt1.mesh");

	Parser::Export(mesh, "./Tests/player_rangerswordt1.fbx", "fbx");

	auto fbx = Parser::Import("./Tests/player_rangerswordt1.fbx", &mesh);

	Parser::Export(fbx, "./Tests/player_rangerswordt1-fbx.mesh", "mesh");
}