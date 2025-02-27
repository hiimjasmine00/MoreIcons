#include "MoreIcons.hpp"
#include <BS_thread_pool.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;
using namespace geode::texture_loader;

$execute {
    new EventListener(+[](SimplePlayer* player, const std::string& icon, IconType type) {
        MoreIconsAPI::updateSimplePlayer(player, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<SimplePlayer*, std::string, IconType>("simple-player"_spr));

    new EventListener(+[](GJRobotSprite* sprite, const std::string& icon) {
        MoreIconsAPI::updateRobotSprite(sprite, icon);
        return ListenerResult::Propagate;
    }, DispatchFilter<GJRobotSprite*, std::string>("robot-sprite"_spr));

    new EventListener(+[](GJRobotSprite* sprite, const std::string& icon, IconType type) {
        MoreIconsAPI::updateRobotSprite(sprite, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<GJRobotSprite*, std::string, IconType>("robot-sprite"_spr));

    new EventListener(+[](PlayerObject* object, const std::string& icon) {
        MoreIconsAPI::updatePlayerObject(object, icon);
        return ListenerResult::Propagate;
    }, DispatchFilter<PlayerObject*, std::string>("player-object"_spr));

    new EventListener(+[](PlayerObject* object, const std::string& icon, IconType type) {
        MoreIconsAPI::updatePlayerObject(object, icon, type);
        return ListenerResult::Propagate;
    }, DispatchFilter<PlayerObject*, std::string, IconType>("player-object"_spr));

    new EventListener(+[](std::vector<std::string>* vec, IconType type) {
        vec->clear();
        *vec = MoreIconsAPI::vectorForType(type);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::vector<std::string>*, IconType>("all-icons"_spr));

    new EventListener(+[](std::string* icon, IconType type, bool dual) {
        *icon = MoreIconsAPI::activeForType(type, dual);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string*, IconType, bool>("active-icon"_spr));

    new EventListener(+[](const std::string& icon, IconType type, bool dual) {
        MoreIconsAPI::setIcon(icon, type, dual);
        return ListenerResult::Propagate;
    }, DispatchFilter<std::string, IconType, bool>("set-icon"_spr));
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

void MoreIcons::naturalSort(std::vector<std::string>& vec) {
    std::map<std::string, std::vector<std::string>> packs;
    for (auto& str : vec) {
        auto index = str.find_first_of(':');
        packs[str.substr(0, index != std::string::npos ? index : 0)].push_back(str);
    }

    for (auto& [_, pack] : packs) {
        std::sort(pack.begin(), pack.end(), naturalSorter);
    }

    std::vector<std::string> packIDs;
    for (auto& [packID, _] : packs) {
        packIDs.push_back(packID);
    }
    std::sort(packIDs.begin(), packIDs.end(), naturalSorter);

    vec.clear();
    for (auto& packID : packIDs) {
        auto& pack = packs[packID];
        vec.insert(vec.end(), pack.begin(), pack.end());
    }
}

bool MoreIcons::naturalSorter(const std::string& aStr, const std::string& bStr) {
    auto a = aStr.substr(aStr.find_first_of(':') + 1);
    auto b = bStr.substr(bStr.find_first_of(':') + 1);
    auto aIt = a.begin();
    auto bIt = b.begin();

    while (aIt != a.end() && bIt != b.end()) {
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

std::vector<IconPack> MoreIcons::getTexturePacks() {
    std::vector<IconPack> packs;

    packs.push_back({
        .name = "More Icons",
        .id = "",
        .path = dirs::getGeodeDir(),
        .vanilla = false
    });
    for (auto& pack : getAppliedPacks()) {
        if (std::filesystem::exists(pack.resourcesPath / "icons")) packs.push_back({
            .name = pack.name,
            .id = pack.id,
            .path = pack.resourcesPath,
            .vanilla = true
        });
        else {
            auto trailCount = GameManager::get()->countForType(IconType::Special);
            for (int i = 1; i <= trailCount; i++) {
                if (std::filesystem::exists(pack.resourcesPath / fmt::format("streak_{:02}_001.png", i))) {
                    packs.push_back({
                        .name = pack.name,
                        .id = pack.id,
                        .path = pack.resourcesPath,
                        .vanilla = true
                    });
                    break;
                }
            }
        }
        if (std::filesystem::exists(pack.resourcesPath / "config" / GEODE_MOD_ID)) packs.push_back({
            .name = pack.name,
            .id = pack.id,
            .path = pack.resourcesPath,
            .vanilla = false
        });
    }

    return packs;
}

void MoreIcons::unzipVanillaAssets() {
    #ifdef GEODE_IS_ANDROID
    auto tempDir = Mod::get()->getTempDir();
    if (std::filesystem::exists(tempDir / "assets")) {
        std::error_code cleanCode;
        std::filesystem::remove_all(tempDir / "assets", cleanCode);
        if (cleanCode) return log::error("Failed to clean assets folder: {}", cleanCode.message());
    }
    if (std::filesystem::exists(tempDir / "vanilla")) {
        std::error_code cleanCode;
        std::filesystem::remove_all(tempDir / "vanilla", cleanCode);
        if (cleanCode) return log::error("Failed to clean vanilla folder: {}", cleanCode.message());
    }
    auto unzipRes = file::Unzip::intoDir(*reinterpret_cast<const char**>(dlsym(dlopen("libcocos2dcpp.so", RTLD_NOW), "g_apkPath")), tempDir / "vanilla");
    if (unzipRes.isErr()) return log::error("Failed to unzip vanilla assets: {}", unzipRes.unwrapErr());
    std::error_code renameCode;
    std::filesystem::rename(tempDir / "vanilla" / "assets", tempDir / "assets", renameCode);
    if (renameCode) return log::error("Failed to move assets folder: {}", renameCode.message());
    std::error_code removeCode;
    std::filesystem::remove_all(tempDir / "vanilla", removeCode);
    if (removeCode) return log::error("Failed to remove vanilla folder: {}", removeCode.message());
    #endif
}

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    std::string ret = path;
    auto textureQuality = CCDirector::sharedDirector()->getLoadedTextureQuality();
    #ifdef GEODE_IS_ANDROID
    if (textureQuality == kTextureQualityHigh && !skipSuffix) {
        if (auto highGraphicsAndroid = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsAndroid->getConfigDir(false) / Loader::get()->getGameVersion();
            if (std::filesystem::exists(configDir)) ret = fmt::format("{}/{}", configDir.string(), path);
        }
    }
    else {
        auto assetsDir = Mod::get()->getTempDir() / "assets";
        if (std::filesystem::exists(assetsDir)) ret = fmt::format("{}/{}", assetsDir.string(), path);
    }
    #else
    ret = fmt::format("{}/Resources/{}", dirs::getGameDir().string(), path);
    #endif
    return ret;
}

// https://github.com/GlobedGD/globed2/blob/v1.6.2/src/util/cocos.cpp#L44
namespace {
    template <typename TC>
    using priv_method_t = void(TC::*)(CCDictionary*, CCTexture2D*);

    template <typename TC, priv_method_t<TC> func>
    struct priv_caller {
        friend void _addSpriteFramesWithDictionary(CCDictionary* dict, CCTexture2D* texture) {
            (CCSpriteFrameCache::sharedSpriteFrameCache()->*func)(dict, texture);
        }
    };

    template struct priv_caller<CCSpriteFrameCache, &CCSpriteFrameCache::addSpriteFramesWithDictionary>;

    void _addSpriteFramesWithDictionary(CCDictionary*, CCTexture2D*);
}

std::string getFrameName(const std::string& name, const std::string& prefix, IconType type) {
    if (type != IconType::Robot && type != IconType::Spider) {
        if (name.ends_with("_2_001.png")) return fmt::format("{}_2_001.png"_spr, prefix);
        else if (name.ends_with("_3_001.png")) return fmt::format("{}_3_001.png"_spr, prefix);
        else if (name.ends_with("_extra_001.png")) return fmt::format("{}_extra_001.png"_spr, prefix);
        else if (name.ends_with("_glow_001.png")) return fmt::format("{}_glow_001.png"_spr, prefix);
        else if (name.ends_with("_001.png")) return fmt::format("{}_001.png"_spr, prefix);
    }
    else for (int i = 1; i < 5; i++) {
        if (name.ends_with(fmt::format("_{:02}_2_001.png", i))) return fmt::format("{}_{:02}_2_001.png"_spr, prefix, i);
        else if (i == 1 && name.ends_with(fmt::format("_{:02}_extra_001.png", i))) return fmt::format("{}_{:02}_extra_001.png"_spr, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_glow_001.png", i))) return fmt::format("{}_{:02}_glow_001.png"_spr, prefix, i);
        else if (name.ends_with(fmt::format("_{:02}_001.png", i))) return fmt::format("{}_{:02}_001.png"_spr, prefix, i);
    }

    return name;
}

std::string replaceEnd(const std::string& str, std::string_view end, std::string_view replace) {
    return fmt::format("{}{}", str.substr(0, str.size() - end.size()), replace);
}

BS::thread_pool<BS::tp::none>& sharedPool() {
    static BS::thread_pool _sharedPool(std::thread::hardware_concurrency());
    return _sharedPool;
}

void MoreIcons::loadIcons(const std::vector<IconPack>& packs, std::string_view suffix, IconType type) {
    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / suffix;
            if (!std::filesystem::exists(path)) {
                if (i == 0) std::filesystem::create_directories(path);
                continue;
            }
            log::info("Loading {}s from {}", suffix, path.string());

            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    auto& entryPath = entry.path();
                    if (entryPath.extension() != ".plist") continue;

                    loadIcon(entryPath, pack, type);
                }
                else if (entry.is_directory()) loadIcon(entry.path(), pack, type);
            }
        }
        else {
            auto path = pack.path / "icons";
            if (!std::filesystem::exists(path)) continue;
            log::info("Loading {}s from {}", suffix, path.string());

            auto prefix = "";
            switch (type) {
                case IconType::Cube: prefix = "player_"; break;
                case IconType::Ship: prefix = "ship_"; break;
                case IconType::Ball: prefix = "player_ball_"; break;
                case IconType::Ufo: prefix = "bird_"; break;
                case IconType::Wave: prefix = "dart_"; break;
                case IconType::Robot: prefix = "robot_"; break;
                case IconType::Spider: prefix = "spider_"; break;
                case IconType::Swing: prefix = "swing_"; break;
                case IconType::Jetpack: prefix = "jetpack_"; break;
                default: break;
            }
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png" || !entryPath.filename().string().starts_with(prefix) ||
                    (type == IconType::Cube && string::contains(entryPath.filename().string(), "player_ball_"))) continue; // Nice one RobTop

                loadVanillaIcon(entryPath, pack, type);
            }
        }
    }

    sharedPool().wait();

    {
        std::lock_guard lock(IMAGE_MUTEX);
        auto textureCache = CCTextureCache::sharedTextureCache();
        auto spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
        auto& vec = MoreIconsAPI::vectorForType(type);
        for (auto& image : IMAGES) {
            auto texture = new CCTexture2D();
            if (texture->initWithImage(image.image)) {
                textureCache->m_pTextures->setObject(texture, image.texturePath);
                if (!image.dict && !image.frameName.empty())
                    spriteFrameCache->addSpriteFrame(
                        CCSpriteFrame::createWithTexture(texture, { { 0, 0 }, texture->getContentSize() }),
                        image.frameName.c_str()
                    );
                else if (image.dict) {
                    _addSpriteFramesWithDictionary(image.dict, texture);
                    CC_SAFE_RELEASE(image.dict);
                }
                if (image.index == 0) {
                    vec.push_back(image.name);
                    infoForType(type)[image.name] = image.pack;
                }
            }

            texture->release();
            CC_SAFE_RELEASE(image.image);
        }

        log::info("Loaded {} {}{}", vec.size(), suffix, vec.size() == 1 ? "" : "s");
        IMAGES.clear();
        naturalSort(vec);
    }
}

void MoreIcons::loadIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    if (!std::filesystem::exists(path)) return;

    auto textureQuality = CCDirector::sharedDirector()->getLoadedTextureQuality();
    if (std::filesystem::is_directory(path)) {
        sharedPool().detach_task([path, pack, textureQuality, type] {
            auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());
            auto textureCache = CCTextureCache::sharedTextureCache();
            auto spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
            int i = 0;
            for (auto& subEntry : std::filesystem::directory_iterator(path)) {
                if (!subEntry.is_regular_file()) continue;

                auto subEntryPath = subEntry.path();
                if (subEntryPath.extension() != ".png") continue;

                auto pathFilename = subEntryPath.filename().string();
                auto fileQuality = kTextureQualityLow;
                if (pathFilename.ends_with("-uhd.png")) {
                    if (textureQuality != kTextureQualityHigh) {
                        auto logMessage = fmt::format("{}: Ignoring high-quality PNG file for {} texture quality", subEntryPath.string(),
                            textureQuality == kTextureQualityMedium ? "medium" : "low");
                        log::warn("{}", logMessage);
                        {
                            std::lock_guard lock(LOG_MUTEX);
                            LOGS.push_back({ .message = logMessage, .type = LogType::Info });
                        }
                        continue;
                    }

                    fileQuality = kTextureQualityHigh;
                }
                else if (pathFilename.ends_with("-hd.png")) {
                    if (textureQuality != kTextureQualityHigh && textureQuality != kTextureQualityMedium) {
                        auto logMessage = fmt::format("{}: Ignoring medium-quality PNG file for low texture quality", subEntryPath.string());
                        log::warn("{}", logMessage);
                        {
                            std::lock_guard lock(LOG_MUTEX);
                            LOGS.push_back({ .message = logMessage, .type = LogType::Info });
                        }
                        continue;
                    }

                    if (
                        std::filesystem::exists(replaceEnd(subEntryPath.string(), "-hd.png", "-uhd.png")) &&
                        textureQuality == kTextureQualityHigh
                    ) continue;
                    else fileQuality = kTextureQualityMedium;
                }
                else {
                    if (
                        std::filesystem::exists(replaceEnd(subEntryPath.string(), ".png", "-uhd.png")) &&
                        textureQuality == kTextureQualityHigh
                    ) continue;
                    else if (
                        std::filesystem::exists(replaceEnd(subEntryPath.string(), ".png", "-hd.png")) &&
                        (textureQuality == kTextureQualityMedium || textureQuality == kTextureQualityHigh)
                    ) continue;
                    else fileQuality = kTextureQualityLow;
                }

                auto pngPath = subEntryPath.string();
                std::string noGraphicPngPath;
                if (fileQuality == kTextureQualityHigh) noGraphicPngPath = replaceEnd(pngPath, "-uhd.png", ".png");
                else if (fileQuality == kTextureQualityMedium) noGraphicPngPath = replaceEnd(pngPath, "-hd.png", ".png");
                else noGraphicPngPath = pngPath;
                auto image = new CCImage();
                if (image->initWithImageFileThreadSafe(pngPath.c_str())) {
                    std::lock_guard lock(IMAGE_MUTEX);
                    if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) return;
                    IMAGES.push_back({
                        .image = image,
                        .dict = nullptr,
                        .texturePath = pngPath,
                        .name = name,
                        .frameName = getFrameName(std::filesystem::path(noGraphicPngPath).filename().string(), name, type),
                        .pack = {
                            .name = pack.name,
                            .id = pack.id
                        },
                        .index = i
                    });

                    i++;
                }
                else image->release();
            }
        });
    }
    else if (std::filesystem::is_regular_file(path)) {
        sharedPool().detach_task([path, pack, textureQuality, type] {
            auto pathFilename = path.filename().string();
            auto fileQuality = kTextureQualityLow;
            if (pathFilename.ends_with("-uhd.plist")) {
                if (textureQuality != kTextureQualityHigh) {
                    auto logMessage = fmt::format("{}: Ignoring high-quality PLIST file for {} texture quality", path.string(),
                        textureQuality == kTextureQualityMedium ? "medium" : "low");
                    log::warn("{}", logMessage);
                    {
                        std::lock_guard lock(LOG_MUTEX);
                        LOGS.push_back({ .message = logMessage, .type = LogType::Info });
                    }
                    return;
                }

                fileQuality = kTextureQualityHigh;
            }
            else if (pathFilename.ends_with("-hd.plist")) {
                if (textureQuality != kTextureQualityHigh && textureQuality != kTextureQualityMedium) {
                    auto logMessage = fmt::format("{}: Ignoring medium-quality PLIST file for low texture quality", path.string());
                    log::warn("{}", logMessage);
                    {
                        std::lock_guard lock(LOG_MUTEX);
                        LOGS.push_back({ .message = logMessage, .type = LogType::Info });
                    }
                    return;
                }

                if (
                    std::filesystem::exists(replaceEnd(path.string(), "-hd.plist", "-uhd.plist")) &&
                    textureQuality == kTextureQualityHigh
                ) return;
                else fileQuality = kTextureQualityMedium;
            }
            else {
                if (
                    std::filesystem::exists(replaceEnd(path.string(), ".plist", "-uhd.plist")) &&
                    textureQuality == kTextureQualityHigh
                ) return;
                else if (
                    std::filesystem::exists(replaceEnd(path.string(), ".plist", "-hd.plist")) &&
                    (textureQuality == kTextureQualityMedium || textureQuality == kTextureQualityHigh)
                ) return;
                else fileQuality = kTextureQualityLow;
            }

            auto plistPath = path.string();
            std::string noGraphicPlistPath;
            if (fileQuality == kTextureQualityHigh) noGraphicPlistPath = replaceEnd(plistPath, "-uhd.plist", ".plist");
            else if (fileQuality == kTextureQualityMedium) noGraphicPlistPath = replaceEnd(plistPath, "-hd.plist", ".plist");
            else noGraphicPlistPath = plistPath;
            auto name = pack.id.empty() ? std::filesystem::path(noGraphicPlistPath).stem().string() :
                fmt::format("{}:{}", pack.id, std::filesystem::path(noGraphicPlistPath).stem());

            auto dict = CCDictionary::createWithContentsOfFileThreadSafe(plistPath.c_str());
            auto frames = new CCDictionary();
            for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
                frames->setObject(frame, getFrameName(frameName, name, type));
            }
            dict->setObject(frames, "frames");
            auto metadata = static_cast<CCDictionary*>(dict->objectForKey("metadata"));
            auto texturePath = std::filesystem::path(metadata->valueForKey("textureFileName")->getCString()).filename().string();
            auto fullTexturePath = path.parent_path() / texturePath;
            if (!std::filesystem::exists(fullTexturePath)) {
                auto fallbackTexturePath = std::filesystem::path(path).replace_extension(".png");
                if (!std::filesystem::exists(fallbackTexturePath)) {
                    auto logMessage = fmt::format("{}: Texture file {} not found, no fallback", path.string(), texturePath);
                    log::warn("{}", logMessage);
                    {
                        std::lock_guard lock(LOG_MUTEX);
                        LOGS.push_back({ .message = logMessage, .type = LogType::Error });
                        if (HIGHEST_SEVERITY < LogType::Error) HIGHEST_SEVERITY = LogType::Error;
                    }
                }
                else fullTexturePath = fallbackTexturePath;
                return;
            }

            auto image = new CCImage();
            if (image->initWithImageFileThreadSafe(fullTexturePath.string().c_str())) {
                std::lock_guard lock(IMAGE_MUTEX);
                if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) return;
                IMAGES.push_back({
                    .image = image,
                    .dict = dict,
                    .texturePath = fullTexturePath.string(),
                    .name = name,
                    .frameName = "",
                    .pack = {
                        .name = pack.name,
                        .id = pack.id
                    },
                    .index = 0
                });
            }
            else {
                dict->release();
                image->release();
            }
        });
    }
}

