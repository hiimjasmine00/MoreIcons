#include "MoreIcons.hpp"
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

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
}

$on_mod(DataSaved) {
    MoreIcons::saveTrails();
}

std::vector<std::filesystem::directory_entry> MoreIcons::naturalSort(const std::filesystem::path& path) {
    std::vector<std::filesystem::directory_entry> entries;
    for (auto& entry : std::filesystem::directory_iterator(path)) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
        auto aStr = a.path().filename().string();
        auto bStr = b.path().filename().string();

        auto aIt = aStr.begin();
        auto bIt = bStr.begin();

        while (aIt != aStr.end() && bIt != bStr.end()) {
            if (std::isdigit(*aIt) && std::isdigit(*bIt)) {
                std::string aNum, bNum;
                while (std::isdigit(*aIt)) aNum += *aIt++;
                while (std::isdigit(*bIt)) bNum += *bIt++;
                if (aNum != bNum) return std::stoi(aNum) < std::stoi(bNum);
            }
            else {
                auto aLower = std::tolower(*aIt);
                auto bLower = std::tolower(*bIt);
                if (aLower != bLower) return aLower < bLower;
                aIt++;
                bIt++;
            }
        }

        return aStr.size() < bStr.size();
    });
    return entries;
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
            if (aNum != bNum) return std::stoi(aNum) < std::stoi(bNum);
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

// The cooler https://github.com/Alphalaneous/HappyTextures/blob/1.5.0/src/Utils.h#L60
std::vector<std::filesystem::path> MoreIcons::getTexturePacks() {
    std::vector<std::filesystem::path> packs;
    packs.push_back(Mod::get()->getConfigDir());
    if (auto textureLoader = Loader::get()->getLoadedMod("geode.texture-loader")) {
        auto searchPaths = CCFileUtils::get()->getSearchPaths();
        auto texturePacks = textureLoader->getConfigDir() / "packs";
        auto unzippedPacks = textureLoader->getSaveDir() / "unzipped";
        auto modID = Mod::get()->getID();
        for (auto searchPath : searchPaths) {
            auto path = std::filesystem::path(searchPath);
            auto parentPath = std::filesystem::path(searchPath);
            while (parentPath.has_parent_path()) {
                if (parentPath == texturePacks || parentPath == unzippedPacks && std::find(packs.begin(), packs.end(), path) == packs.end()) {
                    auto configPath = path / "config" / modID;
                    if (std::filesystem::exists(configPath)) packs.push_back(configPath);
                    break;
                }
                if (parentPath == std::filesystem::current_path().root_path()) break;
                parentPath = parentPath.parent_path();
            }
        }
    }
    return packs;
}

void MoreIcons::loadIcons(const std::vector<std::filesystem::path>& packs, std::string_view suffix, IconType type) {
    int i = 0;
    for (auto& packPath : packs) {
        std::string packName;
        std::string packID;
        if (i != 0) {
            auto rootPackPath = packPath.parent_path().parent_path();
            auto packJSON = rootPackPath / "pack.json";
            if (std::filesystem::exists(packJSON)) {
                std::ifstream file(packJSON);
                std::stringstream bufferStream;
                bufferStream << file.rdbuf();
                auto tryJson = matjson::parse(bufferStream.str());
                auto packFileName = rootPackPath.filename().string();
                if (!tryJson.isOk()) {
                    packName = packFileName;
                    packID = packFileName;
                }
                auto json = tryJson.unwrapOr(matjson::Value());
                if (json.contains("name") && json["name"].isString()) packName = json["name"].asString().unwrap();
                else packName = packFileName;
                if (json.contains("id") && json["id"].isString()) packID = json["id"].asString().unwrap();
                else packID = packFileName;
            }
        }
        else {
            packName = "More Icons";
            packID = "";
        }

        auto path = packPath / suffix;
        if (!std::filesystem::exists(path)) {
            if (i == 0) std::filesystem::create_directories(path);
            continue;
        }
        log::info("Loading {}s from {}", suffix, path.string());

        for (auto& entry : naturalSort(path)) {
            if (!entry.is_regular_file() && !entry.is_directory()) continue;

            loadIcon(entry.path(), {
                .name = packName,
                .id = packID
            }, type);
        }

        i++;
    }

    sharedPool().wait();

    {
        std::lock_guard lock(IMAGE_MUTEX);
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
        switch (type) {
            case IconType::Cube: Mod::get()->setSavedValue("icons", vec); break;
            case IconType::Ship: Mod::get()->setSavedValue("ships", vec); break;
            case IconType::Ball: Mod::get()->setSavedValue("balls", vec); break;
            case IconType::Ufo: Mod::get()->setSavedValue("ufos", vec); break;
            case IconType::Wave: Mod::get()->setSavedValue("waves", vec); break;
            case IconType::Robot: Mod::get()->setSavedValue("robots", vec); break;
            case IconType::Spider: Mod::get()->setSavedValue("spiders", vec); break;
            case IconType::Swing: Mod::get()->setSavedValue("swings", vec); break;
            case IconType::Jetpack: Mod::get()->setSavedValue("jetpacks", vec); break;
            default: break;
        }
    }
}

