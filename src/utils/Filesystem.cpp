#include "Filesystem.hpp"
#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode;

namespace std::filesystem {
    path operator/(path&& lhs, const path& rhs) {
        #ifdef GEODE_IS_WINDOWS
        auto& left = lhs.native();
        auto& right = rhs.native();
        if (!right.empty() && !_Has_drive_letter_prefix(right.data(), right.data() + right.size()) && !_Is_slash(right[0])) {
            auto needsSlash = !left.empty() && (left.size() != 2 || !_Is_drive_prefix(left.data())) && !_Is_slash(left.back());
            const_cast<std::wstring&>(left).resize_and_overwrite(left.size() + right.size() + (needsSlash ? 1 : 0), [
                &left, &right, needsSlash
            ](wchar_t* dest, size_t size) {
                ::memcpy(dest, left.data(), left.size() * sizeof(wchar_t));
                dest += left.size();
                if (needsSlash) {
                    *dest = L'\\';
                    dest++;
                }
                ::memcpy(dest, right.data(), right.size() * sizeof(wchar_t));
                return size;
            });
            return std::move(lhs);
        }
        #endif
        lhs /= rhs;
        return std::move(lhs);
    }
}

bool Filesystem::doesExist(const std::filesystem::path& path) {
    std::error_code code;
    return std::filesystem::exists(path, code);
}

#ifdef GEODE_IS_WINDOWS
std::wstring Filesystem::strWide(std::string_view path) {
    auto count = MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), nullptr, 0);
    std::wstring str(count, L'\0');
    if (count != 0) MultiByteToWideChar(CP_UTF8, 0, path.data(), path.size(), &str[0], count);
    return str;
}

std::string Filesystem::strNarrow(std::wstring_view wstr) {
    auto count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string str(count, '\0');
    if (count != 0) WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, &str[0], count, nullptr, nullptr);
    return str;
}
#else
std::string Filesystem::strWide(std::string_view str) {
    return std::string(str);
}

std::string Filesystem::strNarrow(std::string_view str) {
    return std::string(str);
}
#endif

std::filesystem::path Filesystem::strPath(std::string_view path) {
    return std::filesystem::path(strWide(path));
}

#ifdef GEODE_IS_WINDOWS
std::wstring_view _filename(const std::filesystem::path& path) {
    return std::filesystem::_Parse_filename(path.native());
}

std::wstring_view _parent_path(const std::filesystem::path& path) {
    return std::filesystem::_Parse_parent_path(path.native());
}
#else
template <std::string_view(std::filesystem::path::*func)() const>
struct FilenameCaller {
    friend std::string_view _filename(const std::filesystem::path& path) {
        return (path.*func)();
    }
};

template struct FilenameCaller<&std::filesystem::path::__filename>;

std::string_view _filename(const std::filesystem::path& path);

template <std::string_view(std::filesystem::path::*func)() const>
struct ParentCaller {
    friend std::string_view _parent_path(const std::filesystem::path& path) {
        return (path.*func)();
    }
};

template struct ParentCaller<&std::filesystem::path::__parent_path>;

std::string_view _parent_path(const std::filesystem::path& path);
#endif

std::basic_string_view<std::filesystem::path::value_type> Filesystem::filenameView(const std::filesystem::path& path, size_t removeCount) {
    auto filename = _filename(path);
    filename.remove_suffix(removeCount);
    return filename;
}

std::filesystem::path::string_type& getPathString(std::filesystem::path& path) {
    return const_cast<std::filesystem::path::string_type&>(path.native());
}

std::filesystem::path Filesystem::filenamePath(const std::filesystem::path& path) {
    return std::filesystem::path(std::basic_string(_filename(path)));
}

std::filesystem::path Filesystem::filenamePath(std::filesystem::path&& path) {
    auto filename = _filename(path);
    return std::filesystem::path(std::move(getPathString(path)).substr(filename.data() - path.c_str(), filename.size()));
}

std::filesystem::path Filesystem::parentPath(const std::filesystem::path& path) {
    return std::filesystem::path(std::basic_string(_parent_path(path)));
}

std::filesystem::path Filesystem::parentPath(std::filesystem::path&& path) {
    auto parent = _parent_path(path);
    return std::filesystem::path(std::move(getPathString(path)).substr(parent.data() - path.c_str(), parent.size()));
}

#ifdef GEODE_IS_WINDOWS
std::string Filesystem::filenameFormat(const std::filesystem::path& path) {
    return strNarrow(_filename(path));
}
#else
std::string_view Filesystem::filenameFormat(const std::filesystem::path& path) {
    return _filename(path);
}
#endif

Result<> Filesystem::renameFile(const std::filesystem::path& from, const std::filesystem::path& to) {
    if (doesExist(from)) {
        std::error_code code;
        if (doesExist(to) && !std::filesystem::remove(to, code)) {
            return Err(fmt::format("Failed to remove {}: {}", Filesystem::filenameFormat(to), code.message()));
        }
        std::filesystem::rename(from, to, code);
        if (code) return Err(fmt::format("Failed to rename {}: {}", Filesystem::filenameFormat(from), code.message()));
    }
    return Ok();
}

void Filesystem::iterate(
    const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func
) {
    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return;
    for (; it != std::filesystem::end(it); it.increment(code)) {
        std::error_code code;
        if (it->status(code).type() != type) continue;
        func(it->path());
    }
}
