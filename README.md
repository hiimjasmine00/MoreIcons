# More Icons
A mod that loads custom icons.

[Windows Tutorial](https://youtu.be/Dn0S3DPuq08)\
[Android Tutorial](https://youtu.be/GJKoLUnkyBk)\
[macOS Tutorial](https://youtu.be/1sI4WJE0yqE)

## Adding Icons
To add a custom icon, you need an icon spritesheet (.plist) and an icon atlas (.png). Tools like [GDBrowser's Icon Kit](https://gdbrowser.com/iconkit) can be used to create these spritesheets and atlases, with the "Developer Mode" option enabled in the settings.

With trails, you will only need the trail image (.png). It will be automatically resized to fit the game's resolution.

The spritesheet should be in the format of a typical Geometry Dash icon spritesheet, with the primary sprite, secondary sprite, glow sprite, and an optional detail sprite. UFOs contain a fifth sprite for the dome.

The atlas should be a single image containing all the sprites in the spritesheet. Tools like [Colon's Spritesheet Splitter](https://gdcolon.com/gdsplitter) can be used to split a spritesheet into individual sprites and combine them into an atlas.

The spritesheet should have the icon's name correspond to these sprites:
- Primary: `(name)_001.png`
- Secondary: `(name)_2_001.png`
- Dome (UFO only): `(name)_3_001.png`
- Glow: `(name)_glow_001.png`
- Detail (optional): `(name)_extra_001.png`

The atlas should be named `(name).png`, and the spritesheet should be named `(name).plist`. Optionally, you can add `-hd` and `-uhd` for high-definition spritesheets and atlases, and it is much recommended to do so.

The spritesheets and atlases should be placed in `(Geometry Dash folder)/geode/config/hiimjustin000.more_icons/(gamemode)`, where `(gamemode)` is the gamemode the icon is for. The gamemodes are:
- icon
- ship
- ball
- ufo
- wave
- robot
- spider
- swing
- jetpack
- trail

This can also be done with individual images per icon piece, with the same naming conventions as above. The sprites should be placed in `(Geometry Dash folder)/geode/config/hiimjustin000.more_icons/(gamemode)/(icon name)`, where `(icon name)` is the name of the icon.

If anything goes wrong, the mod will log warnings and errors to the console, which can be checked in the icon kit with a button on the left side of the screen.

## Texture Packs
Icon texture packs can be loaded with the mod "Texture Loader" by Geode Team. Normal icon texture packs can be loaded without prior modification, and More Icons will separate the icons without replacing the original icons. However, trail properties from these packs cannot be modified, and they are instead derived from vanilla trails.

## Using Icons
To use a custom icon, you will need to go into the icon kit. In the icon kit, there is a second row of dots that you can click on to view the extra icons. Click on the dot (or scroll to it with the page arrows) to view a page of extra icons, and click on one of the icons to select it.

To deselect a custom icon, use the first row of dots to go back to the default icons, and click on one of the default icons to select it.

## More Icons API
To use the More Icons API, add the following to the `dependencies` object in your `mod.json`:
```json
{
    "dependencies": {
        "hiimjustin000.more_icons": {
            "version": ">=v1.7.0",
            "importance": "suggested"
        }
    }
}
```

Here are some examples of how to use the More Icons API:
```cpp
// Include the More Icons API header
#include <hiimjustin000.more_icons/include/MoreIcons.hpp>

// Get the player's icon
auto icon = MoreIcons::activeForType(IconType::Cube);
// Get the player's dual icon (If the player has the mod "Separate Dual Icons" by Weebify enabled)
auto dualIcon = MoreIcons::activeForType(IconType::Cube, true);

// Get the list of icons (Read-only)
auto icons = MoreIcons::vectorForType(IconType::Cube);

// Change a SimplePlayer to a custom icon
MoreIcons::updateSimplePlayer(simplePlayer, "my-icon", IconType::Cube);

// Change a GJRobotSprite to a custom icon
MoreIcons::updateRobotSprite(robotSprite, "my-icon"); // Determines the icon type
MoreIcons::updateRobotSprite(robotSprite, "my-icon", IconType::Robot);

// Change a PlayerObject to a custom icon
MoreIcons::updatePlayerObject(playerObject, "my-icon"); // Determines the icon type
MoreIcons::updatePlayerObject(playerObject, "my-icon", IconType::Cube);
```

## Credits
- [DeepResonanceX](https://gdbrowser.com/u/5668656) - Ideas for the mod
- [hiimjasmine00](https://gdbrowser.com/u/7466002) - Creator of the mod

# License
This mod is licensed under the [MIT License](./LICENSE).