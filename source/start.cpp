#include "program.hpp"
#include "assets.hpp"

#define DEV_VERSION 1

void configure() {
#if DEV_VERSION
	no::set_asset_directory("../..");
#endif
	no::register_font("seguiemj.ttf", 16);
}

void start() {
	no::create_state<main_state>("Milky Tags");
}