void MoreIcons::loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    auto textureQuality = CCDirector::sharedDirector()->getLoadedTextureQuality();
    sharedPool().detach_task([pack, path, textureQuality, type] {
        auto pathFilename = path.filename().string();
        auto fileQuality = kTextureQualityLow;
        if (pathFilename.ends_with("-uhd.png")) {
            if (textureQuality != kTextureQualityHigh) return;
            fileQuality = kTextureQualityHigh;
        }
        else if (pathFilename.ends_with("-hd.png")) {
            if (
                std::filesystem::exists(replaceEnd(path.string(), "-hd.png", "-uhd.png")) &&
                textureQuality == kTextureQualityHigh
            ) return;
            else fileQuality = kTextureQualityMedium;
        }
        else {
            if (
                std::filesystem::exists(replaceEnd(path.string(), ".png", "-uhd.png")) &&
                textureQuality == kTextureQualityHigh
            ) return;
            else if (
                std::filesystem::exists(replaceEnd(path.string(), ".png", "-hd.png")) &&
                (textureQuality == kTextureQualityMedium || textureQuality == kTextureQualityHigh)
            ) return;
            else fileQuality = kTextureQualityLow;
        }

        auto pngPath = path.string();
        auto plistPath = replaceEnd(pngPath, ".png", ".plist");
        if (!std::filesystem::exists(plistPath)) plistPath = vanillaTexturePath(fmt::format("icons/{}", std::filesystem::path(plistPath).filename()), false);
        if (!std::filesystem::exists(plistPath)) {
            auto logMessage = fmt::format("{}: PLIST file not found (Last attempt: {})", path.string(), plistPath);
            log::error("{}", logMessage);
            {
                std::lock_guard lock(LOG_MUTEX);
                LOGS.push_back({ .message = logMessage, .type = LogType::Error });
                if (HIGHEST_SEVERITY < LogType::Error) HIGHEST_SEVERITY = LogType::Error;
            }
            return;
        }
        std::string noGraphicPngPath;
        if (fileQuality == kTextureQualityHigh) noGraphicPngPath = replaceEnd(pngPath, "-uhd.png", ".png");
        else if (fileQuality == kTextureQualityMedium) noGraphicPngPath = replaceEnd(pngPath, "-hd.png", ".png");
        else noGraphicPngPath = pngPath;
        auto name = fmt::format("{}:{}", pack.id, std::filesystem::path(noGraphicPngPath).stem());

        auto dict = CCDictionary::createWithContentsOfFileThreadSafe(plistPath.c_str());
        auto frames = new CCDictionary();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
            frames->setObject(frame, getFrameName(frameName, name, type));
        }
        dict->setObject(frames, "frames");

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(pngPath.c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty()) ranges::remove(IMAGES, [name](const ImageData& image) { return image.name == name; });
            IMAGES.push_back({
                .image = image,
                .dict = dict,
                .texturePath = pngPath,
                .name = name,
                .frameName = "",
                .pack = {
                    .name = pack.name,
                    .id = pack.id
                },
                .index = 0
            });
        }
        else image->release();
    });
}

