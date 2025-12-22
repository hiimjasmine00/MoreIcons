#include <pugixml.hpp>
#include "Load.hpp"
#include "Get.hpp"
#include <Geode/cocos/support/zip_support/unzip.h>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/file.hpp>
#include <jasmine/mod.hpp>
#include <texpack.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;

std::string Load::getFrameName(std::string_view frameName, std::string_view name, IconType type) {
    if (type == IconType::DeathEffect) {
        return fmt::format("{}{}"_spr, name, frameName.substr(std::max<ptrdiff_t>(frameName.size() - 8, 0)));
    }

    if (!frameName.ends_with("_001.png")) return std::string(frameName);

    std::string_view suffix;
    auto isRobot = type == IconType::Robot || type == IconType::Spider;
    auto end = std::string_view(frameName.data(), frameName.size() - 8);

    if (end.ends_with("_2")) {
        if (isRobot) {
            end.remove_suffix(2);
            if (end.ends_with("_01")) suffix = "_01_2_001.png";
            else if (end.ends_with("_02")) suffix = "_02_2_001.png";
            else if (end.ends_with("_03")) suffix = "_03_2_001.png";
            else if (end.ends_with("_04")) suffix = "_04_2_001.png";
        }
        else suffix = "_2_001.png";
    }
    else if (type == IconType::Ufo && end.ends_with("_3")) {
        suffix = "_3_001.png";
    }
    else if (end.ends_with("_extra")) {
        if (isRobot) {
            end.remove_suffix(6);
            if (end.ends_with("_01")) suffix = "_01_extra_001.png";
        }
        else suffix = "_extra_001.png";
    }
    else if (end.ends_with("_glow")) {
        if (isRobot) {
            end.remove_suffix(5);
            if (end.ends_with("_01")) suffix = "_01_glow_001.png";
            else if (end.ends_with("_02")) suffix = "_02_glow_001.png";
            else if (end.ends_with("_03")) suffix = "_03_glow_001.png";
            else if (end.ends_with("_04")) suffix = "_04_glow_001.png";
        }
        else suffix = "_glow_001.png";
    }
    else {
        if (isRobot) {
            if (end.ends_with("_01")) suffix = "_01_001.png";
            else if (end.ends_with("_02")) suffix = "_02_001.png";
            else if (end.ends_with("_03")) suffix = "_03_001.png";
            else if (end.ends_with("_04")) suffix = "_04_001.png";
        }
        else suffix = "_001.png";
    }

    return suffix.empty() ? std::string(frameName) : name.empty() ? std::string(suffix) : fmt::format("{}{}"_spr, name, suffix);
}

#ifdef GEODE_IS_ANDROID
const char* getApkPath();

struct ApkEntry {
    unz_file_pos pos;
    uint64_t size;
};

struct ApkFile {
    void* zipFile = nullptr;
    std::unordered_map<std::string, ApkEntry> fileList;
};

thread_local ApkFile* apkFile = [] -> ApkFile* {
    ApkFile* apkFile = new ApkFile();

    auto apkPath = getApkPath();
    auto zipFile = unzOpen(apkPath);
    if (!zipFile) {
        log::error("Failed to open APK file at {}", apkPath);
        return apkFile;
    }

    apkFile->zipFile = zipFile;

    unz_file_info64 fileInfo;
    std::string filename;
    filename.reserve(256);

    auto& fileList = apkFile->fileList;
    auto res = unzGoToFirstFile64(zipFile, &fileInfo, filename.data(), 256);
    for (; res == 0; res = unzGoToNextFile64(zipFile, &fileInfo, filename.data(), 256)) {
        if (!filename.starts_with("assets/")) continue;
        ApkEntry entryInfo;
        if (unzGetFilePos(zipFile, &entryInfo.pos) != 0) continue;
        entryInfo.size = fileInfo.uncompressed_size;
        fileList[filename] = entryInfo;
    }

    if (res != 0) {
        if (filename.empty()) log::error("Failed to iterate over APK file entries");
        else log::error("APK file iteration prematurely aborted after {}", filename);
    }

    return apkFile;
}();
#endif

