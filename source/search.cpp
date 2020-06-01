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
	if (!ImGui::CollapsingHeader("Search##search-ui")) {
		return;
	}
	ImGui::PushID("search");
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
	no::timer filter_timer;
	filter_timer.start();
	for (auto& cache : cache_list.caches) {
		for (const auto& path : cache.paths()) {
			auto tags = directory_entry::parse_tags(path);
			auto tag_predicate = [tags] (const auto& tag) {
				return std::find(tags.begin(), tags.end(), tag) != tags.end();
			};
			if (std::all_of(include_tags.begin(), include_tags.end(), tag_predicate)) {
				if (!std::any_of(exclude_tags.begin(), exclude_tags.end(), tag_predicate)) {
					paths.emplace_back(path);
				}
			}
		}
	}
	INFO("Filtered in " << filter_timer.milliseconds() << " ms");
	browser.load_paths(paths);
}

search_path_cache::search_path_cache(const std::filesystem::path& path) : search_path{ path } {
	// might take a few seconds.
	future_paths = std::async(std::launch::async, no::entries_in_directory, path, no::entry_inclusion::everything, true);
}

search_path_cache::search_path_cache(search_path_cache&& that) noexcept : search_path{ that.search_path } {
	std::swap(cached_paths, that.cached_paths);
	std::swap(future_paths, that.future_paths);
}

const std::filesystem::path& search_path_cache::directory() const {
	return search_path;
}

const std::vector<std::filesystem::path>& search_path_cache::paths() {
	if (no::is_future_ready(future_paths)) {
		cached_paths = future_paths.get();
	}
	return cached_paths;
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