void MoreIcons::loadTrails(const std::vector<IconPack>& packs) {
    for (int i = 0; i < packs.size(); i++) {
        auto& pack = packs[i];

        if (!pack.vanilla) {
            auto path = pack.path / "config" / GEODE_MOD_ID / "trail";
            if (!std::filesystem::exists(path)) {
                if (i == 0) std::filesystem::create_directories(path);
                continue;
            }
            log::info("Loading trails from {}", path.string());

            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                loadTrail(entryPath, {
                    .name = pack.name,
                    .id = pack.id
                });
            }
        }
        else {
            auto trailCount = GameManager::get()->countForType(IconType::Special);
            for (auto& entry : std::filesystem::directory_iterator(pack.path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                auto entryFilename = entryPath.filename().string();
                for (int i = 1; i <= trailCount; i++) {
                    if (entryFilename == fmt::format("streak_{:02}_001.png", i)) {
                        loadVanillaTrail(entryPath, {
                            .name = pack.name,
                            .id = pack.id
                        });
                        break;
                    }
                }
            }
        }
    }

    sharedPool().wait();

    {
        std::lock_guard lock(IMAGE_MUTEX);
        auto textureCache = CCTextureCache::get();
        for (auto& image : IMAGES) {
            auto texture = new CCTexture2D();
            if (texture->initWithImage(image.image)) {
                textureCache->m_pTextures->setObject(texture, image.texturePath);
                MoreIconsAPI::TRAILS.push_back(image.name);
                TRAIL_INFO[image.name] = {
                    .texture = image.texturePath,
                    .pack = image.pack,
                    .trailID = image.trailID,
                    .blend = image.blend,
                    .tint = image.tint
                };
            }

            texture->release();
            CC_SAFE_RELEASE(image.image);
        }

        log::info("Loaded {} trail{}", MoreIconsAPI::TRAILS.size(), MoreIconsAPI::TRAILS.size() == 1 ? "" : "s");
        IMAGES.clear();
        naturalSort(MoreIconsAPI::TRAILS);
    }
}

