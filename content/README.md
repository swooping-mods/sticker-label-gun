# Sticker Label Gun

Buy the Sticker Label Gun in Modded Shop, then assign it in Equipment.

## Install

Install [Moon Sweepers Mod Loader](https://github.com/swooping-mods/moon-sweepers-mod-loader) first.

Download [sticker-label-gun-0.2.1.msmod](content/sticker-label-gun-0.2.1.msmod), import it with **Install mod package**, then select the mod and its required frameworks.

Required mods: [shop-framework](https://github.com/swooping-mods/shop-framework), [equipment-framework](https://github.com/swooping-mods/equipment-framework), [hints-framework](https://github.com/swooping-mods/hints-framework).

## Controls

- Buy the Sticker Label Gun in Modded Shop, then assign it in Equipment.
- Select its hotbar slot; Q cycles designs, R places a sticker and Backspace undoes.
- Native shop/equipment integration implemented; gameplay testing deferred.

## Compatibility

Version 0.2.1. Targets the pinned Steam Windows x64 IL2CPP build of Moon Sweepers (Steam app 5335510). On Linux, the game runs through Proton. Experimental: back up saves before using save-affecting mods.

## Build

Run `python3 build.py --check` to verify source hashes, then `python3 build.py` to write a build under `.local/build/`. Requires installed Clang, lld-link and llvm-dlltool.

## Credits

See [CREDITS.txt](CREDITS.txt) and [third-party notices](vendored/THIRD_PARTY_NOTICES.md).
