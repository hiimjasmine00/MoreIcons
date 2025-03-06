---
title: Getting the Current Icon
order: 1
---

# Getting the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the current icon
auto icon = MoreIcons::activeForType(IconType::Cube);

// Get the current icon (Separate Dual Icons)
auto dualIcon = MoreIcons::activeForType(IconType::Cube, true);

// Get the current icon (Internal behavior)
if (auto MI = MoreIcons::get()) {
    auto icon = MI->getSavedValue(MoreIcons::savedForType(IconType::Cube), "");
    auto dualIcon = MI->getSavedValue(MoreIcons::savedForType(IconType::Cube, true), "");
}
```
