#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <future>

class file_browser;

class search_path_cache {
public:

	search_path_cache(const std::filesystem::path& path);
	search_path_cache(const search_path_cache&) = delete;
	search_path_cache(search_path_cache&&) noexcept;

	search_path_cache& operator=(const search_path_cache&) = delete;
	search_path_cache& operator=(search_path_cache&&) = delete;

	const std::filesystem::path& directory() const;
	const std::vector<std::filesystem::path>& paths();

private:

	const std::filesystem::path search_path;
	std::vector<std::filesystem::path> cached_paths;
	std::future<std::vector<std::filesystem::path>> future_paths;

};

class search_path_cache_list {
public:

	std::vector<search_path_cache> caches;

	search_path_cache_list() = default;
	search_path_cache_list(const search_path_cache_list&) = delete;
	search_path_cache_list(search_path_cache_list&&) = delete;

	search_path_cache_list& operator=(const search_path_cache_list&) = delete;
	search_path_cache_list& operator=(search_path_cache_list&&) = delete;

	void add_search_directory(const std::filesystem::path& path);
	std::vector<std::filesystem::path> directories() const;

};

class search_ui {
public:

	search_path_cache_list cache_list;
	
	void update(file_browser& browser);

private:

	void select_tag_popup(std::string_view popup_id, bool include);
	void update_browser(file_browser& browser);

	bool must_update_browser{ false };
	std::vector<std::string> include_tags;
	std::vector<std::string> exclude_tags;


};
