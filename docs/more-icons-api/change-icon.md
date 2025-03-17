---
title: Changing the Current Icon
order: 2
---

# Changing the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Change the current icon
std::string oldIcon = MoreIcons::setIcon("my_custom_icon", IconType::Cube);

// Change the current icon (Separate Dual Icons)
std::string oldDualIcon = MoreIcons::setIcon("my_custom_icon", IconType::Cube, true);

// Change the current icon (Internal behavior)
if (geode::Mod* MI = MoreIcons::get()) {
    std::string oldIcon = MI->setSavedValue(MoreIcons::saveKey(IconType::Cube), "my_custom_icon");
    std::string oldDualIcon = MI->setSavedValue(MoreIcons::saveKey(IconType::Cube, true), "my_custom_icon");
}
```
