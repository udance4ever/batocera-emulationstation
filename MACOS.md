### SUMMARY
0. **hardware**: Apple Silicon (M1) (macOS Sequoia 15.3.2) & Sony DualSense controller
1. **frontend**: `emulationstation` target compiles and crashes after verifying rom directories in config file
	* Batocera splash logo
	* renders window with icons and help text and timestamp on bottom
	* renders dialog box if config file not present
	* reads `es_systems.cfg` and verifies which directories exist
	* reports not existent systems to log file
2. **system coverage**: verified **119** ES systems launchable ([full index](#SYSTEMS-COVERAGE))
	* `.squashfs` support in macOS implemented with [squashfuse](https://github.com/vasi/squashfuse) and [fuse-t](https://github.com/macos-fuse-t/fuse-t)
	* [ES-DE config files](macOS/ES-DE) & [Python launcher scripts](macOS/launchers)

### ISSUES
1. crash after reading `es_systems.cfg` and verifying rom directories
2. [configgen](https://github.com/udance4ever/batocera.linux/tree/master/package/batocera/core/batocera-configgen/configgen/configgen) (non-ES component) dependency on Linux `evdev` Python module

### NEXT STEPS
1. debug seg fault
2. port relevant #ifdef __APPLE__ statements in [ES-DE](https://gitlab.com/es-de/emulationstation-de/-/tree/master) codebase
	* controller input (this is the current block as [evdev is Linux specific](https://stackoverflow.com/questions/20701715/how-to-install-python-evdev-on-mac-os/20711677#20711677) (used in [configgen](https://github.com/udance4ever/batocera.linux/tree/master/package/batocera/core/batocera-configgen/configgen/configgen)))
		* extract ES-DE controller input component & build helper executable in C# or
		* port `evdev` dependencies using a more portable Python equivalent (pygame?)
 	* review of macOS-specific changes in [ES-DE CHANGELOG](https://gitlab.com/es-de/emulationstation-de/-/blob/master/CHANGELOG.md)
		* build support: clang/LLVM and Apple Silicon and eliminating Homebrew library support (./external)
	 	* controller drivers
	 	* file case-sensitivity
	    	* code optimizations (eg. reducing background CPU usage)
	        * high DPI display support
	  	* startup animations
	  	* Apple conventions (eg. Command-Q to quit instead of F4 on Windows/UN*X)
	   	* Game Mode compatibility
	   	* cleanup app shutdown
	   	* ES vars: %STARTDIR% and %EMUPATH%

### COMPILATION
Hints to get emulationstation compiled on macOS (MacBookPro 2020 M1 16GB 1TB):

1. copied ES-DE `.clang-format`
2. merged batocera-emulationstation and ES-DE CMake configurations
	* [CMakeLists.txt](CMakeLists.txt)
		* merged options
		* merged if-then branches
		* unify on `GLSYSTEM` (instead of `GLSystem`)
		* changes to ensure include files are found
			* `plugixml/src/pugixml.hpp`
			* `vlc/vlc.h`
	*  merged `CMake/Packages/Find*.make`
3. brought ES-DE [./external](https://gitlab.com/es-de/emulationstation-de/-/tree/master/external) libs and [./tools/macOS_*](https://gitlab.com/es-de/emulationstation-de/-/tree/master/tools) to build locally
4. downloaded manually and built from source:
	* [SDL_mixer-release-2.8.0](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-2.8.0)
	* [vlc-3.0.17.3](http://download.videolan.org/pub/videolan/vlc/3.0.17.3/)
	  	* `./configure --disable-lua --disable-avcodec --disable-swscale --disable-a52 --disable-sparkle`
		* create symlinks for `libvlc.dylib` and `libvlccore.dylib` (linker doesn't resolve versioned filenames)

### CODE CHANGES

1. internationalization code fails to compile correctly
	* `_()` and `_U()` macros failed to work (solution: globally preprocess and replace)
		* `_()` with `std::string()`
		* `_U()` for string literal
	* `ngettext()` replaced with "plural" C++ string
	* `pgettext()` replaced with message string
	* `LC_TYPE` and `LC_MESSAGES` not defined - commented out code references
	* conditionals with `isRTL()` replaced with (0)
2. WebImageComponent `ptr_fun` - commented out (sure to cause runtime error)
3. renderers/GlExtensions.cpp: consistent use of `PFNGLCREATEPROGRAMPROC` (was non-existent `PFNGLCREATEPROGRAMOBJECTARBPROC`)
4. utils/FileSystemUtil.cpp: `struct stat64` works differently on Apple Silicon
	* https://github.com/rpm-software-management/rpm/pull/1775a
5. vlc API differences
	* libvlc_media_player_set_time(vlcMediaPlayer, ms, **1**)
	* libblc_media_player_stop_async(vlcMediaPlayer)
6. ApiSystem.cpp: uncollapse use of `WEXITSTATUS` macro (behaves differently on Apple) (https://stackoverflow.com/a/13674801)
7. components/GuiComponent declares setSize as a virtual method
	* compiler errors disappared after declaring `GuiImageViewer::setSize` locally
8. components/ImageGridComponent and TextListComponent- IList `using` issues
	* `using = IList<ImageGridData, T>;` allows it to compile
	* `using = IList<TextListData, T>;` allows it to compile
9. libcheevos: cdrom.c: `retry:`  cdrom_send_command() missing if not compiling on Windows or Linux
10. OpenGL compatibility profile - `SDL_GL_SetAttribute()` needs to be called between `SDL_Init()` and `SDL_CreateWindows()` or window attributes will not be set

### KNOWN NOT WORKING (prior to fork)

1. class VolumeControl - removed preprocessor `#error` statements so it does not halt compilation (may cause crash during runtime)

### SYSTEMS COVERAGE
* [ES-DE](https://es-de.org) 3.2.0-alpha used as basis to verify underlying launchability on Apple Silicon
* 106 Batocera systems working & tested:
	* 3do, cdi
	* 3ds (Citra/[Lime3DS](https://github.com/Lime3DS/lime3ds-archive) with .squashfs support using Python launcher)
	* apple2e, amstradcpc, macintosh, fmtowns ([MAME](https://github.com/mamedev/mame))
	* apple2gs ([GSPlus](https://github.com/digarok/gsplus) using Python launcher script for configgen)
	* arduboy, megaduck
	* atari2600, atari5200, atari7800, atari800, atarist, jaguar, lynx
	* intellivsion, colecovision, vectrex, o2em, wswan, wswanc
	* c64, amiga500, amiga1200, amigacd32, amigcdtv
	* coco ([XRoar](https://www.6809.org.uk/xroar/) w rudimentary autoboot support)
	* daphne, singe ([Hypseus Singe](https://github.com/DirtBagXon/hypseus-singe) w .squashfs support via Python launcher)
	* dreamcast, atomiswave, naomi, naomi2 ([Flycast](https://github.com/flyinghead/flycast))
	* gameandwatch ([MAME](https://github.com/mamedev/mame))
	* gamecube, wii ([Dolphin](https://github.com/dolphin-emu/dolphin))
	* gb, gba, gbc
	* gb2players ([tgbdual-libretro](https://github.com/libretro/tgbdual-libretro))
	* megadrive, segacd, sega32x, gamegear, mastersystem, saturn, sg1000
	* mame (arcade) ([MAME](https://github.com/mamedev/mame))
	* fnbeo, neogeo ([FinalBurn Neo](https://github.com/finalburnneo/FBNeo))
	* model3 ([Supermodel](https://github.com/trzy/Supermodel))
	* namco2x6 ([Play!](https://github.com/jpd002/Play-))
	* msx, msx2
	* n64
	* n64dd ([ares](https://github.com/ares-emulator/ares))
	* nds, nes, fds
	* neogeocd, ngp, ngpc
	* pcengine, pcenginecd, pcfx, supergrafx
	* prboom, quake, quake2, ecwolf
	* psx
	* ps2 ([PCSX2](https://github.com/PCSX2/pcsx2))
	* ps3 ([RPCS3](https://github.com/RPCS3/rpcs3) with .squashfs support using Python launcher)
	* psp ([PPSSPP](https://github.com/hrydgard/ppsspp))
	* psvita ([Vita3K](https://github.com/Vita3K/Vita3K) with .psvita launch file support using Python launcher)
	* reminiscence
	* satellaview, snes
	* scummvm ([ScummVM](https://github.com/scummvm/scummvm) with .squashfs support using Python launcher)
	* snes-msu1 (with .squashfs support via Python launcher)
	* solarus ([Solarus](https://www.solarus-games.org) Solarus Launcher)
	* steam
	* switch (Ryujinx)
	* thomson
	* tic80, wasm4, pokemini
	* uzebox, vircon32
	* xrick, cannonball, mrboom, superbroswar, cavestory
	* sdlpop ([SDLPoP](https://github.com/NagyD/SDLPoP)) ([build instructions](https://github.com/NagyD/SDLPoP?tab=readme-ov-file#macos-1))
	* vpinball ([Visual Pinball](https://github.com/vpinball/vpinball))
	* virtualboy
	* wiiu ([Cemu](https://github.com/cemu-project/Cemu))
	* windows (WINE in [Whisky](https://github.com/Whisky-App/Whisky))
	* x1, x68000
	* zxspectrum
	* xbox, chihiro ([xemu](https://github.com/xemu-project/xemu) with .squashfs support using Python launcher)
* 6 launch (so ES is not the issue) but emulation appears broken
	* c128 - keyboard won't bind (NOTE: works in Linux)
	* gx4000 - stuck on "Ready" (missing autoboot commands?)
	* pc98 - controller bindings missing (same issue on Windows, macOS, and Linux)
	* pet - controls don't seem to do anything (same issue in Linux)
	* pico8 - game doesn't run/boot (static image)
	* zx81 - doesn't respond to input (keyboard or controller) - stuck on "Anyone there?" (get past this screen in Linux)
* 7 additional working RetroArch cores available in macOS
	* ports - NOTE: severalrequire `%INJECT%=nothing` workaround ([ES-DE#1930](https://gitlab.com/es-de/emulationstation-de/-/issues/1930))
		* 2048 ([2048-libretro](https://github.com/libretro/libretro-2048))
		* craft ([craft-libretro](https://github.com/libretro/craft))
		* dinothawr ([dinothawr-libretro](https://github.com/libretro/dinothawr))
		* gong ([gong-libretro](https://github.com/libretro/gong))
		* jumpnbump ([jumpnbump-libretro](https://github.com/libretro/jumpnbump-libretro))
		* thepowdertoy ([thepowertoy-libretro](https://github.com/libretro/ThePowderToy))
	* palm ([Mu-libretro](https://github.com/libretro/Mu))
* 1 not tested
	* jaguarcd (Catalyst version of [BigPEmu.app exists in App Store](https://apps.apple.com/us/app/bigpemu/id6737359949) for $9.99)
* 5 future
	* xbox360 (custom Xenia with AVX check removed verified working in Whisky)
		* [WhiskyCmd run: what path should I use?](https://github.com/Whisky-App/Whisky/issues/1325)
	* lindbergh ([Lindbergh Loader](https://github.com/lindbergh-loader/lindbergh-loader)) (port to macOS)
	* triforce ([Dolphin Triforce](https://github.com/JuanMiguelBG/dolphin-triforce))
		* ([macOS (M1) build errors](https://github.com/JuanMiguelBG/dolphin-triforce/issues/2))
	* model2 ([model2Emu](https://github.com/batocera-linux/model2emu)) (via WINE (not working yet))
	* hikaru ([Demul](http://demul.emulation64.com)) (via WINE (not working yet))
