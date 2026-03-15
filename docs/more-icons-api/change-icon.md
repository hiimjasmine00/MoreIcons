---
title: Changing the Current Icon
order: 2
---

# Changing the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get your custom icon
IconInfo* myCustomIcon = more_icons::getIcon("my_custom_icon", IconType::Cube);

// Change the current icon
IconInfo* oldIcon = more_icons::setIcon(myCustomIcon, IconType::Cube);

// Change the current icon (Separate Dual Icons)
IconInfo* oldDualIcon = more_icons::setIcon(myCustomIcon, IconType::Cube, true);
```
