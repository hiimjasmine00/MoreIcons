#include "Filesystem.hpp"
#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode;

inline std::filesystem::path::string_type& getPathString(std::filesystem::path& path) {
    return const_cast<std::filesystem::path::string_type&>(path.native());
}

MI_FILESYSTEM_BEGIN
namespace filesystem {
    void appendPath(path& p, Filesystem::PathView right) {
        auto& left = getPathString(p);
        #ifdef GEODE_IS_WINDOWS
        if (!right.empty() && (right.size() < 2 || !_Is_drive_prefix(right.data())) && !_Is_slash(right[0])) {
            auto needsSlash = !left.empty() && (left.size() != 2 || !_Is_drive_prefix(left.data())) && !_Is_slash(left.back());
            return left.resize_and_overwrite(left.size() + right.size() + (needsSlash ? 1 : 0), [
                &left, right, needsSlash
            ](wchar_t* dest, size_t size) {
                dest += left.size();
                if (needsSlash) {
                    *dest = L'\\';
                    dest++;
                }
                ::memcpy(dest, right.data(), right.size() * sizeof(wchar_t));
                return size;
            });
        }

        if (right.size() >= 3 && _Is_drive_prefix(right.data()) && _Is_slash(right[2])) {
            left = right;
            return;
        }

        auto rightName = _Parse_root_name(right);
        if (!rightName.empty()) {
            left = right;
            return;
        }

        auto leftName = _Parse_root_name(left);
        if (right.size() != rightName.size() && _Is_slash(rightName.back())) {
            left.erase(leftName.size());
        }
        else if (left.size() == leftName.size()) {
            if (leftName.size() >= 3) left += L'\\';
        }
        else {
            if (!_Is_slash(left.back())) left += L'\\';
        }

        left += right.substr(rightName.size());
        #else
        if (!right.empty() && right[0] == '/') {
            left = right;
            return;
        }

        if (p.has_filename()) left += '/';
        left += right;
        #endif
    }

    path operator/(const path& lhs, Filesystem::PathView rhs) {
        path ret = lhs;
        appendPath(ret, rhs);
        return ret;
    }

    path operator/(path&& lhs, Filesystem::PathView rhs) {
        appendPath(lhs, rhs);
        return std::move(lhs);
    }
}
MI_FILESYSTEM_END

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
#endif

#ifdef GEODE_IS_WINDOWS
#define PRIVATE_WRAPPER(func) \
    inline std::wstring_view _##func(const std::filesystem::path& path) { \
        return std::filesystem::_Parse_##func(path.native()); \
    }
#else
#define PRIVATE_WRAPPER(func) \
    template <std::string_view(std::filesystem::path::*funcT)() const> \
    struct func##Caller { \
        inline friend std::string_view _##func(const std::filesystem::path& path) { \
            return (path.*funcT)(); \
        } \
    }; \
    template struct func##Caller<&std::filesystem::path::__##func>; \
    inline std::string_view _##func(const std::filesystem::path& path);
#endif

PRIVATE_WRAPPER(extension)
PRIVATE_WRAPPER(filename)
PRIVATE_WRAPPER(parent_path)

Filesystem::PathView Filesystem::filenameView(const std::filesystem::path& path) {
    return _filename(path);
}

std::filesystem::path Filesystem::parentPath(const std::filesystem::path& path) {
    return std::filesystem::path(std::filesystem::path::string_type(_parent_path(path)));
}

std::filesystem::path Filesystem::parentPath(std::filesystem::path&& path) {
    getPathString(path).erase(_parent_path(path).size());
    return std::move(path);
}

std::filesystem::path Filesystem::withExt(const std::filesystem::path& path, Filesystem::PathView ext) {
    std::filesystem::path ret = path;
    auto extension = _extension(ret);
    getPathString(ret).replace(extension.data() - ret.c_str(), extension.size(), ext);
    return ret;
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
