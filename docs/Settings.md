# GTSPlugin Settings

All user modified settings are stored in Data/SKSE/Plugins/GTSPlugin/Settings.toml.
The mod handles reading and writing to this file on its own, you no longer need to modify this file in order to change some of the mods settings.

See [SettingsList](https://github.com/sermitse/GTS_Plugin/blob/master/src/Config/SettingsList.hpp) for all current settings

To enable the advanced settings category add the following table to the begining of Settings.toml:

```toml
[Hidden]
IKnowWhatImDoing = true
```

This enables the advanced options in the mod's configuration menu and allows which now get automaticaly saved/loaded instead of default values being used.
