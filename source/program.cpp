#include "program.hpp"
#include "window.hpp"
#include "assets.hpp"
#include "ui.hpp"
#include "imgui/imgui_platform.hpp"

main_state::main_state() {
	no::ui::create(window(), "calibril.ttf", 18);
	tags::load();
	window().set_clear_color({ 27.0f / 256.0f,  27.0f / 256.0f, 27.0f / 256.0f });
	set_synchronization(no::draw_synchronization::if_updated);
	window().set_swap_interval(no::swap_interval::immediate);
	tag_ui = std::make_unique<tag_system_ui>();
	files = std::make_unique<file_view>(window(), mouse(), keyboard());
	files->load_directory("E:/"); // todo: don't hardcode the starting directory lol
}

main_state::~main_state() {
	no::ui::destroy();
}

void main_state::update() {
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Options")) {
		ImGui::PushItemWidth(360.0f);
		if (ImGui::MenuItem("Limit FPS", nullptr, &limit_fps)) {
			set_synchronization(limit_fps ? no::draw_synchronization::if_updated : no::draw_synchronization::always);
		}
		ImGui::MenuItem("Double click to open directories", "", &files->config.double_click_opens_directories);
		ImGui::MenuItem("Double click to open files", "", &files->config.double_click_opens_files);
		if (ImGui::MenuItem("Theme")) {
			show_theme_options = true;
		}
		ImGui::PopItemWidth();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("View")) {
		ImGui::PushItemWidth(360.0f);
		if (ImGui::MenuItem("320px")) {
			files->entry_size = 320.0f;
		}
		if (ImGui::MenuItem("256px")) {
			files->entry_size = 256.0f;
		}
		if (ImGui::MenuItem("128px")) {
			files->entry_size = 128.0f;
		}
		if (ImGui::MenuItem("96px")) {
			files->entry_size = 96.0f;
		}
		ImGui::PopItemWidth();
		ImGui::EndMenu();
	}
	no::ui::colored_text({ 0.9f, 0.9f, 0.1f }, "\tFPS: %i", frame_counter().current_fps());
	if (no::ui::button("←")) {
		files->pop_history();
	}
	ImGui::EndMainMenuBar();
	files->update();
	tag_ui->update(window().size().to<float>());
	if (show_theme_options) {
		no::vector2f theme_size{ 640.0f, 512.0f };
		no::ui::push_window("Theme options", window().size().to<float>() / 2.0f - theme_size / 2.0f, theme_size);
		if (no::ui::button("Reset")) {
			files->config.entry_hover_color = { 1.0f, 1.0f, 1.0f, 0.19f };
		}
		no::ui::inline_next();
		no::ui::text("Hover entry color");
		no::ui::inline_next();
		ImGui::ColorEdit4("##entry-hover-color", &files->config.entry_hover_color.x);
		no::ui::separate();
		no::ui::separate();
		no::ui::separate();
		if (ImGui::Button("Close")) {
			show_theme_options = false;
		}
		no::ui::pop_window();
	}
}

void main_state::draw() {
	
}
