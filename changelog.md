# More Icons Changelog
## v2.0.0 (2026-02-10)
- Ported to Geometry Dash v2.208 / Geode SDK v5.0.0
- Added custom death effect and ship fire support
- Renamed `MoreIconsV2.hpp` to `MoreIcons.hpp` and made it the main API header

## v1.16.2 (2025-12-08)
- Fixed a bug where some icons with weird offsets would be cut off when exported (I don't know asc095's in-game name but thanks for reporting!)
- Fixed a bug where the icon viewer would not show the icon pieces (I LITERALLY forgot to fix this :sob:)

## v1.16.1 (2025-12-08)
- Fixed a bug where some scroll layers would get freaky if it didn't have enough content to scroll ([#52](https://github.com/hiimjasmine00/MoreIcons/issues/52))
- Fixed a bug where some icons from texture packs would not load properly
- Removed Happy Textures from the mod dependencies
- Other miscellaneous fixes and improvements

## v1.16.0 (2025-12-06)
- Added load/save state functionality to the icon editor
- Added color previews to the icon editor
- Added green flashing to the PNG/Plist buttons in the icon editor when the other file needs to be selected
- Added pink selection to the PNG/Plist buttons in the icon editor when the file is selected
- Changed the clear piece button to remove the extra piece instead of making it empty
- Moved trail saving to the trail settings popup
- Changed the icon viewer popup to show the icon name in the title
- Fixed a bug where clicking a trail in the vanilla icon list popup would crash the game
- Fixed a bug where saving an icon with a 0 size piece would crash the game (Reported by [Doritosxz](user:14012181))
- Fixed a possible bug where the icon editor would not properly transform the extra piece preview
- Fixed some pre-multiplied alpha issues when saving icons with presets
- Removed some logs I left in by accident
- Refactored the API in `MoreIconsV2.hpp`, utilizing Geode's event export system
  - This will replace `MoreIcons.hpp` in v2.0.0, which will be released after the release of Geometry Dash v2.208
- Another HUGE internal refactor

## v1.15.2 (2025-12-01)
- Fixed a bug where unloading icons on game exit would crash the game (Reported by [DeepResonanceX](user:5668656))

## v1.15.1 (2025-12-01)
- Added 2 pixels of transparent padding around exported icon pieces to fix overlapping issues (Suggested by [DeepResonanceX](user:5668656))
- Fixed a bug where saving an icon with a cleared piece would crash the game (Reported by [UltraDestroya48](user:21986176))
- Updated the gallery with new screenshots

## v1.15.0 (2025-11-30)
- It took me four months to make this update. Let that sink in.
- Revamped the icon editor UI to support more extensive editing features
  - Added the ability to customize icon piece positions, scales, and rotations
  - Split the PNG and Plist buttons into separate buttons again since that was probably very confusing
  - Added presets taken from vanilla icons and your loaded icons
  - Added the ability to clear individual icon pieces
  - Added exporting to different icon qualities (SD, HD, UHD)
- Replaced the "Debug Logs" and "Info Logs" settings with a single "Log Level" setting
- Fixed a bug where custom robots and spiders would not properly display their ghost trail (Reported by [Drongostache](user:11558507))
- Added advanced IconInfo comparison operators to the API
- Many internal code improvements and optimizations

## v1.14.7 (2025-08-19)
- Added an API method to open the icon info popup (Suggested by [NyteLyte](user:26352471))
- Removed API macros and replaced `MORE_ICONS_EXPAND` with operator `""_mi`

