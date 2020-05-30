#include "tags.hpp"

#include "ui.hpp"
#include "draw.hpp"
#include "assets.hpp"
#include "font.hpp"

#include <unordered_map>

namespace tags {

struct tag_group {
	std::string name;
	std::vector<file_tag> tags;
};

static std::unordered_map<std::string, tag_group> groups;

void load() {
	// todo: milky.tags should be a text format. maybe json? doing binary atm since it's easiest.
	no::io_stream stream;
	no::file::read(no::asset_path("milky.tags"), stream);
	if (stream.empty()) {
		create_group("default");
		create_tag("default", "important");
		create_tag("default", "note");
		create_tag("default", "funny");
		create_tag("default", "wallpaper");
		create_tag("default", "cats");
		return;
	}
	const auto group_count = stream.read<int32_t>();
	for (int32_t group_index{ 0 }; group_index < group_count; group_index++) {
		const auto group_name = stream.read<std::string>();
		const auto tag_count = stream.read<int32_t>();
		auto& group = groups[group_name];
		for (int32_t tag_index{ 0 }; tag_index < tag_count; tag_index++) {
			tags::file_tag tag;
			tag.name = stream.read<std::string>();
			tag.pretty_name = stream.read<std::string>();
			tag.description = stream.read<std::string>();
			tag.background_color = stream.read<no::vector4f>();
			tag.text_color = stream.read<no::vector4f>();
			if (!find_tag(tag.name)) {
				group.tags.push_back(tag);
			} else {
				WARNING("Discarded duplicate tag " << tag.name);
			}
		}
	}
}

void save() {
	no::io_stream stream;
	stream.write(static_cast<int32_t>(groups.size()));
	for (const auto& [group_name, group] : groups) {
		stream.write(group_name);
		stream.write(static_cast<int32_t>(group.tags.size()));
		for (const auto& tag : group.tags) {
			stream.write(tag.name);
			stream.write(tag.pretty_name);
			stream.write(tag.description);
			stream.write(tag.background_color);
			stream.write(tag.text_color);
		}
	}
	no::file::write(no::asset_path("milky.tags"), stream);
}

void create_group(const std::string& name) {
	ASSERT(!group_exists(name));
	groups.try_emplace(name);
	tags::save();
}

void create_tag(const std::string& group, const std::string& tag) {
	if (!find_tag(tag)) {
		auto& new_tag = groups[group].tags.emplace_back();
		new_tag.name = tag;
		new_tag.pretty_name = tag;
		tags::save();
	}
}

void delete_tag(const std::string& name) {
	for (auto& [group_name, group] : groups) {
		auto erased_tag = std::remove_if(group.tags.begin(), group.tags.end(), [name](auto& tag) {
			return tag.name == name;
		});
		if (erased_tag != group.tags.end()) {
			group.tags.erase(erased_tag);
			break;
		}
	}
}

std::vector<std::string> get_all_groups() {
	return no::get_map_keys(groups);
}

bool group_exists(const std::string& name) {
	return groups.find(name) != groups.end();
}

std::vector<std::string> get_all_tags_in_group(const std::string& name) {
	std::vector<std::string> tags;
	if (const auto group = groups.find(name); group != groups.end()) {
		for (const auto& tag : group->second.tags) {
			tags.push_back(tag.name);
		}
	}
	return tags;
}

std::vector<std::string> get_all_tags() {
	std::vector<std::string> tags;
	for (const auto& [name, group] : groups) {
		for (const auto& tag : group.tags) {
			tags.push_back(tag.name);
		}
	}
	return tags;
}

std::optional<file_tag> find_tag(const std::string& name) {
	for (const auto& [group_name, group] : groups) {
		for (const auto& tag : group.tags) {
			if (tag.name == name) {
				return tag;
			}
		}
	}
	return std::nullopt;
}

bool replace_tag(const std::string& tag_to_replace, const file_tag& new_tag) {
	if (find_tag(new_tag.name) && tag_to_replace != new_tag.name) {
		return false; // a tag with the new name already exists, and it's not the one being replaced.
	}
	for (auto& [group_name, group] : groups) {
		for (auto& tag : group.tags) {
			if (tag.name == tag_to_replace) {
				tag = new_tag;
				tags::save();
				return true;
			}
		}
	}
	return false;
}

std::optional<std::string> find_group_with_tag(const std::string& tag_name) {
	for (auto& [group_name, group] : groups) {
		for (auto& tag : group.tags) {
			if (tag.name == tag_name) {
				return group_name;
			}
		}
	}
	return std::nullopt;
}

std::string find_tag_string_in_path(const std::string& path) {
	if (size_t start{ path.find('[') }; start != std::string::npos) {
		if (size_t end{ path.find(']', start) }; end != std::string::npos) {
			return path.substr(start + 1, end - start - 1);
		}
	}
	return "";
}

std::string filename_without_tags(const std::string& filename) {
	return no::erase_substring(filename, "[" + find_tag_string_in_path(filename) + "]");
}

}

