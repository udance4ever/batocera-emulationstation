Aug 20, 2024

### SUMMARY
1. `emulationstation` target compiles to with no errors in this patched fork of batocera-emulationstation but OpenGL fails to initialize
2. alpha build of [ES-DE](https://es-de.org) launches all RetroArch cores (sans same_cdi) and standalone emulators (sans Vita3K) on Apple Silicon (M1) (macOS Sonoma 14.6.1)
	* custom Xenia build working in beta macOS Sequoia 15 w AVX support in Rosetta 2

### ISSUES
current block: GL extension checks in `initializeGlExtensions()` fail at runtime

an attempt to ignore these checks results in ES attempting to initialize video (black screen) and reporting errors ([full log](https://pastebin.com/vL6PcTdY)):
```
udance4ever@host batocera-emulationstation % ./emulationstation
2024-08-19 15:11:18	ERROR	GL Extensions not supported. GL_ARB_shader_objects
2024-08-19 15:11:18	ERROR	GL Extensions not supported. GL_ARB_shading_language_100
2024-08-19 15:11:18	ERROR	GL Extensions not supported. GL_ARB_vertex_shader
2024-08-19 15:11:18	ERROR	GL Extensions not supported. GL_ARB_fragment_shader
2024-08-19 15:11:18	ERROR	GL error: glShaderSource(id, 1, &source, nullptr) failed with error code: 1280
2024-08-19 15:11:18	ERROR	GLSL Vertex Compile Error
ERROR: 0:1: '' :  version '120' is not supported
ERROR: 0:2: '' :  #version required and missing.
ERROR: 0:4: 'attribute' : syntax error: syntax error
```
### PROPOSED SOLUTION

assess effort to backport [Renderer from ES-DE](https://gitlab.com/es-de/emulationstation-de/-/blob/master/es-core/src/renderers/Renderer.cpp) (completely rewritten and verified working on macOS)

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
	* SDL_mixer-release-2.8.0
	* vlc-3.0.17.3
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
* https://github.com/rpm-software-management/rpm/pull/1775

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

### KNOWN NOT WORKING (prior to fork)

1. class VolumeControl - removed preprocessor `#error` statements so it does not halt compilation (may cause crash during runtime)
