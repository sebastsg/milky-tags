#pragma once

#include "entry.hpp"
#include "draw.hpp"
#include "input.hpp"

class file_browser {
public:

	thumbnail_loader loader;

	no::vector2f top_left_position{ 335.0f, 23.0f };
	no::vector2f entry_size{ 288.0f, 288.0f };
	no::vector2f entry_margin{ 12.0f, 12.0f };
	no::vector2f entry_full_size{ entry_size + entry_margin };

	struct {
		std::string default_open_path;
		bool double_click_opens_directories{ true };
		bool double_click_opens_files{ true };
		bool show_pretty_name{ true };
		no::vector4f entry_hover_color{ 1.0f, 1.0f, 1.0f, 0.19f };
	} config;

	file_browser(no::window& window, no::mouse& mouse, no::keyboard& keyboard);
	file_browser(const file_browser&) = delete;
	file_browser(file_browser&&) = delete;

	file_browser& operator=(const file_browser&) = delete;
	file_browser& operator=(file_browser&&) = delete;

	void update();
	bool is_active() const;
	void clear_entries();
	void load_directory(const std::filesystem::path& path);
	void load_paths(const std::vector<std::filesystem::path>& paths);
	void pop_history();
	void clear_selection();
	void select_all();

	std::filesystem::path active_directory() const {
		return directory_history.empty() ? config.default_open_path : directory_history.back();
	}

	std::vector<directory_entry*> selected_entries();
	std::vector<directory_entry*> entries_between(directory_entry* from, directory_entry* to);

private:

	void update_start();
	void update_entries();

	void directory_entry_control(directory_entry& entry);
	void update_entry_context_menu();

	no::transform2 transform;
	std::vector<directory_entry> entries;
	no::rectangle rectangle;
	no::window& window;
	no::mouse& mouse;
	no::keyboard& keyboard;

	directory_entry* context_entry{ nullptr };
	directory_entry* single_selected_entry{ nullptr };

	no::platform::system_cursor old_cursor{ no::platform::system_cursor::arrow };
	no::platform::system_cursor new_cursor{ no::platform::system_cursor::arrow };

	std::vector<std::filesystem::path> directory_history;

	std::vector<std::filesystem::path> root_directories;

};
