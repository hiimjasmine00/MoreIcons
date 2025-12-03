---
title: Changing the Current Icon
order: 2
---

# Changing the Current Icon
```cpp
// Include the MoreIconsV2.hpp header
#include <hiimjustin000.more_icons/include/MoreIconsV2.hpp>

// Change the current icon
std::string oldIcon = more_icons::setIcon("my_custom_icon", IconType::Cube);

// Change the current icon (Separate Dual Icons)
std::string oldDualIcon = more_icons::setIcon("my_custom_icon", IconType::Cube, true);

// Change the current icon (Internal behavior)
if (geode::Mod* MI = more_icons::get()) {
    std::string oldIcon = MI->setSavedValue(more_icons::saveKey(IconType::Cube), "my_custom_icon");
    std::string oldDualIcon = MI->setSavedValue(more_icons::saveKey(IconType::Cube, true), "my_custom_icon");
}
```
