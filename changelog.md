# More Icons Changelog
## v1.6.0 (2025-01-23)
- Added a header-only API for other mods to use
- Fixed a bug where the player's icon would not change properly when exiting dual mode (Reported by [Cocoiscool748](https://github.com/Cocoiscool748))

## v1.5.3 (2024-12-24)
- Fixed a bug where the game would crash when using a custom trail on macOS ([#18](https://github.com/hiimjustin000/MoreIcons/issues/18))

## v1.5.2 (2024-12-11)
- Fixed a bug where selecting a new icon would not work properly in the icon kit

## v1.5.1 (2024-12-11)
- Separated icon loading by icon type
- Separated icons by texture pack in the icon kit ([#17](https://github.com/hiimjustin000/MoreIcons/issues/17))
- Fixed a bug that would reset the player's icon when exiting dual mode ([#15](https://github.com/hiimjustin000/MoreIcons/issues/15))
- Fixed pages not updating properly in the icon kit using arrow buttons (Reported by [NyteLyte](https://github.com/NyteLytee))

## v1.5.0 (2024-11-18)
- Added custom icons to the main menu
- Fixed a bug where switching separate dual icons would not work properly in the icon kit
- Fixed crashing incompatibilities with the mod "Fine Outline" by Alphalaneous
- Fixed some potential spider weirdness

## v1.4.6 (2024-11-15)
- Ported to Geode v4.0.0-beta.1

## v1.4.6-beta.1 (2024-11-15)
- Ported to Geometry Dash v2.207

## v1.4.5 (2024-11-04)
- Fixed potential blending issues with robots and spiders

## v1.4.4 (2024-11-02)
- Fixed events not working properly

## v1.4.3 (2024-10-29)
- Tweaked the icon kit functionality
- Added user objects for custom icons

## v1.4.2 (2024-10-27)
- Fixed a bug where the icon glow would be offset in the game

## v1.4.1 (2024-10-27)
- Tweaked the trail info display in the icon kit

## v1.4.0 (2024-10-27)
- Merged the More Icons API into the mod
- Added support for events for other mods to use
- Fixed texture pack credits only showing up for cubes ([#14](https://github.com/hiimjustin000/MoreIcons/issues/14))

## v1.3.4 (2024-10-21)
- Moved some logic to More Icons API
- Tweaked the logo to add drop shadow

## v1.3.3 (2024-10-19)
- Improved the duplicate system
- Changed page functionality in the icon kit
- Added texture pack credits to the icon kit ([#8](https://github.com/hiimjustin000/MoreIcons/issues/8))
- Added number formatting to the info popup in the icon kit

## v1.3.2 (2024-10-13)
- Changed the More Icons button in the icon kit to only show the info when there are no logs ([#10](https://github.com/hiimjustin000/MoreIcons/issues/10))

## v1.3.1 (2024-10-12)
- Fixed a bug where the game would crash when pressing the More Icons button in the icon kit

## v1.3.0 (2024-10-12)
- Added ability to view logs and amount of loaded icons in the icon kit
- Changed logs to be more specific
- Updated tutorial links in the mod's description

## v1.2.7 (2024-10-10)
- Fixed plist icons changing names

## v1.2.6 (2024-10-10)
- Fixed robots and spiders sometimes crashing the game
- Fixed icons with individual images being added multiple times
- Fixed dual icons not being deselected properly
- Added loading text while loading the game

## v1.2.5 (2024-10-10)
- Utilized multiple CPU cores for loading icons, reducing load times
- Fixed non-robots and non-spiders not changing in icon popups

## v1.2.4 (2024-10-09)
- Fixed trails not adding to the mod's saved values
- Removed loaded icons from the mod's saved values when closing the game

## v1.2.3 (2024-10-08)
- Fixed blend mode not working properly ([#5](https://github.com/hiimjustin000/MoreIcons/issues/5))
- Fixed trail customization resetting when reloading textures
- Fixed trail display in the icon kit
- Fixed loading text not showing up

## v1.2.2 (2024-10-08)
- Added support for individual images per icon piece
- Added trail customization options ([#5](https://github.com/hiimjustin000/MoreIcons/issues/5))
- Fixed robots and spiders not changing in icon popups

## v1.2.1 (2024-10-08)
- Fixed custom spiders not showing up in game ([#4](https://github.com/hiimjustin000/MoreIcons/issues/4))

## v1.2.0 (2024-10-07)
- Added support for texture packs
- Added support for custom trails
- Added support for Icon Profile in the main menu and Animated Profile Icons in the icon kit
- Fixed a bug where the robot's boost particles would not show up ([#3](https://github.com/hiimjustin000/MoreIcons/issues/3))

## v1.1.2 (2024-10-07)
- Added sprite name detection for spritesheets
- Added icon loading text to the loading screen

## v1.1.1 (2024-10-06)
- Fixed custom jetpacks not showing up in game
- Fixed icon sorting not working properly ([#2](https://github.com/hiimjustin000/MoreIcons/issues/2))

## v1.1.0 (2024-10-06)
- Added support for Separate Dual Icons
- Added loaded icon list in saved.json
- Added icon popups when selecting an icon

## v1.0.4 (2024-10-04)
- Fixed pages resetting when selecting a new tab
- Fixed a bug where the game would crash when selecting the death effect tab

## v1.0.3 (2024-10-04)
- Fixed a bug where selecting a ball, wave, or swing icon would change the cube icon
- Fixed a bug where the jetpack menu would not show the first row of navigation dots

## v1.0.2 (2024-10-04)
- Added support for excluding lower-quality spritesheets and atlases
- Ignored directories in spritesheet texture names

## v1.0.1 (2024-10-04)
- Fixed texture reloading not working
- (Maybe, possibly) Fixed an incompatibility with the mod "Animated Profile Icons" by TheSillyDoggo
- Fixed many other bugs

## v1.0.0 (2024-10-04)
- Initial release