Result<std::vector<uint8_t>> Load::readBinary(const std::filesystem::path& path) {
    #ifdef GEODE_IS_ANDROID
    auto& str = path.native();
    if (str.starts_with("assets/")) {
        auto& fileList = apkFile->fileList;
        if (auto it = fileList.find(str); it != fileList.end()) {
            auto& entryInfo = it->second;
            auto zipFile = apkFile->zipFile;

            if (unzGoToFilePos(zipFile, &entryInfo.pos) != 0) return Err("Failed to seek to file");
            if (unzOpenCurrentFile(zipFile) != 0) return Err("Failed to open file");

            std::vector<uint8_t> data(entryInfo.size);
            auto readBytes = unzReadCurrentFile(zipFile, data.data(), data.size());
            unzCloseCurrentFile(zipFile);
            if (readBytes != entryInfo.size) return Err("Failed to read file");
            return Ok(std::move(data));
        }
        else return Err("File not found");
    }
    #endif
    return file::readBinary(path);
}

bool Load::doesExist(const std::filesystem::path& path) {
    #ifdef GEODE_IS_ANDROID
    auto& str = path.native();
    if (str.starts_with("assets/")) return apkFile->fileList.contains(str);
    #endif
    std::error_code code;
    return std::filesystem::exists(path, code);
}