void MoreIcons::loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    sharedPool().detach_task([path, pack] {
        auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());
        auto jsonPath = std::filesystem::path(path).replace_extension(".json");
        matjson::Value json;
        if (!std::filesystem::exists(jsonPath)) json = matjson::makeObject({ { "blend", false }, { "tint", false } });
        else {
            auto readRes = file::readJson(jsonPath);
            if (readRes.isErr()) {
                auto logMessage = fmt::format("{}: Failed to read JSON file ({})", path.string(), readRes.unwrapErr());
                log::warn("{}", logMessage);
                {
                    std::lock_guard lock(LOG_MUTEX);
                    LOGS.push_back({ .message = logMessage, .type = LogType::Warn });
                    if (HIGHEST_SEVERITY < LogType::Warn) HIGHEST_SEVERITY = LogType::Warn;
                }
                json = matjson::makeObject({ { "blend", false }, { "tint", false } });
            }
            else json = readRes.unwrap();
        }

        auto fullTexturePath = path.string();
        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(fullTexturePath.c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) return;
            IMAGES.push_back({
                .image = image,
                .dict = nullptr,
                .texturePath = fullTexturePath,
                .name = name,
                .frameName = "",
                .pack = {
                    .name = pack.name,
                    .id = pack.id
                },
                .index = 0,
                .trailID = 0,
                .blend = json.contains("blend") && json["blend"].isBool() ? json["blend"].asBool().unwrap() : false,
                .tint = json.contains("tint") && json["tint"].isBool() ? json["tint"].asBool().unwrap() : false,
            });
        }
        else image->release();
    });
}

