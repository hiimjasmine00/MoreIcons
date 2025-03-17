---
title: Listing the Loaded Icons
order: 3
---

# Listing the Loaded Icons
```cpp
// Include the MoreIcons.hpp header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// List all loaded icons
for (std::vector<MoreIcons::IconInfo*> icon : MoreIcons::getIcons()) {
    // Do something with the icon
}

// List all loaded icons of a specific type
for (std::vector<MoreIcons::IconInfo*> icon : MoreIcons::getIcons(IconType::Cube)) {
    // Do something with the icon
}
```
