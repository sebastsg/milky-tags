#include "program.hpp"
#include "assets.hpp"

void configure() {
#if _DEBUG
	no::set_asset_directory("../..");
#endif
}

void start() {
	no::create_state<main_state>("Milky Tags");
}
