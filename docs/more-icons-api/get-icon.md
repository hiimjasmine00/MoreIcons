---
title: Getting an Icon's Information
order: 4
---

# Getting an Icon's Information
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the current icon's information
IconInfo* icon = more_icons::getIcon(IconType::Cube);

// Get the current icon's information (Separate Dual Icons)
IconInfo* icon = more_icons::getIcon(IconType::Cube, true);

// Get the icon's information
IconInfo* icon = more_icons::getIcon("my_custom_icon", IconType::Cube);
```
