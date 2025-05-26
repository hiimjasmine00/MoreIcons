#include "MoreIcons.hpp"
#include "api/MoreIconsAPI.hpp"
#include "classes/misc/ThreadPool.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/ranges.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>

using namespace geode::prelude;

std::string toString(const std::filesystem::path& path) {
    return GEODE_WINDOWS(string::wideToUtf8)(path);
}

bool doesExist(const std::filesystem::path& path) {
    std::error_code code;
    auto exists = std::filesystem::exists(path, code);
    if (code) log::error("{}: Failed to check existence: {}", path, code.message());
    return exists;
}

$on_mod(Loaded) {
    auto mod = Mod::get();
    MoreIcons::debugLogs = mod->getSettingValue<bool>("debug-logs");
    MoreIcons::traditionalPacks = mod->getSettingValue<bool>("traditional-packs");
    MoreIconsAPI::preloadIcons = mod->getSettingValue<bool>("preload-icons");
    #ifdef GEODE_IS_ANDROID
    auto assetsDir = mod->getTempDir() / "assets";
    if (doesExist(assetsDir)) {
        std::error_code code;
        std::filesystem::remove_all(assetsDir, code);
        if (code) log::error("{}: Failed to delete: {}", assetsDir, code.message());
    }
    #endif
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

std::vector<IconPack> MoreIcons::getTexturePacks() {
    std::vector<IconPack> packs = { { "More Icons", "", dirs::getGeodeDir(), false } };
    for (auto& pack : texture_loader::getAppliedPacks()) {
        if (traditionalPacks) {
            if (doesExist(pack.resourcesPath / "icons")) packs.push_back({ pack.name, pack.id, pack.resourcesPath, true });
            else {
                auto trailCount = GameManager::get()->countForType(IconType::Special);
                for (int i = 1; i <= trailCount; i++) {
                    if (doesExist(pack.resourcesPath / fmt::format("streak_{:02}_001.png", i))) {
                        packs.push_back({ pack.name, pack.id, pack.resourcesPath, true });
                        break;
                    }
                }
            }
        }
        if (doesExist(pack.resourcesPath / "config" / GEODE_MOD_ID)) packs.push_back({ pack.name, pack.id, pack.resourcesPath, false });
    }

    return packs;
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_MOBILE
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsMobile = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsMobile->getConfigDir(false) / GEODE_STR(GEODE_GD_VERSION);
            if (doesExist(configDir)) return configDir / path;
        }
        return path;
    }
    #endif
    return toString(dirs::getResourcesDir() / path);
}

bool naturalSorter(const std::string& aStr, const std::string& bStr) {
    auto a = aStr.substr(aStr.find(':') + 1);
    auto b = bStr.substr(bStr.find(':') + 1);
    auto aIt = a.begin();
    auto bIt = b.begin();

    while (aIt < a.end() && bIt < b.end()) {
        if (std::isdigit(*aIt) && std::isdigit(*bIt)) {
            std::string aNum, bNum;
            while (std::isdigit(*aIt)) aNum += *aIt++;
            while (std::isdigit(*bIt)) bNum += *bIt++;
            if (aNum != bNum) {
                if (aNum.size() != bNum.size()) return aNum.size() < bNum.size();
                for (int i = 0; i < aNum.size(); i++) {
                    if (aNum[i] != bNum[i]) return aNum[i] < bNum[i];
                }
            }
        }
        else {
            auto aLower = std::tolower(*aIt);
            auto bLower = std::tolower(*bIt);
            if (aLower != bLower) return aLower < bLower;
            aIt++;
            bIt++;
        }
    }

    return a.size() < b.size();
}

struct NaturalSorter {
    bool operator()(const std::string& a, const std::string& b) const {
        return naturalSorter(a, b);
    }
};

void naturalSort(IconType type) {
    using PackMap = std::map<std::string, std::vector<IconInfo>, NaturalSorter>;
    auto packs = ranges::reduce<PackMap>(MoreIconsAPI::icons, [type](PackMap& acc, const IconInfo& info) {
        if (info.type != type) return acc;
        auto index = info.name.find(':');
        acc[index != std::string::npos ? info.name.substr(0, index) : ""].push_back(info);
        return acc;
    });

    ranges::remove(MoreIconsAPI::icons, [type](const IconInfo& icon) { return icon.type == type; });
    ranges::push(MoreIconsAPI::icons, ranges::reduce<std::vector<IconInfo>>(packs, [](std::vector<IconInfo>& acc, const PackMap::value_type& pack) {
        auto vec = pack.second;
        std::ranges::sort(vec, [](const IconInfo& a, const IconInfo& b) { return naturalSorter(a.name, b.name); });
        return ranges::push(acc, vec);
    }));
}

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return str.substr(0, str.size() - end).append(replace);
}

