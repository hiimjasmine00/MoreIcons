#include "Filesystem.hpp"
#include <asp/fs/fs.hpp>
#include <filesystem>

using namespace geode;

std::string& Filesystem::getPathString(asp::fs::Path& path) {
    return *reinterpret_cast<std::string*>(&path);
}

Result<> Filesystem::giveAllPermissions(asp::fs::PathView path) {
    std::error_code code;
    std::filesystem::permissions(std::filesystem::path(path.native()), std::filesystem::perms::all, code);
    if (code) return Err(code.message());
    else return Ok();
}

Result<> Filesystem::renameFile(asp::fs::PathView from, asp::fs::PathView to) {
    if (from.exists()) {
        if (to.exists()) {
            if (auto res = asp::fs::remove(to); res.isErr()) {
                return Err(fmt::format("Failed to remove {}: {}", to.filename().value_or(""), res.unwrapErr()));
            }
        }
        if (auto res = asp::fs::rename(from, to); res.isErr()) {
            return Err(fmt::format("Failed to rename {}: {}", from.filename().value_or(""), res.unwrapErr()));
        }
    }
    return Ok();
}

void Filesystem::iterate(asp::fs::PathView path, asp::fs::FileType type, FunctionRef<void(const asp::fs::Path&)> func) {
    auto iterRes = asp::fs::DirectoryIterator::create(path);
    if (iterRes.isErr()) return;
    for (auto itemRes : std::move(iterRes).unwrap()) {
        if (itemRes.isErr()) continue;

        auto item = std::move(itemRes).unwrap();
        if (item.type != type) continue;
        func(item.path);
    }
}
