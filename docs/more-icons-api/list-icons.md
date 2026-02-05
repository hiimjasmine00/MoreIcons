---
title: Listing the Loaded Icons
order: 3
---

# Listing the Loaded Icons
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// List all loaded icons
std::map<std::string, std::vector<IconInfo>>& icons = more_icons::getAllIcons();
for (std::pair<const std::string, std::vector<IconInfo>>& [key, iconList] : icons) {
    for (IconInfo& icon : iconList) {
        // Do something with the icon
    }
}

// List all loaded icons of a specific type
std::vector<IconInfo>* cubeIcons = more_icons::getIcons(IconType::Cube);
if (cubeIcons) {
    for (IconInfo& icon : *cubeIcons) {
        // Do something with the icon
    }
}
```
