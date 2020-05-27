#pragma once

#include "entry.hpp"
#include "draw.hpp"
#include "input.hpp"
#include "event.hpp"

class file_view {
public:

	no::vector2f top_left_position{ 335.0f, 23.0f };
	no::vector2f entry_size{ 288.0f, 288.0f };
	no::vector2f entry_margin{ 12.0f, 12.0f };
	no::vector2f entry_full_size{ entry_size + entry_margin };

	struct {
		bool double_click_opens_directories{ true };
		bool double_click_opens_files{ true };
		no::vector4f entry_hover_color{ 1.0f, 1.0f, 1.0f, 0.19f };
	} config;

	file_view(no::window& window, no::mouse& mouse, no::keyboard& keyboard);
	file_view(const file_view&) = delete;
	file_view(file_view&&) = delete;

	~file_view();

	file_view& operator=(const file_view&) = delete;
	file_view& operator=(file_view&&) = delete;

	void update();
	bool is_active() const;
	void clear_entries();
	void load_directory(const std::filesystem::path& path);
	void pop_history();
	void clear_selection();
	void select_all();

	std::vector<directory_entry*> selected_entries();
	std::vector<directory_entry*> entries_between(directory_entry* from, directory_entry* to);

private:

	void directory_entry_control(directory_entry& entry);
	void update_entry_context_menu();

	no::transform2 transform;
	std::vector<directory_entry> entries;
	no::rectangle rectangle;
	int blank_texture{ -1 };
	no::window& window;
	no::mouse& mouse;
	no::keyboard& keyboard;

	no::event_listener mouse_button_listener;
	no::event_listener double_click_listener;

	directory_entry* context_entry{ nullptr };
	directory_entry* single_selected_entry{ nullptr };

	no::platform::system_cursor old_cursor{ no::platform::system_cursor::arrow };
	no::platform::system_cursor new_cursor{ no::platform::system_cursor::arrow };

	std::vector<std::filesystem::path> directory_history;

};
