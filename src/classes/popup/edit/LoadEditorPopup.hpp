#include <Geode/ui/Popup.hpp>
#include <std23/move_only_function.h>

class LoadEditorPopup : public geode::Popup<IconType, std23::move_only_function<void(const std::filesystem::path&)>> {
protected:
    std23::move_only_function<void(const std::filesystem::path&)> m_callback;

    bool setup(IconType type, std23::move_only_function<void(const std::filesystem::path&)> callback) override;
public:
    static LoadEditorPopup* create(IconType type, std23::move_only_function<void(const std::filesystem::path&)> callback);
};
