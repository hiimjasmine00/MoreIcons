#include "../BasePopup.hpp"

class LoadEditorPopup : public BasePopup {
protected:
    geode::Function<void(const std::filesystem::path&, std::string_view)> m_callback;

    bool init(IconType type, geode::Function<void(const std::filesystem::path&, std::string_view)> callback);
    void onEntry(cocos2d::CCObject* sender);
public:
    static LoadEditorPopup* create(IconType type, geode::Function<void(const std::filesystem::path&, std::string_view)> callback);
};
