# More Icons Documentation
This website hosts the documentation for the More Icons mod for Geometry Dash.\
At this moment, it only hosts API documentation, but user documentation will be considered in the future.

## Adding the API
To use the More Icons API, you must add it as a suggested dependency in your `mod.json` file.\
```json
{
    "dependencies": {
        "hiimjustin000.more_icons": {
            "version": ">=v1.12.0",
            "importance": "suggested"
        }
    }
}
```

## Supported [Icon Types](https://github.com/geode-sdk/bindings/blob/bfd6e27b9e00cee4d46e531de8599d0012e0a220/bindings/include/Geode/Enums.hpp#L764)
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
