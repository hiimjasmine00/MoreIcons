#include <cocos2d.h>
#include <Geode/Enums.hpp>
#include <Geode/Result.hpp>

template <class T>
struct Autorelease {
    T* data;

    Autorelease() : data(nullptr) {}
    Autorelease(T* data) : data(data) {}
    Autorelease(const Autorelease& other) : data(other.data) {
        CC_SAFE_RETAIN(data);
    }
    Autorelease(Autorelease&& other) : data(other.data) {
        other.data = nullptr;
    }

    ~Autorelease() {
        CC_SAFE_RELEASE(data);
    }

    Autorelease& operator=(const Autorelease& other) {
        if (this != &other) {
            CC_SAFE_RELEASE(data);
            data = other.data;
            CC_SAFE_RETAIN(data);
        }
        return *this;
    }
    Autorelease& operator=(Autorelease&& other) {
        if (this != &other) {
            data = other.data;
            other.data = nullptr;
        }
        return *this;
    }

    operator T*() const {
        return data;
    }
    T* operator->() const {
        return data;
    }
};

struct ImageResult {
    std::string name;
    std::vector<uint8_t> data;
    Autorelease<cocos2d::CCTexture2D> texture;
    Autorelease<cocos2d::CCDictionary> frames;
    uint32_t width;
    uint32_t height;
};

class Load {
public:
    static std::string getFrameName(std::string_view frameName, std::string_view name, IconType type);
    static geode::Result<std::vector<uint8_t>> readBinary(const std::filesystem::path& path);
    static geode::Result<Autorelease<cocos2d::CCTexture2D>> createTexture(const std::filesystem::path& path);
    static Autorelease<cocos2d::CCTexture2D> createTexture(const uint8_t* data, uint32_t width, uint32_t height);
    static void initTexture(cocos2d::CCTexture2D* texture, const uint8_t* data, uint32_t width, uint32_t height, bool premultiplyAlpha = true);
    static geode::Result<ImageResult> createFrames(
        const std::filesystem::path& png, const std::filesystem::path& plist, std::string_view name, IconType type, bool premultiplyAlpha = true
    );
    static geode::Result<Autorelease<cocos2d::CCDictionary>> createFrames(
        const std::filesystem::path& path, cocos2d::CCTexture2D* texture, std::string_view name, IconType type, bool fixNames = true
    );
    static cocos2d::CCTexture2D* addFrames(const ImageResult& image, std::vector<std::string>& frameNames, std::string_view suffix = {});
};