template <typename... Args>
void printLog(const std::string& name, int severity, fmt::format_string<Args...> message, Args&&... args) {
    auto logMessage = fmt::format(message, std::forward<Args>(args)...);
    log::logImpl(Severity::cast(severity), Mod::get(), "{}: {}", name, logMessage);
    MoreIcons::logs.push_back({ name, logMessage, MoreIcons::currentType, severity });
    auto& currentSeverity = MoreIcons::severities[MoreIcons::currentType];
    if (currentSeverity < severity) currentSeverity = severity;
    if (MoreIcons::severity < severity) MoreIcons::severity = severity;
}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (MoreIcons::debugLogs) log::debug(message, std::forward<Args>(args)...);
}

void loadFolderIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = toString(path.filename());
    auto pathString = toString(path);
    std::string name;
    if (pathFilename.ends_with("-uhd")) {
        name = replaceEnd(pathFilename, 4, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (factor < 4.0f && factor >= 2.0f) {
            if (!doesExist(replaceEnd(pathString, 4, "-hd")) && !doesExist(replaceEnd(pathString, 4, "")))
                return printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            return;
        }
        else if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 4, "")))
                return printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            return;
        }
    }
    else if (pathFilename.ends_with("-hd")) {
        name = replaceEnd(pathFilename, 3, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 3, "")))
                return printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            return;
        }

        if (doesExist(replaceEnd(pathString, 3, "-uhd")) && factor >= 4.0f) return;
    }
    else {
        name = pathFilename;
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (doesExist(pathString + "-uhd") && factor >= 4.0f) return;
        else if (doesExist(pathString + "-hd") && factor >= 2.0f) return;
    }

    safeDebug("Pre-loading folder icon {} from {}", name, pack.name);

    std::error_code code;
    std::filesystem::directory_iterator it(path, code);
    if (code) return printLog(name, Severity::Error, "Failed to create directory iterator: {}", code.message());

    std::vector<std::string> textures;
    std::vector<std::string> frameNames;

    for (; it != std::filesystem::end(it); it.increment(code)) {
        auto& entry = *it;
        if (!entry.is_regular_file()) continue;

        auto& entryPath = entry.path();
        if (entryPath.extension() != ".png") continue;

        textures.push_back(toString(entryPath));
        frameNames.push_back(MoreIconsAPI::getFrameName(toString(entryPath.filename()), name, type));
    }
    if (code) printLog(name, Severity::Warning, "Failed to iterate over directory: {}", code.message());

    if (!textures.empty()) {
        if (MoreIconsAPI::hasIcon(name, type)) return printLog(name, Severity::Warning, "Duplicate icon name");

        MoreIconsAPI::icons.push_back({
            .name = name,
            .textures = textures,
            .frameNames = frameNames,
            .sheetName = "",
            .packName = pack.name,
            .packID = pack.id,
            .type = type,
            .trailID = 0,
            .blend = false,
            .tint = false,
            .show = false,
            .fade = 0.0f,
            .stroke = 0.0f,
            .folderName = toString(path)
        });
    }

    safeDebug("Finished pre-loading folder icon {} from {}", name, pack.name);
}

void loadFileIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = toString(path.filename());
    auto pathStem = toString(path.stem());
    auto pathString = toString(path);
    std::string name;
    if (pathFilename.ends_with("-uhd.plist")) {
        name = replaceEnd(pathStem, 4, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (factor < 4.0f && factor >= 2.0f) {
            if (!doesExist(replaceEnd(pathString, 10, "-hd.plist")) && !doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on medium texture quality");
            return;
        }
        else if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 10, ".plist")))
                printLog(name, Severity::Warning, "Ignoring high-quality icon on low texture quality");
            return;
        }
    }
    else if (pathFilename.ends_with("-hd.plist")) {
        name = replaceEnd(pathStem, 3, "");
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (factor < 2.0f) {
            if (!doesExist(replaceEnd(pathString, 9, ".plist")))
                printLog(name, Severity::Warning, "Ignoring medium-quality icon on low texture quality");
            return;
        }

        if (doesExist(replaceEnd(pathString, 9, "-uhd.plist")) && factor >= 4.0f) return;
    }
    else {
        name = pathStem;
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);

        if (doesExist(replaceEnd(pathString, 6, "-uhd.plist")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 6, "-hd.plist")) && factor >= 2.0f) return;
    }

    safeDebug("Pre-loading file icon {} from {}", name, pack.name);

    auto texturePath = replaceEnd(pathString, 6, ".png");
    if (!doesExist(texturePath)) return printLog(name, Severity::Error, "Texture file {}.png not found", pathStem);

    if (MoreIconsAPI::hasIcon(name, type)) return printLog(name, Severity::Warning, "Duplicate icon name");

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { texturePath },
        .frameNames = {},
        .sheetName = pathString,
        .packName = pack.name,
        .packID = pack.id,
        .type = type,
        .trailID = 0,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .folderName = ""
    });

    safeDebug("Finished pre-loading file icon {} from {}", name, pack.name);
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto factor = CCDirector::get()->getContentScaleFactor();
    auto pathFilename = toString(path.filename());
    auto pathStem = toString(path.stem());
    auto pathString = toString(path);
    std::string name;
    if (pathFilename.ends_with("-uhd.png")) {
        if (factor < 4.0f) return;
        name = fmt::format("{}:{}", pack.id, replaceEnd(pathStem, 4, ""));
    }
    else if (pathFilename.ends_with("-hd.png")) {
        if (factor < 2.0f) return;
        if (doesExist(replaceEnd(pathString, 7, "-uhd.png")) && factor >= 4.0f) return;
        name = fmt::format("{}:{}", pack.id, replaceEnd(pathStem, 3, ""));
    }
    else {
        if (doesExist(replaceEnd(pathString, 4, "-uhd.png")) && factor >= 4.0f) return;
        else if (doesExist(replaceEnd(pathString, 4, "-hd.png")) && factor >= 2.0f) return;
        name = fmt::format("{}:{}", pack.id, pathStem);
    }

    safeDebug("Pre-loading vanilla icon {} from {}", name, pack.name);

    auto plistPath = replaceEnd(pathString, 4, ".plist");
    if (!doesExist(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", pathStem), false);
    if (!CCFileUtils::get()->isFileExist(plistPath)) return printLog(name, Severity::Error, "Plist file not found (Last attempt: {})", plistPath);

    ranges::remove(MoreIconsAPI::icons, [name, type](const IconInfo& icon) { return icon.name == name && icon.type == type; });

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { pathString },
        .frameNames = {},
        .sheetName = plistPath,
        .packName = pack.name,
        .packID = pack.id,
        .type = type,
        .trailID = 0,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .folderName = ""
    });

    safeDebug("Finished pre-loading vanilla icon {} from {}", name, pack.name);
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = toString(path.stem());
    auto name = pack.id.empty() ? pathStem : fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading trail {} from {}", name, pack.name);

    if (MoreIconsAPI::hasIcon(name, IconType::Special)) return printLog(name, Severity::Warning, "Duplicate trail name");

    auto json = file::readJson(std::filesystem::path(path).replace_extension(".json")).unwrapOr(matjson::makeObject({
        { "blend", false },
        { "tint", false },
        { "show", false },
        { "fade", 0.3f },
        { "stroke", 14.0f }
    }));

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { toString(path) },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = 0,
        .blend = json.contains("blend") ? json["blend"].asBool().unwrapOr(false) : false,
        .tint = json.contains("tint") ? json["tint"].asBool().unwrapOr(false) : false,
        .show = json.contains("show") ? json["show"].asBool().unwrapOr(false) : false,
        .fade = json.contains("fade") ? (float)json["fade"].asDouble().unwrapOr(0.3) : 0.3f,
        .stroke = json.contains("stroke") ? (float)json["stroke"].asDouble().unwrapOr(14.0) : 14.0f,
        .folderName = ""
    });

    safeDebug("Finished pre-loading trail {} from {}", name, pack.name);
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    auto pathStem = toString(path.stem());
    auto name = fmt::format("{}:{}", pack.id, pathStem);

    safeDebug("Pre-loading vanilla trail {} from {}", name, pack.name);
    auto trailID = numFromString<int>(pathStem.substr(pathStem.find('_') + 1, pathStem.rfind('_') - pathStem.find('_') - 1)).unwrapOr(0);
    if (trailID <= 0) return;

    ranges::remove(MoreIconsAPI::icons, [name](const IconInfo& icon) { return icon.name == name && icon.type == IconType::Special; });

    MoreIconsAPI::icons.push_back({
        .name = name,
        .textures = { toString(path) },
        .frameNames = {},
        .sheetName = "",
        .packName = pack.name,
        .packID = pack.id,
        .type = IconType::Special,
        .trailID = trailID,
        .blend = false,
        .tint = false,
        .show = false,
        .fade = 0.0f,
        .stroke = 0.0f,
        .folderName = ""
    });

    safeDebug("Finished pre-loading vanilla trail {} from {}", name, pack.name);
}

void createDirectories(const std::filesystem::path& path) {
    std::error_code code;
    std::filesystem::create_directories(path, code);
    if (code) log::error("{}: Failed to create directory: {}", path, code.message());
}

