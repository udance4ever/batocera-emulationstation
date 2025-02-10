### SUMMARY
1. `emulationstation` target compiles and crashes after verifying rom directories in config file
	* Batocera splash logo
	* renders window with icons and help text and timestamp on bottom
	* renders dialog box if config file not present
	* reads `es_systems.cfg` and verifies which directories exist
	* reports not existent systems to log file
2. alpha build of [ES-DE](https://es-de.org) tested & working on Apple Silicon (M1) (macOS Sequoia 15.3)
	* workaround: enables ES-DE to launch Python-based launcher scripts (issue reported)
		* `alias ES-DE='LANG="en_US.UTF-8" /Applications/#emu/ES-DE.app/Contents/MacOS/ES-DE'`
	* .squashfs support in macOS implemented with [squashfuse](https://github.com/vasi/squashfuse)
	* Batocera systems working & tested:
		* 3do, amstradcpc, gx4000
		* apple2e (MAME)
		* apple2gs (GSPlus using Python launcher script for configgen)
		* atari2600, atari5200, atari7800, atari800
		* jaguar, lynx, wsanc, cannonball, colecovision, c64, amiga500, amiga1200, amigacd32, amigcdtv, prboom, fnbeo, gb2players, vectrex, o2em, intellivision
		* windows (WINE in Whisky)
		* xbox (xemu with .squashfs support using Python launcher)
		* mrboom, msx, msx2
		* mame (standalone MAME 0.274)
		* pcengine, pcenginecd, pc98, pcfx, supergrafx
		* 3ds (Citra/Lime3DS with .squashfs support using Python launcher)
		* n64 (also works in Ares)
		* nds, nes, fds
		* gameandwatch (MAME)
		* gb, gba, gbc
		* gamecube, wii (Dolphin)
		* pokemini, satellaview, snes
		* switch (Ryujinx)
		* virtualboy
		* wiiu (Cemu)
		* cdi (MAME only, RetroArch core not built for M1)
		* pico8, tyrquake
		* dreamcast, atomiswave, naomi, naomi2 (Flycast)
		* scummvm (ScummVM with .squashfs support using Python launcher)
		* segacd, gamegear, mastersystem, megadrive, sega32x, saturn, sg1000, x68000, zxspectrum, neogeo, neogeocd, ngpc
		* solarus (Solarus Launcher)
		* psx
		* ps2 (PCSX2)
		* ps3 (RPCS3 with .squashfs support using Python launcher)
		* psp (PPSSPP)
		* psvita (Vita3K with .psvita launch file support using Python launcher)
		* superbroswar
		* coco (XRoar w rudimentary autoboot support)
		* tic80, wasm4, xrick
	* additonal working RetroArch core
		* palm
	* experimental
		* xbox360 (custom Xenia with AVX check removed verified working in Whisky)
	* not tested
		* jaguarcd (Catalyst version of BigPEmu.app exists in App Store for $9.99)
		* wswan, ngp (assumed working as color counterparts work: wswanc, ngpc)
	* known missing
		* triforce (Dolphin Triforce M1 build not known avail)

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
