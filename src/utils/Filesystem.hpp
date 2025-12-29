#include <filesystem>
#include <fmt/format.h>
#include <Geode/platform/cplatform.h>
#ifdef GEODE_IS_WINDOWS
#include <fmt/xchar.h>
#endif
#include <Geode/Result.hpp>
#include <std23/function_ref.h>

#ifdef GEODE_IS_WINDOWS
#define L(x) std::wstring_view(L##x, sizeof(x) - 1)
#define MI_FILESYSTEM_BEGIN namespace std {
#define MI_FILESYSTEM_END }
#else
#define L(x) std::string_view(x, sizeof(x) - 1)
#define MI_FILESYSTEM_BEGIN namespace std { inline namespace _LIBCPP_ABI_NAMESPACE { inline namespace __fs {
#define MI_FILESYSTEM_END } } }
#endif

MI_FILESYSTEM_BEGIN
namespace filesystem {
    path operator/(const path& lhs, basic_string_view<path::value_type> rhs);

    inline path operator/(const path& lhs, const path::string_type& rhs) {
        return lhs / basic_string_view<path::value_type>(rhs.data(), rhs.size());
    }

    path operator/(path&& lhs, basic_string_view<path::value_type> rhs);

    inline path operator/(path&& lhs, const path::string_type& rhs) {
        return std::move(lhs) / basic_string_view<path::value_type>(rhs.data(), rhs.size());
    }
}
MI_FILESYSTEM_END

namespace Filesystem {
    using PathView = std::basic_string_view<std::filesystem::path::value_type>;

    bool doesExist(const std::filesystem::path& path);
    #ifdef GEODE_IS_WINDOWS
    std::string strNarrow(std::wstring_view str);
    std::wstring strWide(std::string_view str);
    std::string filenameFormat(const std::filesystem::path& path);
    #else
    inline std::string_view strNarrow(std::string_view str) {
        return str;
    }
    inline std::string_view strWide(std::string_view str) {
        return str;
    }
    std::string_view filenameFormat(const std::filesystem::path& path);
    #endif
    std::filesystem::path strPath(std::string_view path);
    #ifndef GEODE_IS_WINDOWS
    std::filesystem::path strPath(std::string&& path);
    #endif
    PathView filenameView(const std::filesystem::path& path);
    std::filesystem::path parentPath(const std::filesystem::path& path);
    std::filesystem::path parentPath(std::filesystem::path&& path);
    std::filesystem::path withExt(const std::filesystem::path& path, PathView ext);
    geode::Result<> renameFile(const std::filesystem::path& from, const std::filesystem::path& to);
    void iterate(const std::filesystem::path& path, std::filesystem::file_type type, std23::function_ref<void(const std::filesystem::path&)> func);
}

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