#define DIRECTORY_ITERATOR \
    std::error_code code; \
    std::filesystem::directory_iterator it(path, code); \
    if (code) { \
        log::error("{}: Failed to create directory iterator: {}", path, code.message()); \
        continue; \
    } \
    for (; it != std::filesystem::end(it); it.increment(code))

#define DIRECTORY_ITERATOR_END if (code) log::error("{}: Failed to iterate over directory: {}", path, code.message());

void MoreIcons::loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type) {
    auto begin = MoreIconsAPI::icons.size();
    MoreIconsAPI::iconIndices[type].first = begin;
    currentType = type;

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / suffix;
            if (!doesExist(path)) {
                if (i == 0) createDirectories(path);
                continue;
            }

            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            log::info("Pre-loading {}s from {}", suffix, path);

            DIRECTORY_ITERATOR {
                auto& entry = *it;
                auto& entryPath = entry.path();
                if (entry.is_regular_file()) {
                    if (entryPath.extension() == ".plist") loadFileIcon(entryPath, pack, type);
                }
                else if (entry.is_directory()) loadFolderIcon(entryPath, pack, type);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading {}s from {}", suffix, path);
        }
        else if (traditionalPacks) {
            auto path = pack.path / "icons";
            if (!doesExist(path)) continue;

            log::info("Pre-loading {}s from {}", suffix, path);

            DIRECTORY_ITERATOR {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                auto filename = toString(entryPath.filename());
                if (!filename.starts_with(prefixes[(int)type]) || (type == IconType::Cube && filename.starts_with("player_ball_"))) continue;

                loadVanillaIcon(entryPath, pack, type);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading {}s from {}", suffix, path);
        }
    }

    naturalSort(type);

    MoreIconsAPI::iconIndices[type].second = MoreIconsAPI::icons.size();

    if (MoreIconsAPI::preloadIcons) {
        auto size = MoreIconsAPI::icons.size() - begin;

        log::info("Pre-loading {} {} textures", size, suffix);

        for (auto it = MoreIconsAPI::icons.begin() + begin; it != MoreIconsAPI::icons.end(); it++) {
            MoreIconsAPI::loadIconAsync(*it);
        }

        ThreadPool::get().wait();
        MoreIconsAPI::finishLoadIcons();

        log::info("Finished pre-loading {} {} textures", size, suffix);
    }
}

void MoreIcons::loadTrails(const std::vector<IconPack>& packs) {
    auto begin = MoreIconsAPI::icons.size();
    MoreIconsAPI::iconIndices[IconType::Special].first = begin;
    currentType = IconType::Special;

    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / "trail";
            if (!doesExist(path)) {
                if (i == 0) createDirectories(path);
                continue;
            }

            if (i == 0) {
                std::error_code code;
                std::filesystem::permissions(path, std::filesystem::perms::all, code);
                if (code) log::error("{}: Failed to change permissions: {}", path, code.message());
            }

            log::info("Pre-loading trails from {}", path);

            DIRECTORY_ITERATOR {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                loadTrail(entryPath, pack);
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading trails from {}", path);
        }
        else if (traditionalPacks) {
            auto& path = pack.path;

            log::info("Pre-loading trails from {}", path);

            auto trailCount = GameManager::get()->countForType(IconType::Special);
            DIRECTORY_ITERATOR {
                auto& entry = *it;
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                auto filename = toString(entryPath.filename());
                for (int i = 1; i <= trailCount; i++) {
                    if (filename == fmt::format("streak_{:02}_001.png", i)) {
                        loadVanillaTrail(entryPath, pack);
                        break;
                    }
                }
            }
            DIRECTORY_ITERATOR_END

            log::info("Finished pre-loading trails from {}", path);
        }
    }

    naturalSort(IconType::Special);

    MoreIconsAPI::iconIndices[IconType::Special].second = MoreIconsAPI::icons.size();

    if (MoreIconsAPI::preloadIcons) {
        auto size = MoreIconsAPI::icons.size() - begin;

        log::info("Pre-loading {} trail textures", size);

        for (auto it = MoreIconsAPI::icons.begin() + begin; it != MoreIconsAPI::icons.end(); it++) {
            MoreIconsAPI::loadIconAsync(*it);
        }

        ThreadPool::get().wait();
        MoreIconsAPI::finishLoadIcons();

        log::info("Finished pre-loading {} trail textures", size);
    }
}

void MoreIcons::saveTrails() {
    for (auto& info : MoreIconsAPI::icons) {
        if (info.type != IconType::Special || info.trailID > 0) continue;
        (void)file::writeToJson(std::filesystem::path(info.textures[0]).replace_extension(".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
            { "show", info.show },
            { "fade", info.fade },
            { "stroke", info.stroke }
        })).inspectErr([info](const std::string& err) { log::error("{}: Failed to save trail info: {}", info.name, err); });
    }
}
