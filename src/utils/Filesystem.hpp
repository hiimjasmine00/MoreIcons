#include <asp/fs/Path.hpp>
#include <Geode/utils/function.hpp>

namespace Filesystem {
    std::string& getPathString(asp::fs::Path& path);
    geode::Result<> giveAllPermissions(asp::fs::PathView path);
    geode::Result<> renameFile(asp::fs::PathView from, asp::fs::PathView to);
    void iterate(asp::fs::PathView path, asp::fs::FileType type, geode::FunctionRef<void(const asp::fs::Path&)> func);
}
