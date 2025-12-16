#define GEODE_DEFINE_EVENT_EXPORTS
#include <IconInfo.hpp>

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
};