Result<Autorelease<cocos2d::CCTexture2D>> Load::createTexture(const std::filesystem::path& path) {
    GEODE_UNWRAP_INTO(auto data, readBinary(path).mapErr([](std::string err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data).mapErr([](std::string err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    return Ok(createTexture(image.data.data(), image.width, image.height));
}

Autorelease<cocos2d::CCTexture2D> Load::createTexture(const uint8_t* data, uint32_t width, uint32_t height) {
    Autorelease texture = new CCTexture2D();
    initTexture(texture, data, width, height, false);
    return texture;
}

void Load::initTexture(cocos2d::CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha) {
    texture->initWithData(data, kCCTexture2DPixelFormat_RGBA8888, width, height, { (float)width, (float)height });
    texture->m_bHasPremultipliedAlpha = premultiplyAlpha;
}

Result<ImageResult> Load::createFrames(
    const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name, IconType type, bool premultiplyAlpha
) {
    GEODE_UNWRAP_INTO(auto data, readBinary(png).mapErr([](std::string err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data, premultiplyAlpha).mapErr([](std::string err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    Autorelease texture = new CCTexture2D();
    GEODE_UNWRAP_INTO(auto frames, createFrames(plist, texture, name, type, !premultiplyAlpha || !name.empty()).mapErr([](std::string err) {
        return fmt::format("Failed to load frames: {}", err);
    }));

    ImageResult result;
    result.name = string::pathToString(png);
    result.data = std::move(image.data);
    result.texture = std::move(texture);
    result.frames = std::move(frames);
    result.width = image.width;
    result.height = image.height;
    return Ok(std::move(result));
}

matjson::Value parseNode(const pugi::xml_node& node) {
    std::string_view name = node.name();
    if (name == "dict") {
        auto json = matjson::Value::object();
        for (auto child = node.child("key"); !child.empty(); child = child.next_sibling("key")) {
            json[child.text().as_string()] = parseNode(child.next_sibling());
        }
        return json;
    }
    else if (name == "array") {
        auto json = matjson::Value::array();
        for (auto child = node.first_child(); !child.empty(); child = child.next_sibling()) {
            json.push(parseNode(child));
        }
        return json;
    }
    else if (name == "string" || name == "data" || name == "date") return node.text().as_string();
    else if (name == "real") return node.text().as_double();
    else if (name == "integer") return node.text().as_llong();
    else if (name == "true") return true;
    else if (name == "false") return false;
    else return nullptr;
}

Result<Autorelease<CCDictionary>> Load::createFrames(
    const std::filesystem::path& path, CCTexture2D* texture, std::string_view name, IconType type, bool fixNames
) {
    if (path.empty()) return Ok(nullptr);

    GEODE_UNWRAP_INTO(auto data, readBinary(path).mapErr([](std::string err) {
        return fmt::format("Failed to read file: {}", err);
    }));

    pugi::xml_document doc;
    auto result = doc.load_buffer(data.data(), data.size());
    if (!result) return Err("Failed to parse XML: {}", result.description());

    auto root = doc.child("plist");
    if (!root) return Err("No root <plist> element found");

    auto json = parseNode(root.first_child());
    if (!json.isObject()) return Err("No root <dict> element found");
    if (!json.contains("frames")) return Err("No frames <dict> element found");

    auto format = json.get("metadata").andThen([](const matjson::Value& v) {
        return v.get<int>("format");
    }).unwrapOr(0);

    Autorelease frames = new CCDictionary();
    for (auto& [frameName, obj] : json["frames"]) {
        if (!obj.isObject()) continue;

        Autorelease frame = new CCSpriteFrame();
        frames->setObject(frame, fixNames ? getFrameName(frameName, name, type) : frameName);

        CCRect rect;
        CCPoint offset;
        CCSize originalSize;
        auto rotated = false;

        switch (format) {
            case 0: {
                if (auto x = obj.get<float>("x")) {
                    rect.origin.x = x.unwrap();
                }
                if (auto y = obj.get<float>("y")) {
                    rect.origin.y = y.unwrap();
                }
                if (auto w = obj.get<float>("width")) {
                    rect.size.width = w.unwrap();
                }
                if (auto h = obj.get<float>("height")) {
                    rect.size.height = h.unwrap();
                }
                if (auto offsetX = obj.get<float>("offsetX")) {
                    offset.x = offsetX.unwrap();
                }
                if (auto offsetY = obj.get<float>("offsetY")) {
                    offset.y = offsetY.unwrap();
                }
                if (auto originalWidth = obj.get<float>("originalWidth")) {
                    originalSize.width = abs(floor(originalWidth.unwrap()));
                }
                if (auto originalHeight = obj.get<float>("originalHeight")) {
                    originalSize.height = abs(floor(originalHeight.unwrap()));
                }
                break;
            }
            case 1: case 2: {
                if (auto textureRect = obj.get<std::string>("frame")) {
                    rect = CCRectFromString(textureRect.unwrap().c_str());
                }
                if (auto spriteSize = obj.get<std::string>("sourceSize")) {
                    originalSize = CCSizeFromString(spriteSize.unwrap().c_str());
                }
                if (auto spriteOffset = obj.get<std::string>("offset")) {
                    offset = CCPointFromString(spriteOffset.unwrap().c_str());
                }
                if (format == 2) {
                    if (auto textureRotated = obj.get<bool>("rotated")) {
                        rotated = textureRotated.unwrap();
                    }
                }
            }
            case 3: {
                if (auto textureRect = obj.get<std::string>("textureRect")) {
                    rect.origin = CCRectFromString(textureRect.unwrap().c_str()).origin;
                }
                if (auto spriteSize = obj.get<std::string>("spriteSize")) {
                    rect.size = CCSizeFromString(spriteSize.unwrap().c_str());
                }
                if (auto spriteOffset = obj.get<std::string>("spriteOffset")) {
                    offset = CCPointFromString(spriteOffset.unwrap().c_str());
                }
                if (auto spriteSourceSize = obj.get<std::string>("spriteSourceSize")) {
                    originalSize = CCSizeFromString(spriteSourceSize.unwrap().c_str());
                }
                if (auto textureRotated = obj.get<bool>("textureRotated")) {
                    rotated = textureRotated.unwrap();
                }
            }
        }

        auto absX = std::abs(offset.x) * 2.0f;
        if (originalSize.width - rect.size.width < absX) originalSize.width = rect.size.width + absX;
        auto absY = std::abs(offset.y) * 2.0f;
        if (originalSize.height - rect.size.height < absY) originalSize.height = rect.size.height + absY;

        frame->initWithTexture(texture, rect, rotated, offset, originalSize);
    }

    return Ok(std::move(frames));
}

CCTexture2D* Load::addFrames(const ImageResult& image, std::vector<std::string>& frameNames, std::string_view target) {
    if (auto texture = image.texture.data) {
        initTexture(texture, image.data.data(), image.width, image.height);
        Get::TextureCache()->m_pTextures->setObject(texture, image.name);
    }

    frameNames.clear();
    if (auto frames = image.frames.data) {
        auto spriteFrameCache = Get::SpriteFrameCache();
        if (target.empty()) {
            frameNames.reserve(frames->count());
            for (auto [frameName, frame] : CCDictionaryExt<const char*, CCSpriteFrame*>(frames)) {
                spriteFrameCache->addSpriteFrame(frame, frameName);
                frameNames.emplace_back(frameName);
            }
        }
        else {
            frameNames.reserve(1);
            for (auto [frameName, frame] : CCDictionaryExt<std::string_view, CCSpriteFrame*>(frames)) {
                if (frameName == target) {
                    spriteFrameCache->addSpriteFrame(frame, frameName.data());
                    frameNames.emplace_back(frameName);
                }
            }
        }
    }

    return image.texture;
}