void MoreIcons::loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    sharedPool().detach_task([path, pack] {
        auto pathStem = path.stem().string();
        auto name = fmt::format("{}:{}", pack.id, pathStem);
        auto trailString = pathStem.substr(pathStem.find_first_of('_') + 1, pathStem.find_last_of('_') - pathStem.find_first_of('_') - 1);
        auto trailID = numFromString<int>(trailString).unwrapOr(0);
        if (trailID <= 0) {
            auto logMessage = fmt::format("{}: Invalid trail filename", path.string());
            log::error("{}", logMessage);
            {
                std::lock_guard lock(LOG_MUTEX);
                LOGS.push_back({ .message = logMessage, .type = LogType::Error });
                if (HIGHEST_SEVERITY < LogType::Error) HIGHEST_SEVERITY = LogType::Error;
            }
            return;
        }

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(path.string().c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty()) ranges::remove(IMAGES, [name](const ImageData& image) { return image.name == name; });
            IMAGES.push_back({
                .image = image,
                .dict = nullptr,
                .texturePath = path.string(),
                .name = name,
                .frameName = "",
                .pack = {
                    .name = pack.name,
                    .id = pack.id
                },
                .index = 0,
                .trailID = trailID,
                .blend = false,
                .tint = false,
            });
        }
        else image->release();
    });
}

