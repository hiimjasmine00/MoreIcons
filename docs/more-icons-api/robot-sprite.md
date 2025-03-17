---
title: Changing a GJRobotSprite's Icon
order: 9
---

# Changing a GJRobotSprite's Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Change the icon of a GJRobotSprite to the current icon (Determined icon type)
MoreIcons::updateRobotSprite(robot);

// Change the icon of a GJRobotSprite to the current icon (Determined icon type, Separate Dual Icons)
MoreIcons::updateRobotSprite(robot, true);

// Change the icon of a GJRobotSprite to a custom icon (Determined icon type)
MoreIcons::updateRobotSprite(robot, "my_custom_icon");

// Change the icon of a GJRobotSprite to the current icon
MoreIcons::updateRobotSprite(robot, IconType::Robot);

// Change the icon of a GJRobotSprite to the current icon (Separate Dual Icons)
MoreIcons::updateRobotSprite(robot, IconType::Robot, true);

// Change the icon of a GJRobotSprite to a custom icon
MoreIcons::updateRobotSprite(robot, "my_custom_icon", IconType::Robot);
```
