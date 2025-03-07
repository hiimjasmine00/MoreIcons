---
title: Changing the Current Icon
order: 2
---

# Changing the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Change the current icon
auto oldIcon = MoreIcons::setIcon("my_custom_icon", IconType::Cube);

// Change the current icon (Separate Dual Icons)
auto oldDualIcon = MoreIcons::setIcon("my_custom_icon", IconType::Cube, true);

// Change the current icon (Internal behavior)
if (auto MI = MoreIcons::get()) {
    auto oldIcon = MI->setSavedValue(MoreIcons::savedForType(IconType::Cube), "my_custom_icon");
    auto oldDualIcon = MI->setSavedValue(MoreIcons::savedForType(IconType::Cube, true), "my_custom_icon");
}
```
