---
title: Changing a SimplePlayer's Icon
order: 8
---

# Changing a SimplePlayer's Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Change the icon of a SimplePlayer to the current icon
more_icons::updateSimplePlayer(player, IconType::Cube);

// Change the icon of a SimplePlayer to the current icon (Separate Dual Icons)
more_icons::updateSimplePlayer(player, IconType::Cube, true);

// Change the icon of a SimplePlayer to a custom icon
more_icons::updateSimplePlayer(player, "my_custom_icon", IconType::Cube);
```
