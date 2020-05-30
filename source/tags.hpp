#pragma once

#include "math.hpp"

#include <string>
#include <vector>
#include <optional>

namespace tags {

struct file_tag {
	std::string name;
	std::string pretty_name;
	std::string description;
	no::vector4f background_color{ 0.3f, 0.3f, 0.3f, 1.0f };
	no::vector4f text_color{ 1.0f };
};

void load();
void save();
void create_group(const std::string& name);
void create_tag(const std::string& group, const std::string& tag);
void delete_tag(const std::string& tag);
std::vector<std::string> get_all_groups();
bool group_exists(const std::string& name);
std::vector<std::string> get_all_tags_in_group(const std::string& name);
std::vector<std::string> get_all_tags();
std::optional<file_tag> find_tag(const std::string& name);
bool replace_tag(const std::string& name, const file_tag& tag);
std::optional<std::string> find_group_with_tag(const std::string& tag);

std::string find_tag_string_in_path(const std::string& path);
std::string filename_without_tags(const std::string& filename);

}

class manage_tag_ui {
public:

	void open(const std::string& tag);
	void close();
	void update();

private:

	std::string original_name;
	std::optional<tags::file_tag> tag;
	int selected_group{ 0 };

};

class manage_tag_groups_ui {
public:

	void update();

private:

	std::string group_to_rename;
	std::string new_group_name;
	std::string new_group_name_to_create;

};

class tag_system_ui {
public:

	void update();

private:

	std::string new_tag_name;
	int new_tag_group{ 0 };

	std::string selected_group;
	int selected_tag{ 0 };

	manage_tag_ui manage_ui;
	manage_tag_groups_ui groups_ui;

};
