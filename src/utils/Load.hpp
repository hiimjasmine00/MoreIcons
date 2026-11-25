#include <Geode/utils/cocos.hpp>
#include <Geode/utils/StringMap.hpp>

struct RGBAImage {
    std::vector<uint8_t> data;
    uint32_t width;
    uint32_t height;

    RGBAImage(std::vector<uint8_t>&& data, uint32_t width, uint32_t height);

    RGBAImage(const RGBAImage&) = delete;
    RGBAImage(RGBAImage&&) noexcept;

    RGBAImage& operator=(const RGBAImage&) = delete;
    RGBAImage& operator=(RGBAImage&&) noexcept;

    ~RGBAImage();
};

struct ImageResult {
    std::string name;
    geode::Ref<cocos2d::CCTexture2D> texture;
    geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>> frames;
    RGBAImage image;

    ImageResult(
        std::string&& name,
        geode::Ref<cocos2d::CCTexture2D>&& texture,
        geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>&& frames,
        RGBAImage&& image
    );

    ImageResult(const ImageResult&) = delete;
    ImageResult(ImageResult&&) noexcept;

    ImageResult& operator=(const ImageResult&) = delete;
    ImageResult& operator=(ImageResult&&) noexcept;

    ~ImageResult();
};

namespace Load {
    void fixFrameName(std::string& frameName, std::string_view name, IconType type);
    geode::Result<std::vector<uint8_t>> readBinary(asp::fs::PathView path);
    geode::Result<RGBAImage> readPNG(asp::fs::PathView path, bool premultiplyAlpha);
    geode::Result<matjson::Value> readPlist(asp::fs::PathView path);
    bool doesExist(asp::fs::PathView path);
    geode::Result<cocos2d::CCTexture2D*> createTexture(asp::fs::PathView path, bool premultiplyAlpha = false);
    cocos2d::CCTexture2D* createTexture(const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha = false);
    void initTexture(cocos2d::CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha = false);
    void initTexture(cocos2d::CCTexture2D* texture, const RGBAImage& image, bool premultiplyAlpha = false);
    void initTexture(const ImageResult& image, bool premultiplyAlpha = false);
    geode::Result<ImageResult> createFrames(
        asp::fs::PathView png, asp::fs::PathView plist, std::string_view name, IconType type,
        std::string_view target = {}, bool premultiply = false
    );
    geode::Result<geode::utils::StringMap<geode::Ref<cocos2d::CCSpriteFrame>>> createFrames(
        asp::fs::PathView path, cocos2d::CCTexture2D* texture, std::string_view name, IconType type,
        std::string_view target = {}, bool fixNames = true
    );
    cocos2d::CCTexture2D* addFrames(ImageResult& image, std::vector<std::string>& frameNames);
}
