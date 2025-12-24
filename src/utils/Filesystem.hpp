#include <filesystem>
#include <fmt/format.h>
#include <Geode/platform/cplatform.h>
#ifdef GEODE_IS_WINDOWS
#include <fmt/xchar.h>
#endif
#include <Geode/Result.hpp>
#include <std23/function_ref.h>

#ifdef GEODE_IS_WINDOWS
#define L(x) L##x
#else
#define L(x) x
#endif

namespace std::filesystem {
    void appendPath(path& lhs, basic_string_view<path::value_type> rhs);

    template <class T>
    path operator/(const path& lhs, const T& rhs) {
        path ret = lhs;
        appendPath(ret, rhs);
        return ret;
    }

    template <class T>
    path operator/(path&& lhs, const T& rhs) {
        appendPath(lhs, rhs);
        return std::move(lhs);
    }
}

class Filesystem {
public:
    using PathView = std::basic_string_view<std::filesystem::path::value_type>;

    static bool doesExist(const std::filesystem::path& path);
    #ifdef GEODE_IS_WINDOWS
    static std::string strNarrow(std::wstring_view str);
    static std::wstring strWide(std::string_view str);
    static std::string filenameFormat(const std::filesystem::path& path);
    #else
    static std::string_view strNarrow(std::string_view str) {
        return str;
    }
    static std::string_view strWide(std::string_view str) {
        return str;
    }
    static std::string_view filenameFormat(const std::filesystem::path& path);
    #endif
    static std::filesystem::path strPath(std::string_view path);
    #ifndef GEODE_IS_WINDOWS
    static std::filesystem::path strPath(std::string&& path);
    #endif
    static PathView filenameView(const std::filesystem::path& path);
    static std::filesystem::path parentPath(const std::filesystem::path& path);
    static std::filesystem::path parentPath(std::filesystem::path&& path);
    static std::filesystem::path withExt(const std::filesystem::path& path, PathView ext);
    static geode::Result<> renameFile(const std::filesystem::path& from, const std::filesystem::path& to);
    static void iterate(
        const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func
    );
};

template <typename Char>
struct fmt::formatter<std::filesystem::path, Char> {
    constexpr const Char* parse(parse_context<Char>& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx) const {
        auto& str = p.native();
        #ifdef GEODE_IS_WINDOWS
        if constexpr (std::is_same_v<Char, wchar_t>) {
            return fmt::format_to(ctx.out(), L"{}", str);
        }
        else {
            return fmt::format_to(ctx.out(), "{}", Filesystem::strNarrow(str));
        }
        #else
        return fmt::format_to(ctx.out(), "{}", str);
        #endif
    }
};
