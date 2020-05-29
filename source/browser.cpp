#include "browser.hpp"
#include "camera.hpp"
#include "ui.hpp"
#include "window.hpp"

#include <set>

file_browser::file_browser(no::window& window, no::mouse& mouse, no::keyboard& keyboard) : window{ window }, mouse { mouse }, keyboard{ keyboard } {
	root_directories = no::platform::get_root_directories(); // todo: update this every now and then
	load_directory(config.default_open_path);
}

void file_browser::update() {
	new_cursor = no::platform::system_cursor::arrow;
	if (entries.size() > 0) {
		update_entries();
	} else {
		update_start();
	}
	if (old_cursor != new_cursor) {
		no::platform::set_system_cursor(new_cursor);
		old_cursor = new_cursor;
	}
}

void file_browser::update_start() {
	const auto window_size = window.size().to<float>() - top_left_position;
	no::ui::push_static_window("##start", top_left_position, window_size);
	no::ui::new_line();

	no::ui::text("Input the directory you wish to open by default.");
	no::ui::input("##config-default-path", config.default_open_path);
	bool valid{ std::filesystem::is_directory(config.default_open_path) };
	if (!valid) {
		no::ui::begin_disabled();
	}
	if (no::ui::button("Save")) {
		load_directory(config.default_open_path);
	}
	if (!valid) {
		no::ui::end_disabled();
		no::ui::inline_next();
		no::ui::colored_text({ 1.0f, 0.2f, 0.2f }, "Not a valid directory.");
	}
	no::ui::separate();

	no::ui::text("You can also select from these:");
	for (const auto& path : root_directories) {
		if (no::ui::button(path.u8string(), 64.0f)) {
			config.default_open_path = path.u8string();
			load_directory(config.default_open_path);
		}
		if (ImGui::IsItemHovered()) {
			new_cursor = no::platform::system_cursor::hand;
		}
		no::ui::inline_next();
	}

	no::ui::pop_window();
}

void file_browser::update_entries() {
	const auto window_size = window.size().to<float>() - top_left_position;
	entry_full_size = entry_size + entry_margin;
	no::ui::push_static_window("##files", top_left_position, window_size);
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

	const int column_count{ static_cast<int>(window_size.x / entry_full_size.x) };
	const int entry_count{ static_cast<int>(entries.size()) };
	const int last_column_count{ entry_count % column_count };
	int total_rows{ entry_count / column_count };
	if (last_column_count > 0) {
		total_rows++;
	}
	ImGuiListClipper clipper{ total_rows };
	while (clipper.Step()) {
		for (int row{ clipper.DisplayStart }; row < clipper.DisplayEnd; row++) {
			for (int column{ 0 }; column < column_count; column++) {
				int entry_index{ row * column_count + column };
				if (entry_index < entry_count) {
					auto& entry = entries[entry_index];
					directory_entry_control(entry);
					entry.visible = true;
				}
			}
		}
	}

	ImGui::PopStyleVar(2);
	ImGui::EndGroup();
	no::ui::pop_window();

	for (auto& entry : entries) {
		if (entry.double_clicked) {
			if (const auto path = entry.path; std::filesystem::is_directory(path)) {
				if (config.double_click_opens_directories) {
					load_directory(path);
				}
			} else {
				if (config.double_click_opens_files) {
					no::platform::open_file(path, false);
				}
			}
			break;
		} else if (entry.left_clicked) {
			if (keyboard.is_key_down(no::key::left_shift)) {
				if (!single_selected_entry) {
					single_selected_entry = &entries[0];
				} else {
					clear_selection();
				}
				for (auto middle : entries_between(single_selected_entry, &entry)) {
					middle->selected = true;
				}
			} else {
				if (!keyboard.is_key_down(no::key::left_control)) {
					clear_selection();
				}
				if (selected_entries().empty()) {
					single_selected_entry = &entry;
				}
				entry.selected = !entry.selected;
			}
			break;
		} else if (entry.right_clicked) {
			if (!entry.selected) {
				clear_selection();
			}
			entry.selected = true;
			context_entry = &entry;
			single_selected_entry = &entry;
			ImGui::OpenPopup("##entry-context");
			break;
		}
	}

	update_entry_context_menu();

	for (auto& entry : entries) {
		entry.update();
		entry.visible = false;
	}
}

