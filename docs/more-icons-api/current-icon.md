---
title: Getting the Current Icon
order: 1
---

# Getting the Current Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the current icon
IconInfo* icon = more_icons::activeIcon(IconType::Cube);

// Get the current icon (Separate Dual Icons)
IconInfo* dualIcon = more_icons::activeIcon(IconType::Cube, true);
```
