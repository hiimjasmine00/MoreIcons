#include <pugixml.hpp>
#include "Load.hpp"
#include "Get.hpp"
#ifdef GEODE_IS_ANDROID
#include <Geode/cocos/support/zip_support/unzip.h>
#endif
#include <Geode/utils/file.hpp>
#include <jasmine/mod.hpp>
#include <texpack.hpp>

using namespace geode::prelude;
using namespace jasmine::mod;
using namespace std::string_view_literals;

void replaceOrErase(std::string& str, size_t offset, std::string_view name) {
    if (name.empty()) {
        str.erase(0, str.size() - offset);
        str.erase(str.size() - 4);
    }
    else {
        str.replace(0, str.size() - offset, fmt::format("{}"_spr, name));
    }
}

std::string Load::getFrameName(std::string&& frameName, std::string_view name, IconType type) {
    if (frameName.size() < 8 || !frameName.ends_with(".png"sv)) return std::move(frameName);

    if (type == IconType::DeathEffect) {
        replaceOrErase(frameName, 8, name);
    }

    if (!frameName.ends_with("_001.png"sv)) return std::move(frameName);

    std::string_view suffix;
    auto isRobot = type == IconType::Robot || type == IconType::Spider;
    auto end = std::string_view(frameName.data(), frameName.size() - 8);

    if (end.ends_with("_2"sv)) {
        if (isRobot) {
            end.remove_suffix(2);
            if (end.ends_with("_01"sv) || end.ends_with("_02"sv) || end.ends_with("_03"sv) || end.ends_with("_04"sv)) {
                replaceOrErase(frameName, 13, name);
            }
        }
        else {
            replaceOrErase(frameName, 10, name);
        }
    }
    else if (type == IconType::Ufo && end.ends_with("_3"sv)) {
        replaceOrErase(frameName, 10, name);
    }
    else if (end.ends_with("_extra"sv)) {
        if (isRobot) {
            end.remove_suffix(6);
            if (end.ends_with("_01"sv)) {
                replaceOrErase(frameName, 17, name);
            }
        }
        else {
            replaceOrErase(frameName, 14, name);
        }
    }
    else if (end.ends_with("_glow"sv)) {
        if (isRobot) {
            if (end.ends_with("_01"sv) || end.ends_with("_02"sv) || end.ends_with("_03"sv) || end.ends_with("_04"sv)) {
                replaceOrErase(frameName, 16, name);
            }
        }
        else {
            replaceOrErase(frameName, 13, name);
        }
    }
    else {
        if (isRobot) {
            if (end.ends_with("_01"sv) || end.ends_with("_02"sv) || end.ends_with("_03"sv) || end.ends_with("_04"sv)) {
                replaceOrErase(frameName, 11, name);
            }
        }
        else {
            replaceOrErase(frameName, 8, name);
        }
    }

    return std::move(frameName);
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

thread_local ApkFile* apkFile = [] {
    ApkFile* apkFile = new ApkFile();

    auto apkPath = getApkPath();
    auto zipFile = unzOpen(apkPath);
    if (!zipFile) {
        log::error("Failed to open APK file at {}", apkPath);
        return apkFile;
    }

    apkFile->zipFile = zipFile;

    unz_file_info64 fileInfo;
    char filename[257];

    auto& fileList = apkFile->fileList;
    auto res = unzGoToFirstFile64(zipFile, &fileInfo, filename, 256);
    for (; res == 0; res = unzGoToNextFile64(zipFile, &fileInfo, filename, 256)) {
        if (memcmp(filename, "assets/", 7) != 0) continue;
        ApkEntry entryInfo;
        if (unzGetFilePos(zipFile, &entryInfo.pos) != 0) continue;
        entryInfo.size = fileInfo.uncompressed_size;
        fileList[filename] = entryInfo;
    }

    if (res != 0) {
        if (filename[0] == '\0') log::error("Failed to iterate over APK file entries");
        else log::error("APK file iteration prematurely aborted after {}", filename);
    }

    return apkFile;
}();
#endif

Result<std::vector<uint8_t>> Load::readBinary(const std::filesystem::path& path) {
    #ifdef GEODE_IS_ANDROID
    auto& str = path.native();
    if (str.starts_with("assets/"sv)) {
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
    if (str.starts_with("assets/"sv)) return apkFile->fileList.contains(str);
    #endif
    std::error_code code;
    return std::filesystem::exists(path, code);
}

Result<CCTexture2D*> Load::createTexture(const std::filesystem::path& path) {
    GEODE_UNWRAP_INTO(auto data, readBinary(path).mapErr([](std::string err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data).mapErr([](std::string err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    return Ok(createTexture(image.data.data(), image.width, image.height));
}

CCTexture2D* Load::createTexture(const uint8_t* data, uint32_t width, uint32_t height) {
    auto texture = new CCTexture2D();
    initTexture(texture, data, width, height, false);
    texture->autorelease();
    return texture;
}

void Load::initTexture(CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha) {
    texture->initWithData(data, kCCTexture2DPixelFormat_RGBA8888, width, height, { (float)width, (float)height });
    texture->m_bHasPremultipliedAlpha = premultiplyAlpha;
}

template <class T>
inline Ref<T> makeRef(T* ptr) {
    return std::move(*reinterpret_cast<Ref<T>*>(&ptr));
}

Result<ImageResult> Load::createFrames(
    const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name, IconType type,
    std::string_view target, bool premultiply
) {
    GEODE_UNWRAP_INTO(auto data, readBinary(png).mapErr([](std::string err) {
        return fmt::format("Failed to read image: {}", err);
    }));

    GEODE_UNWRAP_INTO(auto image, texpack::fromPNG(data, premultiply).mapErr([](std::string err) {
        return fmt::format("Failed to parse image: {}", err);
    }));

    auto texture = makeRef(new CCTexture2D());
    GEODE_UNWRAP_INTO(auto frames, createFrames(plist, texture, name, type, target, !premultiply || !name.empty()).mapErr([](std::string err) {
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

Result<matjson::Value> Load::readPlist(const std::filesystem::path& path) {
    GEODE_UNWRAP_INTO(auto data, readBinary(path));

    pugi::xml_document doc;
    auto result = doc.load_buffer(data.data(), data.size());
    if (!result) return Err("Failed to parse XML: {}", result.description());

    auto root = doc.child("plist");
    if (!root) return Err("No root <plist> element found");

    auto json = parseNode(root.first_child());
    if (!json.isObject()) return Err("No root <dict> element found");

    return Ok(std::move(json));
}

Result<std::unordered_map<std::string, Ref<CCSpriteFrame>>> Load::createFrames(
    const std::filesystem::path& path, CCTexture2D* texture, std::string_view name, IconType type, std::string_view target, bool fixNames
) {
    std::unordered_map<std::string, Ref<CCSpriteFrame>> frames;
    if (path.empty()) return Ok(std::move(frames));

    GEODE_UNWRAP_INTO(const auto json, readPlist(path));

    auto framesRes = json.get("frames");
    if (!framesRes.isOk()) return Err("No frames <dict> element found");

    int format = json["metadata"]["format"].asInt().unwrapOrDefault();
    if (format < 0 || format > 3) return Err("Unsupported plist format: {}", format);

    for (auto& obj : framesRes.unwrap()) {
        if (!obj.isObject()) continue;

        auto frameName = obj.getKey().value_or(std::string());
        if (!target.empty() && frameName != target) continue;

        CCRect rect;
        CCPoint offset;
        CCSize originalSize;
        auto rotated = false;

        switch (format) {
            case 0: {
                rect.origin.x = obj["x"].asDouble().unwrapOr(0.0);
                rect.origin.y = obj["y"].asDouble().unwrapOr(0.0);
                rect.size.width = obj["width"].asDouble().unwrapOr(0.0);
                rect.size.height = obj["height"].asDouble().unwrapOr(0.0);
                offset.x = obj["offsetX"].asDouble().unwrapOr(0.0);
                offset.y = obj["offsetY"].asDouble().unwrapOr(0.0);
                originalSize.width = abs(floor(obj["originalWidth"].asDouble().unwrapOr(0.0)));
                originalSize.height = abs(floor(obj["originalHeight"].asDouble().unwrapOr(0.0)));
                break;
            }
            case 1: case 2: {
                rect = CCRectFromString(obj["frame"].asString().unwrapOrDefault().c_str());
                originalSize = CCSizeFromString(obj["sourceSize"].asString().unwrapOrDefault().c_str());
                offset = CCPointFromString(obj["offset"].asString().unwrapOrDefault().c_str());
                if (format == 2) {
                    rotated = obj["rotated"].asBool().unwrapOr(false);
                }
                break;
            }
            case 3: {
                rect.origin = CCRectFromString(obj["textureRect"].asString().unwrapOrDefault().c_str()).origin;
                rect.size = CCSizeFromString(obj["spriteSize"].asString().unwrapOrDefault().c_str());
                offset = CCPointFromString(obj["spriteOffset"].asString().unwrapOrDefault().c_str());
                originalSize = CCSizeFromString(obj["spriteSourceSize"].asString().unwrapOrDefault().c_str());
                rotated = obj["textureRotated"].asBool().unwrapOr(false);
                break;
            }
        }

        auto absX = std::abs(offset.x) * 2.0f;
        if (originalSize.width - rect.size.width < absX) originalSize.width = rect.size.width + absX;
        auto absY = std::abs(offset.y) * 2.0f;
        if (originalSize.height - rect.size.height < absY) originalSize.height = rect.size.height + absY;

        auto frame = new CCSpriteFrame();
        frame->initWithTexture(texture, rect, rotated, offset, originalSize);
        frames.emplace(fixNames ? getFrameName(std::move(frameName), name, type) : std::move(frameName), makeRef(frame));
    }

    return Ok(std::move(frames));
}

CCTexture2D* Load::addFrames(const ImageResult& image, std::vector<std::string>& frameNames) {
    if (auto texture = image.texture.data()) {
        initTexture(texture, image.data.data(), image.width, image.height);
        Get::TextureCache()->m_pTextures->setObject(texture, image.name);
    }

    frameNames.clear();
    if (!image.frames.empty()) {
        auto spriteFrameCache = Get::SpriteFrameCache();
        frameNames.reserve(image.frames.size());
        for (auto& [frameName, frame] : image.frames) {
            spriteFrameCache->addSpriteFrame(frame, frameName.c_str());
            frameNames.push_back(frameName);
        }
    }

    return image.texture;
}