void MoreIcons::loadIcon(const std::filesystem::path& path, const TexturePack& pack, IconType type) {
    if (!std::filesystem::exists(path)) return;

    auto textureQuality = CCDirector::get()->getLoadedTextureQuality();
    if (std::filesystem::is_directory(path)) {
        sharedPool().detach_task([path, pack, textureQuality, type] {
            auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());
            auto textureCache = CCTextureCache::get();
            auto spriteFrameCache = CCSpriteFrameCache::get();
            int i = 0;
            for (auto& subEntry : naturalSort(path)) {
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
                    IMAGES.push_back({
                        .image = image,
                        .dict = nullptr,
                        .texturePath = pngPath,
                        .name = name,
                        .frameName = getFrameName(std::filesystem::path(noGraphicPngPath).filename().string(), name, type),
                        .pack = pack,
                        .index = i
                    });

                    i++;
                }
                else image->release();
            }
        });
    }
    else if (std::filesystem::is_regular_file(path)) {
        if (path.extension() != ".plist") return;

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
                IMAGES.push_back({
                    .image = image,
                    .dict = dict,
                    .texturePath = fullTexturePath.string(),
                    .name = name,
                    .frameName = "",
                    .pack = pack,
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

void MoreIcons::loadTrails(const std::vector<std::filesystem::path>& packs) {
    int i = 0;
    for (auto& packPath : packs) {
        std::string packName;
        std::string packID;
        if (i != 0) {
            auto rootPackPath = packPath.parent_path().parent_path();
            auto packJSON = rootPackPath / "pack.json";
            if (std::filesystem::exists(packJSON)) {
                std::ifstream file(packJSON);
                std::stringstream bufferStream;
                bufferStream << file.rdbuf();
                auto tryJson = matjson::parse(bufferStream.str());
                auto packFileName = rootPackPath.filename().string();
                if (!tryJson.isOk()) {
                    packName = packFileName;
                    packID = packFileName;
                }
                auto json = tryJson.unwrapOr(matjson::Value());
                if (json.contains("name") && json["name"].isString()) packName = json["name"].asString().unwrap();
                else packName = packFileName;
                if (json.contains("id") && json["id"].isString()) packID = json["id"].asString().unwrap();
                else packID = packFileName;
            }
        }
        else {
            packName = "More Icons";
            packID = "";
        }

        auto path = packPath / "trail";
        if (!std::filesystem::exists(path)) {
            if (i == 0) std::filesystem::create_directories(path);
            continue;
        }
        log::info("Loading trails from {}", path.string());

        for (auto& entry : naturalSort(path)) {
            if (!entry.is_regular_file()) continue;

            auto entryPath = entry.path();
            if (entryPath.extension() != ".png") continue;

            loadTrail(entryPath, {
                .name = packName,
                .id = packID
            });
        }

        i++;
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
        Mod::get()->setSavedValue("trails", MoreIconsAPI::TRAILS);
    }
}

void MoreIcons::loadTrail(const std::filesystem::path& path, const TexturePack& pack) {
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path) || path.extension() != ".png") return;

    sharedPool().detach_task([path, pack] {
        auto name = pack.id.empty() ? path.stem().string() : fmt::format("{}:{}", pack.id, path.stem());
        auto jsonPath = std::filesystem::path(path).replace_extension(".json");
        matjson::Value json;
        if (!std::filesystem::exists(jsonPath)) json = matjson::makeObject({ { "blend", false }, { "tint", false } });
        else {
            std::ifstream file(jsonPath);
            auto tryJson = matjson::parse(file);
            if (!tryJson.isOk()) {
                auto logMessage = fmt::format("{}: Failed to parse JSON file ({})", path.string(), tryJson.unwrapErr());
                log::warn("{}", logMessage);
                {
                    std::lock_guard lock(LOG_MUTEX);
                    LOGS.push_back({ .message = logMessage, .type = LogType::Warn });
                    if (HIGHEST_SEVERITY < LogType::Warn) HIGHEST_SEVERITY = LogType::Warn;
                }
                json = matjson::makeObject({ { "blend", false }, { "tint", false } });
            }
            else json = tryJson.unwrap();
        }

        auto fullTexturePath = path.string();
        auto image = new CCImage();
        if (image->initWithImageFileThreadSafe(fullTexturePath.c_str())) {
            std::lock_guard lock(IMAGE_MUTEX);
            IMAGES.push_back({
                .image = image,
                .dict = nullptr,
                .texturePath = fullTexturePath,
                .name = name,
                .frameName = "",
                .pack = pack,
                .index = 0,
                .blend = json.contains("blend") && json["blend"].isBool() ? json["blend"].asBool().unwrap() : false,
                .tint = json.contains("tint") && json["tint"].isBool() ? json["tint"].asBool().unwrap() : false,
            });
        }
        else image->release();
    });
}

SimplePlayer* MoreIcons::findPlayer(cocos2d::CCNode* node) {
    if (!node) return nullptr;
    return geode::cocos::findFirstChildRecursive<SimplePlayer>(node, [](auto) { return true; });
}

void MoreIcons::showInfoPopup(bool folderButton) {
    geode::createQuickPopup(
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
            if (folderButton && btn2) geode::utils::file::openFolder(geode::Mod::get()->getConfigDir());
        }
    );
}
