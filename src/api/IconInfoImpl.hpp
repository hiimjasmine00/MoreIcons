#include <filesystem>
#include <Geode/Enums.hpp>
#include <matjson.hpp>

class IconInfoImpl {
public:
    std::string m_name;
    std::string m_shortName;
    std::string m_packID;
    std::string m_packName;
    std::filesystem::path m_texture;
    std::filesystem::path m_sheet;
    std::filesystem::path m_json;
    std::filesystem::path m_icon;
    std::vector<std::string> m_frameNames;
    matjson::Value m_specialInfo;
    IconType m_type;
    int m_quality;
    int m_specialID;
    int m_fireCount;
    bool m_vanilla;
    bool m_zipped;

    IconInfoImpl(
        std::string&& name, std::string&& shortName, IconType type, std::filesystem::path&& png, std::filesystem::path&& plist,
        std::filesystem::path&& json, std::filesystem::path&& icon, int quality, std::string&& packID, std::string&& packName,
        int specialID, matjson::Value&& specialInfo, int fireCount, bool vanilla, bool zipped
    ) : m_name(std::move(name)),
        m_shortName(std::move(shortName)),
        m_packID(std::move(packID)),
        m_packName(std::move(packName)),
        m_texture(std::move(png)),
        m_sheet(std::move(plist)),
        m_json(std::move(json)),
        m_icon(std::move(icon)),
        m_type(type),
        m_quality(quality),
        m_specialID(specialID),
        m_specialInfo(std::move(specialInfo)),
        m_fireCount(fireCount),
        m_vanilla(vanilla),
        m_zipped(zipped) {}
};
