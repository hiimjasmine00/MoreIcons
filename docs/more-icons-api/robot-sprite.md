---
title: Changing a GJRobotSprite's Icon
order: 9
---

# Changing a GJRobotSprite's Icon
```cpp
// Include the MoreIconsV2.hpp header
#include <hiimjustin000.more_icons/include/MoreIconsV2.hpp>

// Change the icon of a GJRobotSprite to the current icon (Determined icon type)
more_icons::updateRobotSprite(robot);

// Change the icon of a GJRobotSprite to the current icon (Determined icon type, Separate Dual Icons)
more_icons::updateRobotSprite(robot, true);

// Change the icon of a GJRobotSprite to a custom icon (Determined icon type)
more_icons::updateRobotSprite(robot, "my_custom_icon");

// Change the icon of a GJRobotSprite to the current icon
more_icons::updateRobotSprite(robot, IconType::Robot);

// Change the icon of a GJRobotSprite to the current icon (Separate Dual Icons)
more_icons::updateRobotSprite(robot, IconType::Robot, true);

// Change the icon of a GJRobotSprite to a custom icon
more_icons::updateRobotSprite(robot, "my_custom_icon", IconType::Robot);
```
