---
title: Loading a Custom Icon
order: 6
---

# Loading a Custom Icon
```cpp
// Include the MoreIconsV2.hpp header
#include <hiimjustin000.more_icons/include/MoreIconsV2.hpp>

// Get the next request ID (Preferably one per object)
int requestID = GameManager::get()->getIconRequestID();

// Load a custom icon into the texture cache
cocos2d::CCTexture2D* texture = more_icons::loadIcon("my_custom_icon", IconType::Cube, requestID);
```
