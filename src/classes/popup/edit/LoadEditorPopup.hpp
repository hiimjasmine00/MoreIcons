#include "../BasePopup.hpp"

class LoadEditorPopup : public BasePopup {
protected:
    geode::Function<geode::Result<>(const std::filesystem::path&)> m_callback;

    bool init(IconType type, geode::Function<geode::Result<>(const std::filesystem::path&)> callback);
    void onEntry(cocos2d::CCObject* sender);
public:
    static LoadEditorPopup* create(IconType type, geode::Function<geode::Result<>(const std::filesystem::path&)> callback);
};