## v1.14.6 (2025-08-18)
- Refactored the icon kit code to improve mod compatibility ([#44](https://github.com/hiimjasmine00/MoreIcons/pull/44))
- Update by [NyteLyte](user:26352471) and [hiimjasmine00](user:7466002)

## v1.14.5 (2025-07-17)
- Fixed a rare crash when unloading icons ([#41](https://github.com/hiimjasmine00/MoreIcons/issues/41))

## v1.14.4 (2025-07-14)
- Fixed a few bugs related to overwriting icons with the icon editor

## v1.14.2 (2025-07-06)
- Possibly fixed a bug relating to converting vanilla trails to More Icons trails

## v1.14.1 (2025-07-05)
- Moved the icon converting, renaming, and trashing buttons to the icon popup in the icon kit
- Fixed a bug where the "Preload Icon Textures" setting would load the icons twice (And in the main thread :scream:)

## v1.14.0 (2025-07-01)
- Deprecated folder icons, which will be converted to file icons on first load
- Added the ability to re-name an icon in the icon viewer ([#32](https://github.com/hiimjasmine00/MoreIcons/issues/32))
- Added the ability to convert a traditional pack icon to a More Icons pack icon in the icon viewer ([#32](https://github.com/hiimjasmine00/MoreIcons/issues/32))
- Improved icon trashing, which now accounts for different icon qualities ([#32](https://github.com/hiimjasmine00/MoreIcons/issues/32#issuecomment-2994636123))
- Removed the "Reload Textures" button from the More Icons popup, now handling the reloading automatically
- Merged the "PNG" and "Plist" buttons in the icon editor into a single button

## v1.13.7 (2025-06-22)
- Added the ability to disable non-icon info logs in the mod settings
- Fixed a bug where custom trails from vanilla icon packs would not be added to the icon kit

## v1.13.6 (2025-06-17)
- Removed the cap on the number of icons in traditional icon packs (Suggested by [benBabies](user:9678950))

## v1.13.5 (2025-06-16)
- Added the mod version to the More Icons popup
- Fixed custom trails not showing in the More Icons popup
- Fixed severity ordering in the log viewer
- Fixed mini icons in traditional icon packs being counted as custom icons ([#36](https://github.com/hiimjasmine00/MoreIcons/issues/36))
- Fixed even more pre-multiplied alpha issues ([#37](https://github.com/hiimjasmine00/MoreIcons/issues/37))

## v1.13.4 (2025-06-05)
- Actually fixed the bug where custom robots and spiders would not display properly (This better work)

## v1.13.3 (2025-06-03)
- Fixed a bug where the robot and spider would overlap in the icon kit (Reported by [d4rkn](user:25413733))
- Fixed a bug where custom robots and spiders would not work properly (Fingers crossed)

## v1.13.2 (2025-06-02)
- Fixed some image processing issues ([#35](https://github.com/hiimjasmine00/MoreIcons/issues/35))

## v1.13.1 (2025-05-28)
- Added a setting to preload all icon textures ([#30](https://github.com/hiimjasmine00/MoreIcons/issues/30))
- Improved the loading of the icon viewer
- Added notifications to the icon editor when files cannot be imported
- Fixed blending issues while exporting icons in the icon editor
- Fixed a possible crash with the vanilla icon viewer on Android while using medium or low texture quality
- Fixed a crash when opening the More Icons popup after installing Separate Dual Icons (Reported by [Zilko](user:10300913))
- Fixed a crash when importing a malformed plist file into the icon editor

## v1.13.0 (2025-05-14)
- Revamped the More Icons popup with a new design and more features
- Added an icon editor, which allows you to create new icons by importing icon parts
- Added custom and vanilla icon viewers, with clickable icons
- Improved the log viewer and split it across the icon types
- Added the icon trash, which can be used via the icon viewer and accessed via the More Icons popup
- Added a new API method to get the icon info of the current icon
- Added a gallery for the new popups

## v1.12.3 (2025-04-28)
- Fixed a few issues with mass loading, especially when opening the level editor

## v1.12.2 (2025-04-24)
- Fixed a bug where the game would crash when opening the log viewer on 32-bit Android

## v1.12.1 (2025-04-06)
- Fixed a possible crash when opening the icon info display on iOS

## v1.12.0 (2025-03-31)
- Added support for iOS
- Added more trail customization options (Always Show, Fade Time, Stroke Width)
- Reworked the icon info display in the icon kit
- Reworked the icon loading system to more resemble the vanilla icon loading system
- Added node IDs to the log viewer in the icon kit

## v1.11.1 (2025-03-19)
- Fixed a bug where icons would not load properly (Reported by [NelsonGD54](user:11922670))

## v1.11.0 (2025-03-19)
- Revamped the API, renaming many methods and adding new ones
- Lowered the opacity of the cursor in the icon kit for vanilla selected icons
- Fixed MoreIcons::loadIcon and MoreIcons::unloadIcon not working properly
- Fixed probable crashes caused by malformed plist files (Reported by [MarioTudor22](user:26834754))

## v1.10.1 (2025-03-10)
- Fixed a bug where the game would crash when using a custom trail without entering the icon kit ([#28](https://github.com/hiimjasmine00/MoreIcons/issues/28))

## v1.10.0 (2025-03-09)
- Changed icon loading to only load icon textures when needed (Suggested by [qjivxn](user:5327860))
- Added new API methods (MoreIcons::loadIcon, MoreIcons::unloadIcon)
- Fixed incompatibilities with "Known Players" by iAndyHD3 and "Pity Title Screen Secret Icons" by kittenchilly

## v1.9.1 (2025-03-08)
- Fixed a bug that sometimes caused selecting dual icons to not work properly ([#23](https://github.com/hiimjasmine00/MoreIcons/issues/23))
- Fixed an incompatibility with the mod "Better Unlock Info" by Rynat ([#26](https://github.com/hiimjasmine00/MoreIcons/issues/26))
- Fixed a probable crash that can occur when loading a vanilla icon (Reported by [Dasshu](user:1975253))

## v1.9.0 (2025-03-07)
- Added debug logs, which can be disabled in the settings
- Added a setting to disable loading traditional icon texture packs ([#24](https://github.com/hiimjasmine00/MoreIcons/issues/24))
- Added simpler API overloads and a new method to get the name of an icon
- [Added API documentation](https://more-icons.hiimjasmine00.com)
- Added user objects for custom trails
- Changed the behavior to check plist extension replacement first
- Fixed some memory leaks
- Fixed a bug where selecting a locked icon would reset the player's icon ([#25](https://github.com/hiimjasmine00/MoreIcons/issues/25))

## v1.8.2 (2025-02-27)
- Fixed the icon sorter crashing with file names that end with numbers bigger than 2,147,483,647 (Reported by [Sharkcrome](user:26481173))

## v1.8.1 (2025-02-26)
- Added pagination to the log viewer in the icon kit
- Fixed a bug that caused the 6th trail to not work properly (Reported by [lucaswiese6](user:27435227))

## v1.8.0 (2025-02-26)
- Added traditional icon texture pack support
- Fixed a bug that caused the robot/spider in the icon kit to stay visible when selecting a new icon ([#21](https://github.com/hiimjasmine00/MoreIcons/issues/21))

## v1.7.1 (2025-01-29)
- Fixed a bug that caused custom icons to not work in-game

## v1.7.0 (2025-01-29)
- Added an API method to set the player's icon
- Fixed the API being private

## v1.6.1 (2025-01-28)
- Fixed header inclusion in the API

## v1.6.0 (2025-01-23)
- Added a header-only API for other mods to use
- Fixed a bug where the player's icon would not change properly when exiting dual mode (Reported by [Cocoiscool748](https://github.com/Cocoiscool748))

## v1.5.3 (2024-12-24)
- Fixed a bug where the game would crash when using a custom trail on macOS ([#18](https://github.com/hiimjasmine00/MoreIcons/issues/18))

## v1.5.2 (2024-12-11)
- Fixed a bug where selecting a new icon would not work properly in the icon kit

## v1.5.1 (2024-12-11)
- Separated icon loading by icon type
- Separated icons by texture pack in the icon kit ([#17](https://github.com/hiimjasmine00/MoreIcons/issues/17))
- Fixed a bug that would reset the player's icon when exiting dual mode ([#15](https://github.com/hiimjasmine00/MoreIcons/issues/15))
- Fixed pages not updating properly in the icon kit using arrow buttons (Reported by [NyteLyte](user:26352471))

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
- Fixed texture pack credits only showing up for cubes ([#14](https://github.com/hiimjasmine00/MoreIcons/issues/14))

## v1.3.4 (2024-10-21)
- Moved some logic to More Icons API
- Tweaked the logo to add drop shadow

## v1.3.3 (2024-10-19)
- Improved the duplicate system
- Changed page functionality in the icon kit
- Added texture pack credits to the icon kit ([#8](https://github.com/hiimjasmine00/MoreIcons/issues/8))
- Added number formatting to the info popup in the icon kit

## v1.3.2 (2024-10-13)
- Changed the More Icons button in the icon kit to only show the info when there are no logs ([#10](https://github.com/hiimjasmine00/MoreIcons/issues/10))

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
- Fixed blend mode not working properly ([#5](https://github.com/hiimjasmine00/MoreIcons/issues/5))
- Fixed trail customization resetting when reloading textures
- Fixed trail display in the icon kit
- Fixed loading text not showing up

## v1.2.2 (2024-10-08)
- Added support for individual images per icon piece
- Added trail customization options ([#5](https://github.com/hiimjasmine00/MoreIcons/issues/5))
- Fixed robots and spiders not changing in icon popups

## v1.2.1 (2024-10-08)
- Fixed custom spiders not showing up in game ([#4](https://github.com/hiimjasmine00/MoreIcons/issues/4))

## v1.2.0 (2024-10-07)
- Added support for texture packs
- Added support for custom trails
- Added support for Icon Profile in the main menu and Animated Profile Icons in the icon kit
- Fixed a bug where the robot's boost particles would not show up ([#3](https://github.com/hiimjasmine00/MoreIcons/issues/3))

## v1.1.2 (2024-10-07)
- Added sprite name detection for spritesheets
- Added icon loading text to the loading screen

## v1.1.1 (2024-10-06)
- Fixed custom jetpacks not showing up in game
- Fixed icon sorting not working properly ([#2](https://github.com/hiimjasmine00/MoreIcons/issues/2))

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
