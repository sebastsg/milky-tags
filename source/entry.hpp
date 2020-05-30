#pragma once

#include "transform.hpp"
#include "tags.hpp"
#include "surface.hpp"

#include <filesystem>
#include <future>

class directory_entry {
public:

	static std::vector<directory_entry> load_from_directory(const std::filesystem::path& path);
	static std::vector<std::string> parse_tags(const std::filesystem::path& path);

	std::filesystem::path path;
	no::transform2 transform;
	int thumbnail_texture{ -1 };
	bool hovered{ false };
	bool selected{ false };
	bool double_clicked{ false };
	bool left_clicked{ false };
	bool right_clicked{ false };
	bool visible{ false };

	directory_entry(const std::filesystem::path& path);
	directory_entry(const directory_entry&) = delete;
	directory_entry(directory_entry&&) = default;

	~directory_entry();

	directory_entry& operator=(const directory_entry&) = delete;
	directory_entry& operator=(directory_entry&&) = default;

	void update();
	void load_thumbnail();

	std::string tag_string() const;
	std::string file_name() const;

	bool is_rename_failing() const;

	void add_tag(const std::string& tag);
	void remove_tag(const std::string& tag);
	bool has_tag(const std::string& tag);
	std::vector<std::string> get_tags() const;

private:

	void rename_if_needed();

	std::string name;
	std::vector<std::string> tags;
	bool needs_rename{ false };
	bool rename_failed{ false };

	std::future<no::surface> future_thumbnail;

};
