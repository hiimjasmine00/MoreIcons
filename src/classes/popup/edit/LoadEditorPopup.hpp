#include "../BasePopup.hpp"
#include <std23/move_only_function.h>

class LoadEditorPopup : public BasePopup {
protected:
    std23::move_only_function<void(const std::filesystem::path&, std::string_view)> m_callback;

    bool init(IconType type, std23::move_only_function<void(const std::filesystem::path&, std::string_view)> callback);
public:
    static LoadEditorPopup* create(IconType type, std23::move_only_function<void(const std::filesystem::path&, std::string_view)> callback);
};
