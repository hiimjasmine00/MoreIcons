# More Icons Documentation
This website hosts the documentation for the More Icons mod for Geometry Dash.

At this moment, it only hosts API documentation, but user documentation will be considered in the future.

## Adding the API
To use the More Icons API, you must add it as a dependency in your `mod.json` file.

```json
{
    "dependencies": {
        "hiimjustin000.more_icons": {
            "version": ">=v1.16.0",
            "importance": "required"
        }
    }
}
```

Importance can be either `required`, `recommended`, or `suggested`, depending on how critical the dependency is to your mod.

For recommended and suggested dependencies, make sure to add this definition before including the `MoreIconsV2.hpp` header:

```cpp
#define MORE_ICONS_EVENTS
#include <hiimjustin000.more_icons/include/MoreIconsV2.hpp>
```

## Supported [Icon Types](https://github.com/geode-sdk/bindings/blob/c949aeae063175eca528b33cf8f5b573935b0079/bindings/include/Geode/Enums.hpp#L911)
- `IconType::Cube` (0)
- `IconType::Ship` (1)
- `IconType::Ball` (2)
- `IconType::Ufo` (3)
- `IconType::Wave` (4)
- `IconType::Robot` (5)
- `IconType::Spider` (6)
- `IconType::Swing` (7)
- `IconType::Jetpack` (8)
- `IconType::Special` (99)
