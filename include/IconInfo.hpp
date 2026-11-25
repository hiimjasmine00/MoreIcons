#pragma once
#include "Macros.hpp"
#include <asp/fs/Path.hpp>
#include <Geode/Enums.hpp>
#if defined(MORE_ICONS_EVENTS) || defined(GEODE_DEFINE_EVENT_EXPORTS)
#include <Geode/loader/Dispatch.hpp>
#endif
#include <matjson.hpp>

#define MI_EXPAND(suffix) std::string("hiimjustin000.more_icons/" suffix, sizeof("hiimjustin000.more_icons/" suffix) - 1)
#if defined(MORE_ICONS_EVENTS)
#define MI_EXPORT(Class, ret, name, params, quals, args, def) { \
    using FnType = GEODE_EVENT_EXPORT_GET_MEMBERFNTYPE(Class, ret, params, quals, GEODE_EVENT_EXPORT_UNIQ_NAME(name)); \
    static auto storage = geode::geode_internal::callEventExportListener<FnType>(MI_EXPAND(#Class "::" #name)); \
    if (!storage) return def; \
    return storage args; \
}
#define MI_EXPORT_REF(Class, ret, name, params, quals, args) { \
    using FnType = GEODE_EVENT_EXPORT_GET_MEMBERFNTYPE(Class, ret, params, quals, GEODE_EVENT_EXPORT_UNIQ_NAME(name)); \
    static auto storage = geode::geode_internal::callEventExportListener<FnType>(MI_EXPAND(#Class "::" #name)); \
    if (!storage) { \
        static std::remove_reference_t<geode::utils::function::Return<decltype(&Class::name)>> dummy; \
        return dummy; \
    } \
    return storage args; \
}
#elif defined(GEODE_DEFINE_EVENT_EXPORTS)
#define MI_EXPORT(Class, ret, name, params, quals, args, def) ; \
    static inline bool GEODE_EVENT_EXPORT_UNIQ_NAME(_geode_event_export_reg_) \
    = geode::geode_internal::getEventExportListener(static_cast< \
        GEODE_EVENT_EXPORT_GET_MEMBERFNTYPE(Class, ret, params, quals, GEODE_EVENT_EXPORT_UNIQ_NAME(name)) \
    >(&Class::name), MI_EXPAND(#Class "::" #name));
#define MI_EXPORT_REF(Class, ret, name, params, quals, args) ; \
    static inline bool GEODE_EVENT_EXPORT_UNIQ_NAME(_geode_event_export_reg_) \
    = geode::geode_internal::getEventExportListener(static_cast< \
        GEODE_EVENT_EXPORT_GET_MEMBERFNTYPE(Class, ret, params, quals, GEODE_EVENT_EXPORT_UNIQ_NAME(name)) \
    >(&Class::name), MI_EXPAND(#Class "::" #name));
#else
#define MI_EXPORT(Class, ret, name, params, quals, args, def)
#define MI_EXPORT_REF(Class, ret, name, params, quals, args)
#endif

class IconInfoImpl;

/// A class that contains information about a custom icon.
class MORE_ICONS_DLL IconInfo {
private:
    std::shared_ptr<IconInfoImpl> m_impl;

    friend class IconInfoImpl;
    friend IconInfo* addIcon(
        std::string name, std::string shortName, IconType type, asp::fs::Path png, asp::fs::Path plist,
        asp::fs::Path json, asp::fs::Path icon, int quality, std::string packID, std::string packName,
        int specialID, matjson::Value specialInfo, int fireCount, bool vanilla, bool zipped
    );

    IconInfo(std::shared_ptr<IconInfoImpl> impl);
public:
    IconInfo() = delete;
    #ifdef MORE_ICONS_EVENTS
    IconInfo(const IconInfo&) = default;
    IconInfo(IconInfo&&) noexcept = default;
    IconInfo& operator=(const IconInfo&) = default;
    IconInfo& operator=(IconInfo&&) noexcept = default;
    ~IconInfo() = default;
    #else
    IconInfo(const IconInfo&);
    IconInfo(IconInfo&&) noexcept;
    IconInfo& operator=(const IconInfo&);
    IconInfo& operator=(IconInfo&&) noexcept;
    ~IconInfo();
    #endif

    const std::string& getName() const MI_EXPORT_REF(IconInfo, const std::string&, getName, (), const, (this));
    const std::string& getShortName() const MI_EXPORT_REF(IconInfo, const std::string&, getShortName, (), const, (this));
    const asp::fs::Path& getTexture() const MI_EXPORT_REF(IconInfo, const asp::fs::Path&, getTexture, (), const, (this));
    std::vector<std::string> getAllTextures() const MI_EXPORT(IconInfo, std::vector<std::string>, getAllTextures, (), const, (this), {});
    const asp::fs::Path& getSheet() const MI_EXPORT_REF(IconInfo, const asp::fs::Path&, getSheet, (), const, (this));
    const asp::fs::Path& getJSON() const MI_EXPORT_REF(IconInfo, const asp::fs::Path&, getJSON, (), const, (this));
    const asp::fs::Path& getIcon() const MI_EXPORT_REF(IconInfo, const asp::fs::Path&, getIcon, (), const, (this));
    const std::vector<std::string>& getFrameNames() const MI_EXPORT_REF(IconInfo, const std::vector<std::string>&, getFrameNames, (), const, (this));
    const std::string& getPackName() const MI_EXPORT_REF(IconInfo, const std::string&, getPackName, (), const, (this));
    const std::string& getPackID() const MI_EXPORT_REF(IconInfo, const std::string&, getPackID, (), const, (this));
    IconType getType() const MI_EXPORT(IconInfo, IconType, getType, (), const, (this), IconType::Cube);
    int getQuality() const MI_EXPORT(IconInfo, int, getQuality, (), const, (this), 0);
    int getSpecialID() const MI_EXPORT(IconInfo, int, getSpecialID, (), const, (this), 0);
    const matjson::Value& getSpecialInfo() const MI_EXPORT_REF(IconInfo, const matjson::Value&, getSpecialInfo, (), const, (this));
    int getFireCount() const MI_EXPORT(IconInfo, int, getFireCount, (), const, (this), 0);
    bool inTexturePack() const MI_EXPORT(IconInfo, bool, inTexturePack, (), const, (this), false);
    bool isVanilla() const MI_EXPORT(IconInfo, bool, isVanilla, (), const, (this), false);
    bool isZipped() const MI_EXPORT(IconInfo, bool, isZipped, (), const, (this), false);

    void setName(std::string name) MI_EXPORT(IconInfo, void, setName, (std::string), , (this, std::move(name)),);
    void setShortName(std::string shortName) MI_EXPORT(IconInfo, void, setShortName, (std::string), , (this, std::move(shortName)),);
    void setTexture(asp::fs::Path texture) MI_EXPORT(IconInfo, void, setTexture, (asp::fs::Path), , (this, std::move(texture)),);
    void setSheet(asp::fs::Path sheet) MI_EXPORT(IconInfo, void, setSheet, (asp::fs::Path), , (this, std::move(sheet)),);
    void setJSON(asp::fs::Path json) MI_EXPORT(IconInfo, void, setJSON, (asp::fs::Path), , (this, std::move(json)),);
    void setIcon(asp::fs::Path icon) MI_EXPORT(IconInfo, void, setIcon, (asp::fs::Path), , (this, std::move(icon)),);
    void setFrameNames(std::vector<std::string> frameNames) MI_EXPORT(IconInfo, void, setFrameNames, (std::vector<std::string>), , (this, std::move(frameNames)),);
    void setPackName(std::string packName) MI_EXPORT(IconInfo, void, setPackName, (std::string), , (this, std::move(packName)),);
    void setPackID(std::string packID) MI_EXPORT(IconInfo, void, setPackID, (std::string), , (this, std::move(packID)),);
    void setType(IconType type) MI_EXPORT(IconInfo, void, setType, (IconType), , (this, type),);
    void setQuality(int quality) MI_EXPORT(IconInfo, void, setQuality, (int), , (this, quality),);
   	void setSpecialID(int specialID) MI_EXPORT(IconInfo, void, setSpecialID, (int), , (this, specialID),);
   	void setSpecialInfo(matjson::Value specialInfo) MI_EXPORT(IconInfo, void, setSpecialInfo, (matjson::Value), , (this, std::move(specialInfo)),);
   	void setFireCount(int fireCount) MI_EXPORT(IconInfo, void, setFireCount, (int), , (this, fireCount),);
   	void setVanilla(bool vanilla) MI_EXPORT(IconInfo, void, setVanilla, (bool), , (this, vanilla),);
   	void setZipped(bool zipped) MI_EXPORT(IconInfo, void, setZipped, (bool), , (this, zipped),);

    bool equals(std::string_view name, IconType type) const
        MI_EXPORT(IconInfo, bool, equals, (std::string_view, IconType), const, (this, name, type), false);
    bool operator==(const IconInfo& other) const {
        return equals(other.getName(), other.getType());
    }

    std::strong_ordering compare(std::string_view packID, std::string_view shortName, IconType type) const
        MI_EXPORT(IconInfo, std::strong_ordering, compare, (std::string_view, std::string_view, IconType), const, (this, packID, shortName, type), std::strong_ordering::less);
    std::strong_ordering operator<=>(const IconInfo& other) const {
        return compare(other.getPackID(), other.getShortName(), other.getType());
    }
};

#undef MI_EXPAND
#undef MI_EXPORT
#undef MI_EXPORT_REF
