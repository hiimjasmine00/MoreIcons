#include "../BasePopup.hpp"

class LoadEditorPopup : public BasePopup {
protected:
    std::vector<asp::fs::Path> m_entries;
    geode::Function<geode::Result<>(const asp::fs::Path&)> m_callback;

    bool init(IconType type, geode::Function<geode::Result<>(const asp::fs::Path&)> callback);
    void onEntry(cocos2d::CCObject* sender);
public:
    static LoadEditorPopup* create(IconType type, geode::Function<geode::Result<>(const asp::fs::Path&)> callback);
};
