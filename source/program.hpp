#pragma once

#include "tags.hpp"
#include "browser.hpp"
#include "loop.hpp"

class main_state : public no::program_state {
public:

	bool limit_fps{ true };

	main_state();
	~main_state() override;

	void update() override;
	void draw() override;

private:

	std::unique_ptr<file_view> files;
	std::unique_ptr<tag_system_ui> tag_ui;
	bool show_theme_options{ false };

};
