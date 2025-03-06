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
Icon texture packs can be loaded with the mod "Texture Loader" by Geode Team. The texture pack should be placed in `(Geometry Dash folder)/config/geode.texture-loader/packs/(pack name)`, where `(pack name)` is the name of the texture pack, or a zip file containing the texture pack. The same rules apply for the texture pack as they do for the custom icons, with the sprites being added to `(pack name)/config/hiimjustin000.more_icons/(gamemode)`, where `(gamemode)` is the gamemode the icon is for.

Traditional icon texture packs can be loaded without prior modification, and More Icons will separate the icons without replacing the original icons. However, trail properties from these packs cannot be modified; they are instead derived from vanilla trails. This behavior can be disabled in the mod settings.

## Using Icons
To use a custom icon, you will need to go into the icon kit. In the icon kit, there is a second row of dots that you can click on to view the extra icons. Click on the dot (or scroll to it with the page arrows) to view a page of extra icons, and click on one of the icons to select it.

To deselect a custom icon, use the first row of dots to go back to the default icons, and click on one of the default icons to select it.

## Credits
- [DeepResonanceX](https://gdbrowser.com/u/5668656) - Ideas for the mod
- [hiimjasmine00](https://gdbrowser.com/u/7466002) - Creator of the mod

# License
This mod is licensed under the [MIT License](./LICENSE).