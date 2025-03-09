#include "sound_skipping.hpp"

#include "..\game\g_local.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>


namespace speedrun {
namespace {

struct transparent_hash_string_view : public std::hash<std::string_view> {
	using is_transparent = void;
};

struct transparent_equal_to_string_view : public std::equal_to<std::string_view> {
	using is_transparent = void;
};

std::unordered_set<std::string, transparent_hash_string_view, transparent_equal_to_string_view> sounds_to_skip;

class File {
public:
	explicit File(const char* filename) {
		const int len = gi.FS_ReadFile(filename, &buffer);
		if (len == -1) {
			buffer = nullptr;
		}
	}

	~File() {
		if (buffer != nullptr) {
			gi.FS_FreeFile(buffer);
		}
	}

	std::string_view content() const {
		if (!buffer) {
			return {};
		}
		return static_cast<const char*>(buffer);
	}

private:
	void* buffer = nullptr;
};

}  // anonymous namespace


void InitSoundSkipping() {
	sounds_to_skip.clear();

	const File file("speedrun/sounds_to_remove.txt");
	const std::string_view content = file.content();
	auto line_start = content.begin();
	while (line_start != content.end()) {
		const auto line_end = std::find(line_start, content.end(), '\n');
		std::filesystem::path sound{line_start, line_end};
		sound.replace_extension(".wav");
		sounds_to_skip.insert(sound.string());
		if (line_end == content.end()) {
			break;
		}
		line_start = line_end + 1;
	}
}

bool IsSoundToSkip(const char* soundname) {
	return sounds_to_skip.contains(soundname);
}

}  // namespace speedrun