void tag_system_ui::update() {
	ImGui::PushID("tag-system");
	ImGui::PushItemWidth(144.0f);
	groups_ui.update();
	no::ui::separate();
	new_tag_group = no::ui::combo("##tag-group", tags::get_all_groups(), new_tag_group).value_or(new_tag_group);
	no::ui::inline_next();
	no::ui::input("##new-tag-name", new_tag_name);
	ImGui::PopItemWidth();
	no::ui::inline_next();
	if (no::ui::button("+##save-new-tag")) {
		tags::create_tag(tags::get_all_groups()[new_tag_group], new_tag_name);
		new_tag_name = "";
	}
	for (auto& group : tags::get_all_groups()) {
		ImGui::PushID(group.c_str());
		if (ImGui::CollapsingHeader(group.c_str())) {
			const int selected_in_group{ group == selected_group ? selected_tag : -1 };
			if (const auto new_selected_tag = no::ui::list("##tags", tags::get_all_tags_in_group(group), selected_in_group)) {
				selected_group = group; // todo: set group to proper value
				selected_tag = new_selected_tag.value();
				manage_ui.open(tags::get_all_tags_in_group(group)[selected_tag]);
			}
		}
		ImGui::PopID();
	}
	manage_ui.update();
	ImGui::PopID();
}

void manage_tag_ui::open(const std::string& name) {
	close();
	tag = tags::find_tag(name);
	original_name = name;
	selected_group = 0;
	// todo: this really needs to be cleaned up lol... searching like this smh
	if (auto group = tags::find_group_with_tag(name)) {
		for (const auto& group_name : tags::get_all_groups()) {
			if (group_name == group) {
				break;
			}
			selected_group++;
		}
	}
}

void manage_tag_ui::close() {
	tag = std::nullopt;
	original_name = "";
}

void manage_tag_ui::update() {
	if (!tag.has_value()) {
		return;
	}
	no::ui::separate();
	ImGui::PushID("manage-tag");
	no::ui::input("Name", tag->name);
	no::ui::input("Pretty name", tag->pretty_name);
	no::ui::text("Description");
	no::ui::input("##description", tag->description, { -1.0f, 64.0f });

	const auto old_group = selected_group;
	selected_group = no::ui::combo("##group", tags::get_all_groups(), selected_group).value_or(selected_group);
	if (old_group != selected_group) {
		const auto selected_group_name = tags::get_all_groups()[selected_group];
		tags::delete_tag(tag->name);
		tags::create_tag(selected_group_name, tag->name); // todo: make this accept the tag instead of string?
		tags::replace_tag(tag->name, tag.value());
	}

	ImGui::ColorEdit3("Background", &tag->background_color.x);
	ImGui::ColorEdit3("Text", &tag->text_color.x);
	auto temporary_tag = tag.value();
	temporary_tag.name = original_name;
	tags::replace_tag(original_name, temporary_tag);
	if (no::ui::button("Delete")) {
		tags::delete_tag(tag->name);
		tag = std::nullopt;
	}
	ImGui::PopID();
}

void manage_tag_groups_ui::update() {
	ImGui::PushID("manage-groups");
	const bool any_group_is_being_renamed{ !new_group_name.empty() };
	if (any_group_is_being_renamed) {
		no::ui::begin_disabled();
	}
	no::ui::input("##new-group", new_group_name_to_create);
	if (any_group_is_being_renamed) {
		no::ui::end_disabled();
	}
	const bool new_group_already_exists{ tags::groups.find(new_group_name_to_create) != tags::groups.end() };
	if (new_group_already_exists) {
		no::ui::begin_disabled();
	}
	no::ui::inline_next();
	if (no::ui::button("Create group")) {
		tags::create_group(new_group_name_to_create);
		new_group_name_to_create = "";
	}
	if (new_group_already_exists) {
		no::ui::end_disabled();
		no::ui::colored_text({ 1.0f, 0.8f, 0.8f }, "Group already exists.");
	}
	for (const auto& group : tags::get_all_groups()) {
		const bool is_default_group{ group == "default" };
		ImGui::PushID(group.c_str());
		if (is_default_group || any_group_is_being_renamed) {
			no::ui::begin_disabled();
		}
		if (no::ui::button("Delete")) {
			const auto& tags_in_group = tags::groups[group].tags;
			auto& destination_tags = tags::groups["default"].tags;
			destination_tags.insert(destination_tags.end(), tags_in_group.begin(), tags_in_group.end());
			tags::groups.erase(group);
		}
		if (is_default_group || any_group_is_being_renamed) {
			no::ui::end_disabled();
		}
		no::ui::inline_next();
		if (group_to_rename.empty()) {
			no::ui::inline_next();
			if (is_default_group || any_group_is_being_renamed) {
				no::ui::begin_disabled();
			}
			if (no::ui::button("Rename")) {
				group_to_rename = group;
				new_group_name = group;
			}
			if (is_default_group || any_group_is_being_renamed) {
				no::ui::end_disabled();
			}
			no::ui::inline_next();
			no::ui::text(group);
		} else if (group_to_rename == group) {
			no::ui::text(group);
			no::ui::input("##new-name", new_group_name);
			const bool group_already_exists{ tags::groups.find(new_group_name) != tags::groups.end() };
			if (group_already_exists) {
				no::ui::begin_disabled();
			}
			no::ui::inline_next();
			if (no::ui::button("Save") && !group_already_exists) {
				tags::groups[new_group_name] = tags::groups[group_to_rename];
				tags::groups.erase(group_to_rename);
			}
			if (group_already_exists) {
				no::ui::end_disabled();
			}
			no::ui::inline_next();
			if (no::ui::button("Cancel")) {
				group_to_rename = "";
				new_group_name = "";
			}
			if (group_already_exists) {
				no::ui::colored_text({ 1.0f, 0.8f, 0.8f }, "Group already exists.");
			}
		}
		ImGui::PopID();
	}
	ImGui::PopID();
}
