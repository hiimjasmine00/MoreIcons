---
title: Unloading a Custom Icon
order: 7
---

# Unloading a Custom Icon
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the request ID from the previous loading
int requestID = 0; // Replace with the request ID from the loading

// Unload a custom icon from the texture cache
MoreIcons::unloadIcon("my_custom_icon", IconType::Cube, requestID);

// Unload all custom icons from the request ID
MoreIcons::unloadIcons(requestID);
```
