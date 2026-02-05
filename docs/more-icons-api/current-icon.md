---
title: Getting the Current Icon
order: 1
---

# Getting the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the current icon
std::string icon = more_icons::activeIcon(IconType::Cube);

// Get the current icon (Separate Dual Icons)
std::string dualIcon = more_icons::activeIcon(IconType::Cube, true);

// Get the current icon (Internal behavior)
if (geode::Mod* MI = more_icons::get()) {
    std::string icon = MI->getSavedValue(more_icons::saveKey(IconType::Cube), "");
    std::string dualIcon = MI->getSavedValue(more_icons::saveKey(IconType::Cube, true), "");
}
```