void MoreIcons::saveTrails() {
    for (auto& [trail, info] : TRAIL_INFO) {
        if (info.trailID > 0) continue;
        auto writeRes = file::writeToJson(std::filesystem::path(info.texture).replace_extension(".json"),
            matjson::makeObject({ { "blend", info.blend }, { "tint", info.tint }, }));
        if (writeRes.isErr()) log::error("Failed to save trail JSON: {}", writeRes.unwrapErr());
    }
}

bool MoreIcons::dualSelected() {
    auto sdi = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    return sdi && sdi->getSavedValue("2pselected", false);
}

void MoreIcons::showInfoPopup(bool folderButton) {
    createQuickPopup(
        "More Icons",
        fmt::format(std::locale(""),
            "<cg>Icons</c>: {:L}\n"
            "<cp>Ships</c>: {:L}\n"
            "<cr>Balls</c>: {:L}\n"
            "<co>UFOs</c>: {:L}\n"
            "<cj>Waves</c>: {:L}\n"
            "Robots: {:L}\n"
            "<ca>Spiders</c>: {:L}\n"
            "<cy>Swings</c>: {:L}\n"
            "<cd>Jetpacks</c>: {:L}\n"
            "<cb>Trails</c>: {:L}",
            MoreIconsAPI::ICONS.size(),
            MoreIconsAPI::SHIPS.size(),
            MoreIconsAPI::BALLS.size(),
            MoreIconsAPI::UFOS.size(),
            MoreIconsAPI::WAVES.size(),
            MoreIconsAPI::ROBOTS.size(),
            MoreIconsAPI::SPIDERS.size(),
            MoreIconsAPI::SWINGS.size(),
            MoreIconsAPI::JETPACKS.size(),
            MoreIconsAPI::TRAILS.size()
        ),
        "OK",
        folderButton ? "Folder" : nullptr,
        300.0f,
        [folderButton](auto, bool btn2) {
            if (folderButton && btn2) file::openFolder(Mod::get()->getConfigDir());
        }
    );
}
