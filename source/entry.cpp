#include "entry.hpp"
#include "io.hpp"
#include "assets.hpp"
#include "platform.hpp"
#include "draw.hpp"

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

template<typename T>
std::vector<T> merge_vectors(const std::vector<T>& front, const std::vector<T>& back) {
	std::vector<T> result;
	result.reserve(front.size() + back.size());
	result.insert(result.end(), front.begin(), front.end());
	result.insert(result.end(), back.begin(), back.end());
	return result;
}

std::vector<directory_entry> directory_entry::load_from_directory(const std::filesystem::path& path) {
	std::vector<std::filesystem::path> directories;
	std::vector<std::filesystem::path> files;
	const auto blacklist = { "desktop.ini", "System Volume Information", "$RECYCLE.BIN" };
	for (const auto& path : no::entries_in_directory(path, no::entry_inclusion::everything, false)) {
		bool is_blacklisted{ false };
		for (const auto blacklisted : blacklist) {
			if (path.filename().u8string() == blacklisted) {
				is_blacklisted = true;
				break;
			}
		}
		if (is_blacklisted) {
			continue;
		}
		if (path.filename().u8string().front() == '.') {
			continue; // todo: this should be configurable.
		}
		if (std::filesystem::is_directory(path)) {
			directories.emplace_back(path);
		} else {
			files.emplace_back(path);
		}
	}
	std::vector<directory_entry> entries;
	for (const auto& path : merge_vectors(directories, files)) {
		entries.emplace_back(path);
	}
	return entries;
}

directory_entry::directory_entry(const std::filesystem::path& path) : path{ path } {
	auto font = no::require_font("seguiemj.ttf", 16);
	auto tag_string = find_tag_string_in_path(path.filename().u8string());
	for (auto& tag : no::split_string(tag_string, ' ')) {
		tags.push_back(tag);
	}
	name = filename_without_tags(path.filename().u8string());
	no::release_font("seguiemj.ttf", 16);
}

directory_entry::~directory_entry() {
	no::delete_texture(thumbnail_texture);
}

void directory_entry::update() {
	rename_if_needed();
	if (visible && thumbnail_texture == -1) {
		load_thumbnail();
	}
}

std::filesystem::path directory_entry::get_path() const {
	return path;
}

void directory_entry::load_thumbnail() {
	auto surface = no::platform::load_file_thumbnail(path, 256);
	thumbnail_texture = no::create_texture(surface, no::scale_option::linear, false);
}

std::string directory_entry::tag_string() const {
	if (tags.empty()) {
		return "";
	}
	std::string result{ "[" };
	for (const auto& tag : tags) {
		result += tag + " ";
	}
	result.back() = ']';
	return result;
}

std::string directory_entry::file_name() const {
	return name;
}

void directory_entry::rename_if_needed() {
	if (needs_rename) {
		needs_rename = false;
		rename_failed = false;
		auto new_path = path;
		new_path.remove_filename();
		new_path /= std::filesystem::u8path(tag_string() + file_name());
		std::error_code error;
		std::filesystem::rename(path, new_path, error);
		if (error) {
			WARNING("Failed to rename from " << path << " to " << new_path << ". Error: " << error.message());
			rename_failed = true;
		} else {
			path = new_path;
		}
	}
}

bool directory_entry::is_rename_failing() const {
	return rename_failed;
}

void directory_entry::add_tag(const std::string& tag) {
	if (!has_tag(tag)) {
		tags.push_back(tag);
		needs_rename = true;
	}
}

void directory_entry::remove_tag(const std::string& tag) {
	if (auto found_tag = std::find(tags.begin(), tags.end(), tag); found_tag != tags.end()) {
		tags.erase(found_tag);
		needs_rename = true;
	}
}

bool directory_entry::has_tag(const std::string& tag) {
	return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

std::vector<std::string> directory_entry::get_tags() const {
	return tags;
}