bool file_browser::is_active() const {
	return !context_entry && !no::ui::is_hovered();
}

void file_browser::clear_entries() {
	entries.clear();
}

void file_browser::load_directory(const std::filesystem::path& path) {
	if (std::filesystem::is_directory(path)) {
		directory_history.push_back(path);
		entries = directory_entry::load_from_directory(path);
	} else {
		WARNING("Invalid directory: " << path);
		clear_entries();
	}
}

void file_browser::pop_history() {
	if (directory_history.size() > 1) {
		directory_history.pop_back();
		load_directory(directory_history.back());
		directory_history.pop_back();
	}
}

void file_browser::clear_selection() {
	for (auto& entry : entries) {
		entry.selected = false;
	}
}

void file_browser::select_all() {
	for (auto& entry : entries) {
		entry.selected = true;
	}
}

std::vector<directory_entry*> file_browser::selected_entries() {
	std::vector<directory_entry*> result;
	for (auto& entry : entries) {
		if (entry.selected) {
			result.push_back(&entry);
		}
	}
	return result;
}

std::vector<directory_entry*> file_browser::entries_between(directory_entry* from, directory_entry* to) {
	if (from == to) {
		return { from };
	}
	if (from > to) {
		std::swap(from, to);
	}
	std::vector<directory_entry*> result;
	for (auto& entry : entries) {
		if (&entry == from || !result.empty()) {
			result.push_back(&entry);
		}
		if (&entry == to) {
			break;
		}
	}
	return result;
}

void file_browser::directory_entry_control(directory_entry& entry) {
	ImGui::BeginGroup();
	const no::vector2f top_left_cursor{ ImGui::GetCursorScreenPos() };
	auto tag_cursor = top_left_cursor + 4.0f;
	const float file_name_text_width{ ImGui::CalcTextSize(entry.file_name().c_str()).x };
	const float file_name_rows{ std::floor(file_name_text_width / entry_size.x) };
	const float file_name_height{ ImGui::GetTextLineHeight() * file_name_rows };
	auto name_cursor = top_left_cursor;
	name_cursor.y += entry_size.y - 20.0f - file_name_height;
	no::vector4f default_color{ 0.2f, 0.2f, 0.2f, 0.2f };
	auto current_color = default_color;

	ImGui::SetCursorScreenPos(top_left_cursor);
	ImGui::InvisibleButton(CSTRING("entry" << &entry), entry_size);
	entry.double_clicked = false;
	entry.left_clicked = false;
	entry.right_clicked = false;
	entry.hovered = false;
	if (ImGui::IsItemHovered()) {
		entry.hovered = true;
		new_cursor = no::platform::system_cursor::hand;
		current_color = config.entry_hover_color;
		if (ImGui::IsMouseDoubleClicked(0)) {
			entry.double_clicked = true;
		} else if (ImGui::IsMouseClicked(0)) {
			entry.left_clicked = true;
		} else if (ImGui::IsMouseClicked(1)) {
			entry.right_clicked = true;
		}
	}
	if (entry.selected) {
		current_color = config.entry_hover_color;
	}

	// Draw background
	no::ui::rectangle(top_left_cursor, entry_size, current_color);
	no::ui::outline(top_left_cursor, entry_size, { 0.2f, 0.2f, 0.2f, 1.0f });

	// Draw thumbnail
	if (entry.thumbnail_texture != -1) {
		auto thumbnail_size = no::texture_size(entry.thumbnail_texture).to<float>();
		while (thumbnail_size.x > entry_size.x) {
			thumbnail_size *= 0.9f;
		}
		while (entry_size.y > file_name_height && thumbnail_size.y > entry_size.y - file_name_height) {
			thumbnail_size *= 0.9f;
		}
		auto image_cursor = top_left_cursor + entry_size / 2.0f - thumbnail_size / 2.0f;
		ImGui::SetCursorScreenPos(image_cursor);
		ImGui::Image(reinterpret_cast<ImTextureID>(entry.thumbnail_texture), thumbnail_size);
	}

	// Draw tags
	ImGui::SetCursorScreenPos(tag_cursor);
	for (const auto& tag : entry.get_tags()) {
		if (auto tag_data = tags::find_tag(tag)) {
			const auto tag_name = config.show_pretty_name ? tag_data->pretty_name : tag_data->name;
			const no::vector2f tag_image_size = ImGui::CalcTextSize(tag_name.c_str());
			no::vector2f tag_cursor_bg = ImGui::GetCursorScreenPos();
			no::ui::rectangle(tag_cursor_bg - 2.0f, tag_image_size + 4.0f, tag_data->background_color);
			no::ui::outline(tag_cursor_bg - 2.0f, tag_image_size + 4.0f, tag_data->text_color.with_w(0.25f));
			no::ui::colored_text(tag_data->text_color, tag_name);
			if (tag_cursor_bg.x + tag_image_size.x * 2.0f < top_left_cursor.x + entry_size.x) {
				no::ui::inline_next();
			} else {
				ImGui::SetCursorScreenPos(no::vector2f{ ImGui::GetCursorScreenPos() } + 4.0f);
			}
		}
	}

	// Draw file name
	ImGui::SetCursorScreenPos(name_cursor);
	ImGui::PushTextWrapPos(name_cursor.x - 48.0f);
	no::ui::text("%s", entry.file_name().c_str());
	ImGui::PopTextWrapPos();

	ImGui::EndGroup();
	if (top_left_cursor.x + entry_full_size.x < ImGui::GetWindowWidth()) {
		no::ui::inline_next();
	}
}

