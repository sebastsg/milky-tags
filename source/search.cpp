#include "search.hpp"
#include "tags.hpp"
#include "browser.hpp"
#include "ui.hpp"

void search_ui::select_tag_popup(std::string_view popup_id, bool include) {
	if (!ImGui::IsPopupOpen(popup_id.data())) {
		return;
	}
	std::vector<no::ui::popup_item> group_items;
	for (const auto& group : tags::get_all_groups()) {
		std::vector<no::ui::popup_item> tag_items;
		for (const auto& tag : tags::get_all_tags_in_group(group)) {
			auto tag_data = tags::find_tag(tag);
			tag_items.emplace_back(tag_data->pretty_name, "", false, true, [this, tag, include] {
				if (include) {
					include_tags.push_back(tag);
				} else {
					exclude_tags.push_back(tag);
				}
				must_update_browser = true;
			});
		}
		group_items.emplace_back(group, "", false, true, [] {}, tag_items);
	}
	no::ui::popup(popup_id, group_items);
}

void search_ui::update(file_browser& browser) {
	if (cache_list.caches.empty()) {
		if (!browser.config.default_open_path.empty()) {
			cache_list.add_search_directory(browser.config.default_open_path);
		}
		return;
	}
	ImGui::PushID("search");
	no::ui::separate();
	no::ui::text("Include tags:");
	no::ui::inline_next();
	for (int i{ 0 }; i < static_cast<int>(include_tags.size()); i++) {
		const auto tag = tags::find_tag(include_tags[i]);
		if (no::ui::button(tag->pretty_name)) {
			include_tags.erase(include_tags.begin() + i);
			i--;
			must_update_browser = true;
		}
		no::ui::inline_next();
	}
	if (no::ui::button("+##open-context-include")) {
		ImGui::OpenPopup("##context-include-tag");
	}
	select_tag_popup("##context-include-tag", true);
	no::ui::new_line();
	no::ui::text("Exclude tags:");
	no::ui::inline_next();
	for (int i{ 0 }; i < static_cast<int>(exclude_tags.size()); i++) {
		const auto tag = tags::find_tag(exclude_tags[i]);
		if (no::ui::button(tag->pretty_name)) {
			exclude_tags.erase(exclude_tags.begin() + i);
			i--;
			must_update_browser = true;
		}
		no::ui::inline_next();
	}
	if (no::ui::button("+##open-context-exclude")) {
		ImGui::OpenPopup("##context-exclude-tag");
	}
	select_tag_popup("##context-exclude-tag", false);
	no::ui::new_line();
	ImGui::PopID();
	update_browser(browser);
}

void search_ui::update_browser(file_browser& browser) {
	if (!must_update_browser) {
		return;
	}
	must_update_browser = false;
	std::vector<std::filesystem::path> paths;
	for (const auto& cache : cache_list.caches) {
		for (const auto& path : cache.paths()) {
			directory_entry entry{ path, false }; // todo: make function to specifically check the tags
			auto tag_predicate = std::bind(&directory_entry::has_tag, &entry, std::placeholders::_1);
			if (!std::all_of(include_tags.begin(), include_tags.end(), tag_predicate)) {
				continue;
			}
			if (std::any_of(exclude_tags.begin(), exclude_tags.end(), tag_predicate)) {
				continue;
			}
			paths.emplace_back(path);
		}
	}
	browser.load_paths(paths);
}

search_path_cache_list::search_path_cache_list() {
	// todo: load stored cache
}

void search_path_cache_list::rebuild() {
	for (auto& cache : caches) {
		cache.rebuild();
	}
}

void search_path_cache_list::add_search_directory(const std::filesystem::path& directory) {
	for (const auto& cache : caches) {
		if (std::filesystem::equivalent(cache.directory(), directory)) {
			return;
		}
	}
	caches.push_back({ directory });
}

std::vector<std::filesystem::path> search_path_cache_list::directories() const {
	std::vector<std::filesystem::path> paths;
	for (const auto& cache : caches) {
		paths.push_back(cache.directory());
	}
	return paths;
}

search_path_cache::search_path_cache(const std::filesystem::path& path) : search_path{ path } {
	rebuild();
}

void search_path_cache::rebuild() {
	cached_paths = std::move(no::entries_in_directory(search_path, no::entry_inclusion::everything, true));
	std::vector<std::filesystem::path> system_directories;
	for (int i{ 0 }; i < static_cast<int>(cached_paths.size()); i++) {
		if (no::platform::is_system_file(cached_paths[i])) {
			if (std::filesystem::is_directory(cached_paths[i])) {
				system_directories.push_back(cached_paths[i]);
			}
			std::swap(cached_paths[i], cached_paths.back());
			cached_paths.pop_back();
			i--;
		}
	}
	for (const auto& system_directory : system_directories) {
		for (int i{ 0 }; i < static_cast<int>(cached_paths.size()); i++) {
			if (cached_paths[i].parent_path() == system_directory) {
				std::swap(cached_paths[i], cached_paths.back());
				cached_paths.pop_back();
				i--;
			}
		}
	}
}

const std::filesystem::path& search_path_cache::directory() const {
	return search_path;
}

const std::vector<std::filesystem::path>& search_path_cache::paths() const {
	return cached_paths;
}
