#pragma once
#include <Geode/Enums.hpp>
#include <string>
#include <vector>

/// A struct that contains information about a custom icon.
struct IconInfo {
    std::string name;
    std::vector<std::string> textures;
    std::vector<std::string> frameNames;
    std::string sheetName;
    std::string packName;
    std::string packID;
    IconType type;
    int trailID;
    bool blend;
    bool tint;
    bool show;
    float fade;
    float stroke;
    std::string shortName;
    bool vanilla;
    bool zipped;

    bool operator==(const IconInfo& other) const {
        return name == other.name && type == other.type;
    }
};
