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
    path operator/(path&& lhs, const path& rhs);
}

class Filesystem {
public:
    static bool doesExist(const std::filesystem::path& path);
    static std::string strNarrow(std::basic_string_view<std::filesystem::path::value_type> str);
    static std::filesystem::path::string_type strWide(std::string_view str);
    static std::filesystem::path strPath(std::string_view path);
    static std::basic_string_view<std::filesystem::path::value_type> filenameView(const std::filesystem::path& path, size_t removeCount = 0);
    #ifdef GEODE_IS_WINDOWS
    static std::string filenameFormat(const std::filesystem::path& path);
    #else
    static std::string_view filenameFormat(const std::filesystem::path& path);
    #endif
    static std::filesystem::path filenamePath(const std::filesystem::path& path);
    static std::filesystem::path filenamePath(std::filesystem::path&& path);
    static std::filesystem::path parentPath(const std::filesystem::path& path);
    static std::filesystem::path parentPath(std::filesystem::path&& path);
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
        #ifdef GEODE_IS_WINDOWS
        if constexpr (std::is_same_v<Char, wchar_t>) {
            return fmt::format_to(ctx.out(), L"{}", p.native());
        }
        else {
            return fmt::format_to(ctx.out(), "{}", Filesystem::strNarrow(p.native()));
        }
        #else
        return fmt::format_to(ctx.out(), "{}", p.native());
        #endif
    }
};