void file_browser::update_entry_context_menu() {
	if (!context_entry) {
		return;
	}
	auto selection = selected_entries();
	const auto path = context_entry->path;
	std::vector<no::ui::popup_item> items;
	if (selection.size() == 1) {
		if (std::filesystem::is_directory(path)) {
			items.emplace_back("Open directory", "", false, true, [&] {
				load_directory(path);
			});
		} else {
			items.emplace_back("Open file", "", false, true, [&] {
				no::platform::open_file(path, false);
			});
		}
		items.emplace_back("Show in file explorer", "", false, false);
		items.emplace_back("Rename", "", false, false);
	}
	std::vector<no::ui::popup_item> tag_group_items;
	for (const auto& group : tags::get_all_groups()) {
		std::vector<no::ui::popup_item> tag_items;
		for (const auto& tag : tags::get_all_tags_in_group(group)) {
			auto tag_data = tags::find_tag(tag);
			const auto tag_name = config.show_pretty_name ? tag_data->pretty_name : tag_data->name;
			tag_items.emplace_back(tag_name, "", false, true, [this, tag] {
				for (auto selected_entry : selected_entries()) {
					selected_entry->add_tag(tag);
				}
			});
		}
		tag_group_items.emplace_back(group, "", false, true, [] {}, tag_items);
	}
	items.emplace_back("Add tags", "", false, true, [] {}, tag_group_items);
	std::set<std::string> unique_tags;
	for (auto selected_entry : selected_entries()) {
		for (const auto& tag : selected_entry->get_tags()) {
			unique_tags.insert(tag);
		}
	}
	std::vector<no::ui::popup_item> tags_to_remove;
	for (const auto& tag : unique_tags) {
		tags_to_remove.emplace_back(tag, "", false, true, [this, tag] {
			for (auto selected_entry : selected_entries()) {
				selected_entry->remove_tag(tag);
			}
		});
	}
	items.emplace_back("Remove tags", "", false, true, [] {}, tags_to_remove);
	
	no::ui::popup("##entry-context", items);
	if (!ImGui::IsPopupOpen("##entry-context")) {
		context_entry = nullptr;
	}
}
