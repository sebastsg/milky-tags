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
	browser = std::make_unique<file_browser>(window(), mouse(), keyboard());
#if PLATFORM_WINDOWS
	window().set_icon_from_resource(102);
#endif
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
		ImGui::MenuItem("Double click to open directories", "", &browser->config.double_click_opens_directories);
		ImGui::MenuItem("Double click to open files", "", &browser->config.double_click_opens_files);
		ImGui::MenuItem("Show pretty name", "", &browser->config.show_pretty_name);
		if (ImGui::MenuItem("Theme")) {
			show_theme_options = true;
		}
		ImGui::PopItemWidth();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("View")) {
		ImGui::PushItemWidth(360.0f);
		if (ImGui::MenuItem("320px")) {
			browser->entry_size = 320.0f;
		}
		if (ImGui::MenuItem("256px")) {
			browser->entry_size = 256.0f;
		}
		if (ImGui::MenuItem("128px")) {
			browser->entry_size = 128.0f;
		}
		if (ImGui::MenuItem("96px")) {
			browser->entry_size = 96.0f;
		}
		ImGui::PopItemWidth();
		ImGui::EndMenu();
	}
	no::ui::colored_text({ 0.9f, 0.9f, 0.1f }, "\tFPS: %i", frame_counter().current_fps());
	if (no::ui::button("←")) {
		browser->pop_history();
	}
	auto active_dir = browser->active_directory();
	active_dir.make_preferred();
	auto path_parts = no::split_string(active_dir.u8string(), std::filesystem::path::preferred_separator);
	std::filesystem::path current_path_here;
	for (const auto& path_part : path_parts) {
		current_path_here /= path_part;
		current_path_here.make_preferred();
		if (path_part.empty()) {
			no::ui::text("/");
			no::ui::inline_next();
			continue; // for f.ex C://
		}
		std::error_code error_code{};
		if (std::filesystem::equivalent(current_path_here, active_dir, error_code)) {
			if (error_code.value() != 0) {
				WARNING("File system error: " << error_code.message());
			}
			no::ui::text(path_part);
		} else if (error_code.value() == 0 && no::ui::button(path_part)) {
			browser->load_directory(current_path_here);
		}
		no::ui::inline_next();
		no::ui::text("/");
		no::ui::inline_next();
	}
	no::ui::new_line();

	ImGui::EndMainMenuBar();
	
	browser->update();
	
	no::ui::push_static_window("##side", { 0.0f, 23.0f }, { 336.0f, static_cast<float>(window().size().y) - 23.0f });
	tag_ui->update();
	search.update(*browser);
	no::ui::text("%i thumbnail requests", static_cast<int>(browser->loader.requests.size()));
	no::ui::pop_window();

	if (show_theme_options) {
		no::vector2f theme_size{ 640.0f, 512.0f };
		no::ui::push_window("Theme options", window().size().to<float>() / 2.0f - theme_size / 2.0f, theme_size);
		if (no::ui::button("Reset")) {
			browser->config.entry_hover_color = { 1.0f, 1.0f, 1.0f, 0.19f };
		}
		no::ui::inline_next();
		no::ui::text("Hover entry color");
		no::ui::inline_next();
		ImGui::ColorEdit4("##entry-hover-color", &browser->config.entry_hover_color.x);
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
