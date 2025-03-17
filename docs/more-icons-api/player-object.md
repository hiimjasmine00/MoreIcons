---
title: Changing a PlayerObject's Icon
order: 10
---

# Changing a PlayerObject's Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Change the icon of a PlayerObject to the current icon (Determined icon type)
MoreIcons::updatePlayerObject(player);

// Change the icon of a PlayerObject to the current icon (Determined icon type, Separate Dual Icons)
MoreIcons::updatePlayerObject(player, true);

// Change the icon of a PlayerObject to a custom icon (Determined icon type)
MoreIcons::updatePlayerObject(player, "my_custom_icon");

// Change the icon of a PlayerObject to the current icon
MoreIcons::updatePlayerObject(player, IconType::Cube);

// Change the icon of a PlayerObject to the current icon (Separate Dual Icons)
MoreIcons::updatePlayerObject(player, IconType::Cube, true);

// Change the icon of a PlayerObject to a custom icon
MoreIcons::updatePlayerObject(player, "my_custom_icon", IconType::Cube);
```
