#include "MoreIcons.hpp"
#include <BS_thread_pool.hpp>
#include <Geode/binding/GameManager.hpp>
#include <Geode/loader/Dirs.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/Popup.hpp>
#include <geode.texture-loader/include/TextureLoader.hpp>
#include <Geode/utils/ranges.hpp>

using namespace geode::prelude;

$on_mod(Loaded) {
    MoreIcons::DEBUG_LOGS = Mod::get()->getSettingValue<bool>("debug-logs");
    MoreIcons::TRADITIONAL_PACKS = Mod::get()->getSettingValue<bool>("traditional-packs");
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

struct ImageData {
    CCImage* image;
    CCDictionary* dict;
    std::string texturePath;
    std::string name;
    std::string frameName;
    TexturePack pack;
    int index;
    int trailID;
    bool blend;
    bool tint;
};

static std::vector<ImageData> IMAGES;
static std::mutex IMAGE_MUTEX;
static std::mutex LOG_MUTEX;

std::vector<IconPack> MoreIcons::getTexturePacks() {
    std::vector<IconPack> packs;

    packs.push_back({
        .name = "More Icons",
        .id = "",
        .path = dirs::getGeodeDir(),
        .vanilla = false
    });
    for (auto& pack : texture_loader::getAppliedPacks()) {
        if (MoreIcons::TRADITIONAL_PACKS) {
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

#ifdef GEODE_IS_ANDROID
void MoreIcons::unzipVanillaAssets() {
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
    if (file::Unzip::intoDir(*reinterpret_cast<const char**>(dlsym(dlopen("libcocos2dcpp.so", RTLD_NOW), "g_apkPath")), tempDir / "vanilla")
        .mapErr([](const std::string& err) {
            return log::error("Failed to unzip vanilla assets: {}", err), err;
        }).isErr()) return;
    std::error_code renameCode;
    std::filesystem::rename(tempDir / "vanilla" / "assets", tempDir / "assets", renameCode);
    if (renameCode) return log::error("Failed to move assets folder: {}", renameCode.message());
    std::error_code removeCode;
    std::filesystem::remove_all(tempDir / "vanilla", removeCode);
    if (removeCode) return log::error("Failed to remove vanilla folder: {}", removeCode.message());
}
#endif

std::string MoreIcons::vanillaTexturePath(const std::string& path, bool skipSuffix) {
    #ifdef GEODE_IS_ANDROID
    if (CCDirector::get()->getContentScaleFactor() >= 4.0f && !skipSuffix) {
        if (auto highGraphicsAndroid = Loader::get()->getLoadedMod("weebify.high-graphics-android")) {
            auto configDir = highGraphicsAndroid->getConfigDir(false) / Loader::get()->getGameVersion();
            if (std::filesystem::exists(configDir)) return configDir / path;
        }
    }
    else {
        auto assetsDir = Mod::get()->getTempDir() / "assets";
        if (std::filesystem::exists(assetsDir)) return assetsDir / path;
    }
    return path;
    #else
    return (dirs::getGameDir() / "Resources" / path).string();
    #endif
}

bool naturalSorter(const std::string& aStr, const std::string& bStr) {
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

void naturalSort(std::vector<std::string>& vec) {
    auto packs = ranges::reduce<std::map<std::string, std::vector<std::string>>>(vec,
        [](std::map<std::string, std::vector<std::string>>& acc, const std::string& str) {
            acc[str.substr(0, string::contains(str, ':') ? str.find_first_of(':') : 0)].push_back(str);
            return acc;
        });

    auto packIDs = ranges::map<std::vector<std::string>>(packs, [](const std::pair<std::string, std::vector<std::string>>& pair) {
        return pair.first;
    });
    std::ranges::sort(packIDs, naturalSorter);

    vec.clear();
    vec = ranges::reduce<std::vector<std::string>>(packIDs, [&packs](std::vector<std::string>& acc, const std::string& packID) {
        std::ranges::sort(packs[packID], naturalSorter);
        return ranges::push(acc, packs[packID]);
    });
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

#ifdef GEODE_IS_WINDOWS // I simply cannot believe this
std::string replaceEnd(const std::filesystem::path& path, size_t end, std::string_view replace) {
    return replaceEnd(path.string(), end, replace);
}
#endif

std::string replaceEnd(const std::string& str, size_t end, std::string_view replace) {
    return fmt::format("{}{}", str.substr(0, str.size() - end), replace);
}

BS::thread_pool<BS::tp::none>& sharedPool() {
    static BS::thread_pool _sharedPool(std::thread::hardware_concurrency());
    return _sharedPool;
}

template <typename... Args>
void printLog(Severity severity, fmt::format_string<Args...> message, Args&&... args) {
    log::logImpl(severity, Mod::get(), message, std::forward<Args>(args)...);
    {
        std::lock_guard lock(LOG_MUTEX);
        MoreIcons::LOGS.push_back({ fmt::format(message, std::forward<Args>(args)...), severity });
        if (MoreIcons::HIGHEST_SEVERITY < severity) MoreIcons::HIGHEST_SEVERITY = severity;
    }
}

template <typename... Args>
void safeDebug(fmt::format_string<Args...> message, Args&&... args) {
    if (MoreIcons::DEBUG_LOGS) log::debug(message, std::forward<Args>(args)...);
}

void loadFolderIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    sharedPool().detach_task([pack, path, type] {
        auto scaleFactor = CCDirector::get()->getContentScaleFactor();
        auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());

        safeDebug("Loading folder icon {} from {}", name, pack.name);
        auto textureCache = CCTextureCache::get();
        auto spriteFrameCache = CCSpriteFrameCache::get();
        int i = 0;
        for (auto& subEntry : std::filesystem::directory_iterator(path)) {
            if (!subEntry.is_regular_file()) continue;

            auto subEntryPath = subEntry.path();
            if (subEntryPath.extension() != ".png") continue;

            auto pathFilename = subEntryPath.filename().string();
            auto fileQuality = kTextureQualityLow;
            if (pathFilename.ends_with("-uhd.png")) {
                if (scaleFactor < 4.0f) {
                    printLog(Severity::Info, "{}: Ignoring high-quality PNG file for {} texture quality",
                        subEntryPath, scaleFactor >= 2.0f ? "medium" : "low");
                    continue;
                }

                fileQuality = kTextureQualityHigh;
            }
            else if (pathFilename.ends_with("-hd.png")) {
                if (scaleFactor < 2.0f) {
                    printLog(Severity::Info, "{}: Ignoring medium-quality PNG file for low texture quality", subEntryPath);
                    continue;
                }

                if (std::filesystem::exists(replaceEnd(subEntryPath, 7, "-uhd.png")) && scaleFactor >= 4.0f) continue;
                else fileQuality = kTextureQualityMedium;
            }
            else {
                if (std::filesystem::exists(replaceEnd(subEntryPath, 4, "-uhd.png")) && scaleFactor >= 4.0f) continue;
                else if (std::filesystem::exists(replaceEnd(subEntryPath, 4, "-hd.png")) && scaleFactor >= 2.0f) continue;
                else fileQuality = kTextureQualityLow;
            }

            std::string pngPath;
            if (fileQuality == kTextureQualityHigh) pngPath = replaceEnd(subEntryPath, 8, ".png");
            else if (fileQuality == kTextureQualityMedium) pngPath = replaceEnd(subEntryPath, 7, ".png");
            else pngPath = subEntryPath.string();
            auto image = new CCImage();
            if (image->initWithImageFileThreadSafe(subEntryPath.string().c_str())) {
                std::lock_guard lock(IMAGE_MUTEX);
                if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) {
                    printLog(Severity::Warning, "{}: Duplicate icon name {}", subEntryPath, name);
                    return image->release();
                }
                IMAGES.push_back({
                    .image = image,
                    .dict = nullptr,
                    .texturePath = subEntryPath.string(),
                    .name = name,
                    .frameName = getFrameName(string::contains(pngPath, '/') ?
                        pngPath.substr(pngPath.find_last_of('/') + 1) : pngPath, name, type),
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
        safeDebug("Finished loading folder icon {} from {}", name, pack.name);
    });
}

void loadFileIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    sharedPool().detach_task([pack, path, type] {
        auto scaleFactor = CCDirector::get()->getContentScaleFactor();
        auto pathFilename = path.filename().string();
        auto fileQuality = kTextureQualityLow;
        if (pathFilename.ends_with("-uhd.plist")) {
            if (scaleFactor < 4.0f) return printLog(Severity::Info, "{}: Ignoring high-quality plist file for {} texture quality",
                    path, scaleFactor >= 2.0f ? "medium" : "low");

            fileQuality = kTextureQualityHigh;
        }
        else if (pathFilename.ends_with("-hd.plist")) {
            if (scaleFactor < 2.0f) return printLog(Severity::Info, "{}: Ignoring medium-quality plist file for low texture quality", path);

            if (std::filesystem::exists(replaceEnd(path, 9, "-uhd.plist")) && scaleFactor >= 4.0f) return;
            else fileQuality = kTextureQualityMedium;
        }
        else {
            if (std::filesystem::exists(replaceEnd(path, 6, "-uhd.plist")) && scaleFactor >= 4.0f) return;
            else if (std::filesystem::exists(replaceEnd(path, 6, "-hd.plist")) && scaleFactor >= 2.0f) return;
            else fileQuality = kTextureQualityLow;
        }

        auto name = fileQuality == kTextureQualityHigh ? replaceEnd(path.stem(), 4, "") :
            fileQuality == kTextureQualityMedium ? replaceEnd(path.stem(), 3, "") : path.stem().string();
        if (!pack.id.empty()) name = fmt::format("{}:{}", pack.id, name);
        safeDebug("Loading file icon {} from {}", name, pack.name);
        auto dict = CCDictionary::createWithContentsOfFileThreadSafe(path.string().c_str());
        auto frames = new CCDictionary();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
            frames->setObject(frame, getFrameName(frameName, name, type));
        }
        dict->setObject(frames, "frames");
        frames->release();

        auto fullTexturePath = replaceEnd(path, 6, ".png");
        if (!std::filesystem::exists(fullTexturePath)) {
            std::string texturePath = static_cast<CCDictionary*>(dict->objectForKey("metadata"))->valueForKey("textureFileName")->m_sString;
            texturePath = string::contains(texturePath, '/') ? texturePath.substr(texturePath.find_last_of('/') + 1) : texturePath;
            auto fallbackTexturePath = path.parent_path() / texturePath;
            if (!std::filesystem::exists(fallbackTexturePath)) {
                printLog(Severity::Error, "{}: Texture file {} not found", path, texturePath);
                return dict->release();
            }
            else fullTexturePath = fallbackTexturePath.string();
        }

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(fullTexturePath.c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) {
                printLog(Severity::Warning, "{}: Duplicate icon name {}", path, name);
                dict->release();
                return image->release();
            }
            IMAGES.push_back({
                .image = image,
                .dict = dict,
                .texturePath = fullTexturePath,
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
        safeDebug("Finished loading file icon {} from {}", name, pack.name);
    });
}

void loadVanillaIcon(const std::filesystem::path& path, const IconPack& pack, IconType type) {
    sharedPool().detach_task([pack, path, type] {
        auto scaleFactor = CCDirector::get()->getContentScaleFactor();
        auto pathFilename = path.filename().string();
        auto fileQuality = kTextureQualityLow;
        if (pathFilename.ends_with("-uhd.png")) {
            if (scaleFactor < 4.0f) return;
            fileQuality = kTextureQualityHigh;
        }
        else if (pathFilename.ends_with("-hd.png")) {
            if (scaleFactor < 2.0f) return;
            if (std::filesystem::exists(replaceEnd(path, 7, "-uhd.png")) && scaleFactor >= 4.0f) return;
            else fileQuality = kTextureQualityMedium;
        }
        else {
            if (std::filesystem::exists(replaceEnd(path, 4, "-uhd.png")) && scaleFactor >= 4.0f) return;
            else if (std::filesystem::exists(replaceEnd(path, 4, "-hd.png")) && scaleFactor >= 2.0f) return;
            else fileQuality = kTextureQualityLow;
        }

        auto name = fmt::format("{}:{}", pack.id, fileQuality == kTextureQualityHigh ? replaceEnd(path.stem(), 4, "") :
            fileQuality == kTextureQualityMedium ? replaceEnd(path.stem(), 3, "") : path.stem().string());
        safeDebug("Loading vanilla icon {} from {}", name, pack.name);
        auto plistPath = replaceEnd(path, 4, ".plist");
        if (!std::filesystem::exists(plistPath)) plistPath = MoreIcons::vanillaTexturePath(fmt::format("icons/{}.plist", path.stem()), false);
        if (!std::filesystem::exists(plistPath)) return printLog(Severity::Error, "{}: Plist file not found (Last attempt: {})", path, plistPath);

        auto dict = CCDictionary::createWithContentsOfFileThreadSafe(plistPath.c_str());
        auto frames = new CCDictionary();
        for (auto [frameName, frame] : CCDictionaryExt<std::string, CCDictionary*>(static_cast<CCDictionary*>(dict->objectForKey("frames")))) {
            frames->setObject(frame, getFrameName(frameName, name, type));
        }
        dict->setObject(frames, "frames");
        frames->release();

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(path.string().c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty()) {
                for (auto& image : IMAGES) {
                    if (image.name != name) continue;
                    safeDebug("Removing duplicate icon {}", name);
                    CC_SAFE_RELEASE(image.image);
                    CC_SAFE_RELEASE(image.dict);
                }
                ranges::remove(IMAGES, [name](const ImageData& image) { return image.name == name; });
            }
            IMAGES.push_back({
                .image = image,
                .dict = dict,
                .texturePath = path.string(),
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
        safeDebug("Finished loading vanilla icon {} from {}", name, pack.name);
    });
}

void loadTrail(const std::filesystem::path& path, const IconPack& pack) {
    sharedPool().detach_task([pack, path] {
        auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());

        safeDebug("Loading trail {} from {}", name, pack.name);
        auto json = file::readJson(replaceEnd(path, 4, ".json")).unwrapOr(matjson::makeObject({
            { "blend", false },
            { "tint", false }
        }));

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(path.string().c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty() && ranges::contains(IMAGES, [name](const ImageData& image) { return image.name == name; })) {
                printLog(Severity::Warning, "{}: Duplicate trail name {}", path, name);
                return image->release();
            }
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
                .trailID = 0,
                .blend = json.contains("blend") ? json["blend"].asBool().unwrapOr(false) : false,
                .tint = json.contains("tint") ? json["tint"].asBool().unwrapOr(false) : false,
            });
        }
        else image->release();
        safeDebug("Finished loading trail {} from {}", name, pack.name);
    });
}

void loadVanillaTrail(const std::filesystem::path& path, const IconPack& pack) {
    sharedPool().detach_task([pack, path] {
        auto pathStem = path.stem().string();
        auto name = fmt::format("{}:{}", pack.id, pathStem);

        safeDebug("Loading vanilla trail {} from {}", name, pack.name);
        auto trailID = numFromString<int>(pathStem.substr(
            pathStem.find_first_of('_') + 1, pathStem.find_last_of('_') - pathStem.find_first_of('_') - 1)).unwrapOr(0);
        if (trailID <= 0) return;

        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(path.string().c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            if (!pack.id.empty()) {
                for (auto& image : IMAGES) {
                    if (image.name != name) continue;
                    safeDebug("Removing duplicate trail {}", name);
                    CC_SAFE_RELEASE(image.image);
                }
                ranges::remove(IMAGES, [name](const ImageData& image) { return image.name == name; });
            }
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
        safeDebug("Finished loading vanilla trail {} from {}", name, pack.name);
    });
}

namespace {
    template <typename TC>
    using priv_method_t = void(TC::*)(CCDictionary*, CCTexture2D*);

    template <typename TC, priv_method_t<TC> func>
    struct priv_caller {
        friend void addSpriteFrames(CCDictionary* dict, CCTexture2D* texture) {
            (CCSpriteFrameCache::get()->*func)(dict, texture);
        }
    };

    template struct priv_caller<CCSpriteFrameCache, &CCSpriteFrameCache::addSpriteFramesWithDictionary>;

    void addSpriteFrames(CCDictionary* dict, CCTexture2D* texture);
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

            log::info("Loading {}s from {}", suffix, path);
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    auto& entryPath = entry.path();
                    if (entryPath.extension() != ".plist") continue;

                    loadFileIcon(entryPath, pack, type);
                }
                else if (entry.is_directory()) loadFolderIcon(entry.path(), pack, type);
            }

            sharedPool().wait();
            log::info("Finished loading {}s from {}", suffix, path);
        }
        else {
            if (!TRADITIONAL_PACKS) continue;

            auto path = pack.path / "icons";
            if (!std::filesystem::exists(path)) continue;

            log::info("Loading {}s from {}", suffix, path);
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

            sharedPool().wait();
            log::info("Finished loading {}s from {}", suffix, path);
        }
    }

    {
        std::lock_guard lock(IMAGE_MUTEX);

        log::info("Loading {} {}{}", IMAGES.size(), suffix, IMAGES.size() == 1 ? "" : "s");
        auto textureCache = CCTextureCache::get();
        auto spriteFrameCache = CCSpriteFrameCache::get();
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
                    addSpriteFrames(image.dict, texture);
                    image.dict->release();
                }
                if (image.index == 0) {
                    vec.push_back(image.name);
                    infoForType(type)[image.name] = image.pack;
                }
            }

            texture->release();
            CC_SAFE_RELEASE(image.image);
        }

        log::info("Finished loading {} {}{}", vec.size(), suffix, vec.size() == 1 ? "" : "s");
        IMAGES.clear();
        naturalSort(vec);
    }
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

            log::info("Loading trails from {}", path);
            for (auto& entry : std::filesystem::directory_iterator(path)) {
                if (!entry.is_regular_file()) continue;

                auto& entryPath = entry.path();
                if (entryPath.extension() != ".png") continue;

                loadTrail(entryPath, {
                    .name = pack.name,
                    .id = pack.id
                });
            }

            sharedPool().wait();
            log::info("Finished loading trails from {}", path);
        }
        else {
            if (!TRADITIONAL_PACKS) continue;

            log::info("Loading trails from {}", pack.path);
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

            sharedPool().wait();
            log::info("Finished loading trails from {}", pack.path);
        }
    }

    {
        std::lock_guard lock(IMAGE_MUTEX);

        log::info("Loading {} trail{}", IMAGES.size(), IMAGES.size() == 1 ? "" : "s");
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

        log::info("Finished loading {} trail{}", MoreIconsAPI::TRAILS.size(), MoreIconsAPI::TRAILS.size() == 1 ? "" : "s");
        IMAGES.clear();
        naturalSort(MoreIconsAPI::TRAILS);
    }
}

void MoreIcons::saveTrails() {
    for (auto& [trail, info] : TRAIL_INFO) {
        if (info.trailID > 0) continue;
        (void)file::writeToJson(replaceEnd(info.texture, 4, ".json"), matjson::makeObject({
            { "blend", info.blend },
            { "tint", info.tint },
        })).mapErr([](const std::string& err) {
            return log::error("Failed to save trail JSON: {}", err), err;
        });
    }
}

std::unordered_map<std::string, TexturePack>& MoreIcons::infoForType(IconType type) {
    switch (type) {
        case IconType::Cube: return ICON_INFO;
        case IconType::Ship: return SHIP_INFO;
        case IconType::Ball: return BALL_INFO;
        case IconType::Ufo: return UFO_INFO;
        case IconType::Wave: return WAVE_INFO;
        case IconType::Robot: return ROBOT_INFO;
        case IconType::Spider: return SPIDER_INFO;
        case IconType::Swing: return SWING_INFO;
        case IconType::Jetpack: return JETPACK_INFO;
        default: {
            static std::unordered_map<std::string, TexturePack> empty;
            return empty;
        }
    }
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
