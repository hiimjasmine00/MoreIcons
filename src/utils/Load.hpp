#include <Geode/utils/cocos.hpp>

struct ImageResult {
    std::string name;
    std::vector<uint8_t> data;
    geode::Ref<cocos2d::CCTexture2D> texture;
    std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>> frames;
    uint32_t width;
    uint32_t height;

    ImageResult(
        std::string&& name,
        std::vector<uint8_t>&& data,
        geode::Ref<cocos2d::CCTexture2D>&& texture,
        std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>&& frames,
        uint32_t width,
        uint32_t height
    ) : name(std::move(name)), data(std::move(data)), texture(std::move(texture)), frames(std::move(frames)), width(width), height(height) {}

    ImageResult(const ImageResult& result) :
        name(result.name),
        data(result.data),
        texture(result.texture),
        frames(result.frames),
        width(result.width),
        height(result.height) {}
    ImageResult(ImageResult&& result) noexcept :
        name(std::move(result.name)),
        data(std::move(result.data)),
        texture(std::move(result.texture)),
        frames(std::move(result.frames)),
        width(result.width),
        height(result.height) {}

    ImageResult& operator=(const ImageResult& result) {
        name = result.name;
        data = result.data;
        texture = result.texture;
        frames = result.frames;
        width = result.width;
        height = result.height;
        return *this;
    }
    ImageResult& operator=(ImageResult&& result) noexcept {
        name = std::move(result.name);
        data = std::move(result.data);
        texture = std::move(result.texture);
        frames = std::move(result.frames);
        width = result.width;
        height = result.height;
        return *this;
    }
};

namespace Load {
    void fixFrameName(std::string& frameName, std::string_view name, IconType type);
    geode::Result<std::vector<uint8_t>> readBinary(const std::filesystem::path& path);
    geode::Result<matjson::Value> readPlist(const std::filesystem::path& path);
    bool doesExist(const std::filesystem::path& path);
    geode::Result<cocos2d::CCTexture2D*> createTexture(const std::filesystem::path& path);
    cocos2d::CCTexture2D* createTexture(const uint8_t* data, uint32_t width, uint32_t height);
    void initTexture(cocos2d::CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha = true);
    geode::Result<ImageResult> createFrames(
        const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name, IconType type,
        std::string_view target = {}, bool premultiply = true
    );
    geode::Result<std::unordered_map<std::string, geode::Ref<cocos2d::CCSpriteFrame>>> createFrames(
        const std::filesystem::path& path, cocos2d::CCTexture2D* texture, std::string_view name, IconType type,
        std::string_view target = {}, bool fixNames = true
    );
    cocos2d::CCTexture2D* addFrames(ImageResult& image, std::vector<std::string>& frameNames);
}
