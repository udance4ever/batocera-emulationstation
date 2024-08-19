# Aug 18, 2024 - getting batocera-emulationstation to build on macOS

cmake_minimum_required(VERSION 3.10)
# $$ ES-DE
if(APPLE)
    MESSAGE("Targeting Apple")
    # Set this to the minimum supported macOS version, and also update
    # es-app/assets/ES-DE_Info.plist accordingly.
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "macOS deployment target")
    # This optional variable is used for code signing the DMG installer.
    set(MACOS_CODESIGN_IDENTITY "" CACHE STRING "macOS code signing certificate identity")
endif()

# Set this to ON to show verbose compiler output (e.g. compiler flags, include directories etc.)
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Show verbose compiler output" FORCE)

# Package type to use for CPack on Linux.
set(LINUX_CPACK_GENERATOR DEB CACHE STRING "CPack generator, DEB or RPM")
# $$ end ES-DE

# Define the options.
#option(GLES "Set to ON if targeting Embedded OpenGL" ${GLES})
option(GLES "Set to ON if targeting Embedded OpenGL" OFF)
# $$ option(GLES2 "Set to ON if targeting OpenGL ES 2.0" ${GLES2})
#option(GL "Set to ON if targeting Desktop OpenGL" ${GL})
option(GL "Set to ON if targeting Desktop OpenGL" ON)

# $$ ES-DE
option(APPLICATION_UPDATER "Set to OFF to build without the application updater" ON)
# $$ 
#  /bin/sh: --add-comments=TRANSLATION: command not found
#  make[2]: *** [i18n] Error 127
#  make[1]: *** [locale/CMakeFiles/i18n.dir/all] Error 2
#option(COMPILE_LOCALIZATIONS "Set to OFF to skip compilation of localization message catalogs" ON)
option(COMPILE_LOCALIZATIONS "Set to OFF to skip compilation of localization message catalogs" OFF)
option(APPIMAGE_BUILD "Set to ON when building as an AppImage" OFF)
option(AUR_BUILD "Set to ON when building for the AUR" OFF)
option(FLATPAK_BUILD "Set to ON when building as a Flatpak" OFF)
option(STEAM_DECK "Set to ON to enable a Valve Steam Deck specific build" OFF)
option(RETRODECK "Set to ON to enable a RetroDECK specific build" OFF)
# $$ (below) option(RPI "Set to ON to enable a Raspberry Pi specific build" OFF)
option(BUNDLED_CERTS "Set to ON to use bundled TLS/SSL certificates" OFF)
# $$ (below) option(CEC "Set to ON to enable CEC" OFF)
option(VIDEO_HW_DECODING "Set to ON to enable FFmpeg HW decoding" OFF)
option(DEINIT_ON_LAUNCH "Set to ON to deinitialize on game launch" OFF)
option(CLANG_TIDY "Set to ON to build using the clang-tidy static analyzer" OFF)
option(ASAN "Set to ON to build with AddressSanitizer" OFF)
option(TSAN "Set to ON to build with ThreadSanitizer" OFF)
option(UBSAN "Set to ON to build with UndefinedBehaviorSanitizer" OFF)
# $$ end ES-DE

#option(RPI "Set to ON to enable the Raspberry PI video player (omxplayer)" ${RPI})
option(RPI "Set to ON to enable the Raspberry PI video player (omxplayer)" OFF)
#option(CEC "CEC" ON)
option(CEC "CEC" OFF)
option(BCM "BCM host" OFF)

option(BATOCERA "Set to ON to enable BATOCERA specific code" OFF)
option(RETROBAT "Set to ON to enable RETROBAT specific code" OFF)

option(ENABLE_FILEMANAGER "Set to ON to enable f1 shortcut for filesystem" OFF)
option(DISABLE_KODI "Set to ON to disable kodi in menu" OFF)
option(ENABLE_PULSE "Set to ON to enable pulse audio (versus alsa)" OFF)
option(ENABLE_TTS "Set to ON to enable text to speech" OFF)
option(USE_SYSTEM_PUGIXML "Set to ON to use system-wide pugixml library" OFF)


# $$ ES-DE
if(CLANG_TIDY)
    find_program(CLANG_TIDY_BINARY NAMES clang-tidy)
    if(CLANG_TIDY_BINARY MATCHES CLANG_TIDY_BINARY-NOTFOUND)
        message("-- CLANG_TIDY was set but the clang-tidy binary was not found")
    else()
        message("-- Building with the clang-tidy static analyzer")
        set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*,\
                                  -fuchsia-*,\
                                  -hicpp-*,\
                                  -llvm-*,\
                                  -readability-braces-*,\
                                  -google-readability-braces-*,\
                                  -readability-uppercase-literal-suffix,\
                                  -modernize-use-trailing-return-type,\
                                  -cppcoreguidelines-avoid-magic-numbers,\
                                  -readability-magic-numbers")
    endif()
endif()

# $$ end ES-DE




# Win32 default platform & directory detection
if(WIN32)

	if (NOT CMAKE_GENERATOR_PLATFORM)
		set(CMAKE_GENERATOR_PLATFORM Win32)	    
	endif()
	
	MESSAGE("-- Destination platform : " ${CMAKE_GENERATOR_PLATFORM})

	if(NOT VLC_VERSION)
		set(VLC_VERSION 1.0.0)
	endif()

	if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../batocera-emulationstation-win32-dependencies)
		set(CUSTOM_LIBDIR ${CMAKE_CURRENT_SOURCE_DIR}/../batocera-emulationstation-win32-dependencies)
	elseif (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/win32-libs) 
		set(CUSTOM_LIBDIR ${CMAKE_CURRENT_SOURCE_DIR}/win32-libs)
	else ()
		MESSAGE("-- Downloading Windows libraries...")
		
		include(FetchContent)
		
		FetchContent_Declare(win32libs
		  GIT_REPOSITORY "https://github.com/batocera-linux/batocera-emulationstation-win32-dependencies.git"
		  GIT_TAG "origin/master"
		  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win32-libs"
		)

		FetchContent_MakeAvailable(win32libs)
		set(CUSTOM_LIBDIR ${CMAKE_CURRENT_SOURCE_DIR}/win32-libs)
	endif()
	
	if(CUSTOM_LIBDIR)		
		MESSAGE("-- Default libraries path set to : " ${CUSTOM_LIBDIR} )		
	endif()
endif()

project(emulationstation-all)

# Win32 custom directories assignation
if(WIN32 AND CUSTOM_LIBDIR)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CUSTOM_PLATFORM "x64")
	else()
		set(CUSTOM_PLATFORM "x86")
	endif()
	
	set(
	  CMAKE_PREFIX_PATH
	  ${CUSTOM_LIBDIR}/rapidjson
	  ${CUSTOM_LIBDIR}/freetype
	  ${CUSTOM_LIBDIR}/freetype/${CUSTOM_PLATFORM}
	  ${CUSTOM_LIBDIR}/freeimage
	  ${CUSTOM_LIBDIR}/freeimage/${CUSTOM_PLATFORM}
	  ${CUSTOM_LIBDIR}/SDL2_mixer
	  ${CUSTOM_LIBDIR}/SDL2_mixer/${CUSTOM_PLATFORM}
	  ${CUSTOM_LIBDIR}/SDL2
	  ${CUSTOM_LIBDIR}/SDL2/${CUSTOM_PLATFORM}
	  ${CUSTOM_LIBDIR}/libvlc
	  ${CUSTOM_LIBDIR}/libvlc/${CUSTOM_PLATFORM}
	  ${CUSTOM_LIBDIR}/curl/${CUSTOM_PLATFORM}
	)
endif()

#-------------------------------------------------------------------------------
#add local find scripts to CMAKE path
LIST(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utils
    ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages
)

if(BATOCERA)
  MESSAGE("Batocera build")
  add_definitions(-DBATOCERA)
endif()

if(RETROBAT)
  MESSAGE("Retrobat build")
  add_definitions(-DRETROBAT)
endif()

# disable kodi menu
if(DISABLE_KODI)
  MESSAGE("Kodi disabled")
else()
  add_definitions(-D_ENABLE_KODI_)
endif()

# batocera / file manager f1 button
# disable file manager
if(ENABLE_FILEMANAGER)
  MESSAGE("file manager enabled")
  add_definitions(-D_ENABLE_FILEMANAGER_)
endif()

if(BCM)
    set(BCMHOST found)
endif()

# $$ ES-DE
#---------------------------------------------------------------------------------------------------
# Emscripten WebAssembly build.

if(EMSCRIPTEN)
    set(GLES ON)
    set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()
# $$ end ES-DE

#-------------------------------------------------------------------------------
#set up OpenGL system variable
if(GLES)
    set(GLSYSTEM "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
elseif(GLES2)
    set(GLSYSTEM "OpenGL ES 2.0" CACHE STRING "The OpenGL system to be used")
elseif(GL)
    MESSAGE("Desktop OpenGL")

    set(GLSYSTEM "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")
#-------------------------------------------------------------------------------
#check if we're running on Raspberry Pi
elseif(EXISTS "${CMAKE_FIND_ROOT_PATH}/opt/vc/include/bcm_host.h")
    MESSAGE("bcm_host.h found")
    set(BCMHOST found)
    set(GLSYSTEM "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
#-------------------------------------------------------------------------------
#check if we're running on OSMC Vero4K
elseif(EXISTS "${CMAKE_FIND_ROOT_PATH}/opt/vero3/lib/libMali.so")
    MESSAGE("libMali.so found")
    set(VERO4K found)
    set(GLSYSTEM "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
#-------------------------------------------------------------------------------
#check if we're running on olinuxino / odroid / etc
elseif(EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/libMali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/libmali.so" OR # batocera
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/arm-linux-gnueabihf/libMali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/aarch64-linux-gnu/libMali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/arm-linux-gnueabihf/mali-egl/libmali.so" OR
    EXISTS "${CMAKE_FIND_ROOT_PATH}/usr/lib/arm-linux-gnueabihf/libmali.so")
    MESSAGE("libMali.so found")
    set(GLSYSTEM "Embedded OpenGL" CACHE STRING "The OpenGL system to be used")
else()
    set(GLSYSTEM "Desktop OpenGL" CACHE STRING "The OpenGL system to be used")
endif(GLES)

set_property(CACHE GLSYSTEM PROPERTY STRINGS "Desktop OpenGL" "Embedded OpenGL")


# https://stackoverflow.com/a/2211981
list(APPEND CMAKE_LIBRARY_PATH /opt/local/lib)

# $$ ES-DE
#---------------------------------------------------------------------------------------------------
# Package dependencies.

# $$ merged w batocera
if(GLSYSTEM MATCHES "Desktop OpenGL")
    MESSAGE("Checking for Desktop OpenGL package...")
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
elseif(ANDROID)
    find_package(OpenGLES3 REQUIRED)
elseif(${GLSYSTEM} MATCHES "OpenGL ES 2.0")
    find_package(OpenGLES2 REQUIRED)
elseif(GLES AND NOT EMSCRIPTEN)
    find_package(OpenGLES2 REQUIRED)
else()
    find_package(OpenGLES REQUIRED)
endif()

# On macOS and Windows all dependencies are kept in-tree in the "external" directory.
if(APPLE)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/FFmpeg)
        message(FATAL_ERROR "-- You need to build the dependencies in ./external first")
    endif()
    find_package(CURL REQUIRED)

    # $$ since this is a /Library/Framework
    #  we compiled SDL in external...
#    find_package(SDL2 REQUIRED)
elseif(WIN32)
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/external/pugixml/pugixml.dll)
        message(FATAL_ERROR "-- You need to build the dependencies in ./external first")
    endif()
elseif(NOT EMSCRIPTEN AND NOT ANDROID)
    find_package(CURL REQUIRED)
    find_package(FFmpeg REQUIRED)
    find_package(FreeImage REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(HarfBuzz REQUIRED)
    find_package(ICU REQUIRED)
    find_package(Intl REQUIRED)
    find_package(Libgit2 REQUIRED)
    find_package(Pugixml REQUIRED)
    find_package(SDL2 REQUIRED)
endif()

# Add libCEC support.
if(CEC)
    MESSAGE("CEC enabled")
    find_package(libCEC REQUIRED)
endif()

# $$ commented out for now
#
# batocera / background musics

# $$ it's not finding it in external/SDLmixer
#find_package(SDL2MIXER REQUIRED)

# batocera / customisations
# $$ skipping VLC even though we are integrating it on macOS
#find_package(VLC REQUIRED)

find_package(RapidJSON REQUIRED)

#if(NOT WIN32)
#	if(ENABLE_PULSE)
#	  MESSAGE("pulse audio enabled")
#	  find_package(PulseAudio REQUIRED)
#	  add_definitions(-D_ENABLE_PULSE_)
#	else()
#	  MESSAGE("pulse audio disabled")
#	endif()
#
#	find_package(udev)
#	if(UDEV_FOUND)
#	  MESSAGE("udev found. guns enabled")
#	  add_definitions(-DHAVE_UDEV)
#	else()
#	  MESSAGE("udev not found. guns will be disabled")
#	endif()
#	
#	if(ENABLE_TTS)
#	  MESSAGE("tts enabled")
#	  find_library(LIBESPEAK_LIBRARIES NAMES espeak REQUIRED)
#	  if("${LIBESPEAK_LIBRARIES}" STREQUAL "LIBESPEAK_LIBRARIES-NOTFOUND")
#		MESSAGE(FATAL_ERROR "espeak not found")
#	  endif()
#	  add_definitions(-D_ENABLE_TTS_)
#	else()
#	  MESSAGE("tts disabled")
#	endif()
#
#	# Add libCEC support.
#	if(CEC)
#	  MESSAGE("CEC enabled")
#	  find_package(libCEC)
#	else()
#	  MESSAGE("CEC disabled")
#	endif()
#endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(ALSA REQUIRED)
endif()

#---------------------------------------------------------------------------------------------------
# Compiler and linker settings.

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    message("-- Compiler is Clang/LLVM")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0.0)
        message(SEND_ERROR "You need at least Clang 5.0.0 to compile ES-DE")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    message("-- Compiler is GNU/GCC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1)
        message(SEND_ERROR "You need at least GCC 7.1 to compile ES-DE")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    message("-- Compiler is MSVC")
    # Disable the built-in min() and max() macros.
    add_compile_definitions(NOMINMAX)
endif()

if(CMAKE_BUILD_TYPE)
    message("-- Build type is ${CMAKE_BUILD_TYPE}")
endif()

# Set up compiler and linker flags for debug, profiling or release builds.
if(CMAKE_BUILD_TYPE MATCHES Debug)
    # Enable the C++17 standard and disable optimizations as it's a debug build.
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /Od /DEBUG:FULL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0 -g3 -Wall -Wpedantic -Wsign-compare -Wnarrowing -Wmissing-field-initializers -Wunused-macros")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0")
    endif()
    # If using Clang, then add additional debug data needed by GDB.
    # Comment this out if you're using LLDB for debugging as this flag makes the binary
    # much larger and the application much slower. On macOS this setting is never enabled
    # as LLDB is the default debugger on this OS.
    if(NOT APPLE AND CMAKE_CXX_COMPILER_ID MATCHES Clang)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG")
    endif()
elseif(CMAKE_BUILD_TYPE MATCHES Profiling)
    # For the profiling build, we enable optimizations and supply the required profiler flags.
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /O2 /DEBUG:FULL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -g3 -Wall -Wpedantic -Wsign-compare -Wnarrowing -Wmissing-field-initializers -Wunused-macros")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")
    endif()
else()
    # Enable the C++17 standard and enable optimizations as it's a release build.
    # This will also disable all assert() macros. Strip the binary too.
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG /std:c++17 /O2 /DEBUG:NONE")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -DNDEBUG -Wall -Wpedantic -Wsign-compare -Wnarrowing -Wmissing-field-initializers -Wunused-macros")
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")
        else()
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2 -s")
        endif()
    endif()
endif()

# Silence some annoying warnings caused by invalid characters in some FreeImage source comments.
if(CMAKE_CXX_COMPILER_ID MATCHES Clang AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 15.0.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-utf8")
endif()

if(ANDROID)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -llog")
endif()

if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s INITIAL_MEMORY=33554432 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=1 -s USE_SDL=2 \
                                -pthread -s PTHREAD_POOL_SIZE=4 -s DEMANGLE_SUPPORT=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s ERROR_ON_UNDEFINED_SYMBOLS=1 -s ASYNCIFY \
                                --preload-file ${PROJECT_SOURCE_DIR}/resources@/home/web_user/ES-DE/resources/ \
                                --preload-file ${PROJECT_SOURCE_DIR}/themes/slate-es-de@/home/web_user/ES-DE/themes/slate-es-de/ \
                                --preload-file ${PROJECT_SOURCE_DIR}/ROMs@/home/web_user/ROMs/")
endif()

# Raspberry Pi model 3 and higher (ARM Cortex-A53 minimum).
if(RPI_32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -latomic -mcpu=cortex-a53 -mfpu=neon-fp-armv8")
elseif(RPI_64)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -latomic -mcpu=cortex-a53")
endif()

if(ASAN AND TSAN)
    message(FATAL_ERROR "-- AddressSanitizer and ThreadSanitizer can't be combined")
endif()

if(ASAN)
    message("-- Building with AddressSanitizer")
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    endif()
endif()

if(TSAN)
    message("-- Building with ThreadSanitizer")
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        message(FATAL_ERROR "-- ThreadSanitizer not available for MSVC")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")
    endif()
endif()

if(UBSAN)
    message("-- Building with UndefinedBehaviorSanitizer")
    if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
        message(FATAL_ERROR "-- UndefinedBehaviorSanitizer not available for MSVC")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
    endif()
endif()

if(ASAN OR UBSAN)
    # Add some extra checks when building with AddressSanitizer or UndefinedBehaviorSanitizer.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS -D_FORTIFY_SOURCE=3")
endif()

# The following removes half of the ranlib warnings on macOS regarding no symbols for files
# that are #ifdef'ed away. There must be a way to remove the other half as well?
if(APPLE)
    SET(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

if(APPLE)
    if(MACOS_CODESIGN_IDENTITY)
        message("-- Code signing certificate identity: " ${MACOS_CODESIGN_IDENTITY})
    endif()
endif()

if(ANDROID)
    set(BUNDLED_CERTS ON)
    set(COMPILE_LOCALIZATIONS OFF)
    add_compile_definitions(ANDROID_VERSION_CODE=${ANDROID_VERSION_CODE})
    add_compile_definitions(ANDROID_APPLICATION_ID="org.es_de.frontend")
endif()

if(WIN32)
    set(BUNDLED_CERTS ON)
    add_compile_definitions(UNICODE)
    add_compile_definitions(_UNICODE)
endif()

#---------------------------------------------------------------------------------------------------
# Preprocessor directives.

if(GLES)
    add_compile_definitions(USE_OPENGLES)
    message("-- Building with OpenGL ES renderer")
endif()

if(APPIMAGE_BUILD AND FLATPAK_BUILD)
    message(FATAL_ERROR "-- APPIMAGE_BUILD and FLATPAK_BUILD can't be combined")
endif()

if(APPIMAGE_BUILD)
    if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
        message(FATAL_ERROR "-- APPIMAGE_BUILD can only be used when building on Linux")
    endif()
    add_compile_definitions(APPIMAGE_BUILD)
    message("-- Building as an AppImage")
endif()

if(FLATPAK_BUILD)
    if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
        message(FATAL_ERROR "-- FLATPAK_BUILD can only be used when building on Linux")
    endif()
    add_compile_definitions(FLATPAK_BUILD)
    message("-- Building as a Flatpak")
endif()

if(AUR_BUILD)
    message("-- Building for the AUR")
endif()

if(STEAM_DECK AND RETRODECK)
    message(FATAL_ERROR "-- STEAM_DECK and RETRODECK can't be combined")
endif()

if(STEAM_DECK)
    add_compile_definitions(STEAM_DECK)
    message("-- Building for the Valve Steam Deck")
endif()

if(RETRODECK)
    add_compile_definitions(RETRODECK)
    message("-- Building for RetroDECK")
endif()

if(RPI)
    add_compile_definitions(RASPBERRY_PI)
endif()

if(BUNDLED_CERTS)
    add_compile_definitions(USE_BUNDLED_CERTIFICATES)
    message("-- Building with bundled TLS/SSL certificates")
endif()

if(DEFINED libCEC_FOUND)
    add_compile_definitions(HAVE_LIBCEC)
endif()

if(VIDEO_HW_DECODING)
    add_compile_definitions(VIDEO_HW_DECODING)
    message("-- Building with FFmpeg HW decoding")
endif()

if(DEINIT_ON_LAUNCH)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        add_compile_definitions(DEINIT_ON_LAUNCH)
        message("-- Building with deinitialization on game launch")
    else()
        message(FATAL_ERROR "-- Deinitialization on game launch can only be used on Linux")
    endif()
endif()

if(AUR_BUILD OR FLATPAK_BUILD OR RETRODECK OR RPI OR HAIKU OR CMAKE_SYSTEM_NAME MATCHES FreeBSD)
    set(APPLICATION_UPDATER OFF)
endif()

if(APPLICATION_UPDATER)
    add_compile_definitions(APPLICATION_UPDATER)
else()
    message("-- Building without application updater")
endif()

# This is needed by the application updater to identify the package type.
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    if(LINUX_CPACK_GENERATOR MATCHES DEB)
        add_compile_definitions(LINUX_DEB_PACKAGE)
    elseif(LINUX_CPACK_GENERATOR MATCHES RPM)
        add_compile_definitions(LINUX_RPM_PACKAGE)
    endif()
endif()

if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES arm)
        add_compile_definitions(MACOS_APPLE_CPU)
    else()
        add_compile_definitions(MACOS_INTEL_CPU)
    endif()
endif()

if(ANDROID)
    if(ANDROID_ABI MATCHES arm64-v8a)
        message("-- Building for Android arm64-v8a on API level ${ANDROID_PLATFORM}")
        set(ANDROID_CPU_ARCH arm64-v8a)
    elseif(ANDROID_ABI MATCHES armeabi-v7a)
        message("-- Building for Android armeabi-v7a on API level ${ANDROID_PLATFORM}")
        set(ANDROID_CPU_ARCH armeabi-v7a)
    elseif(ANDROID_ABI MATCHES x86_64)
        message("-- Building for Android x86_64 on API level ${ANDROID_PLATFORM}")
        set(ANDROID_CPU_ARCH x86_64)
    else()
        message(FATAL_ERROR "-- Unsupported Android ABI: " ${ANDROID_ABI})
    endif()
    message("-- Building with Android version code " ${ANDROID_VERSION_CODE})
endif()

# Affects the application updater and is used for displaying version info in the main menu.
if(ES_VERSION MATCHES alpha OR ES_VERSION MATCHES beta OR ES_VERSION MATCHES dev)
    add_compile_definitions(IS_PRERELEASE)
endif()

# GLM library options.
add_compile_definitions(GLM_FORCE_CXX17)
add_compile_definitions(GLM_FORCE_XYZW_ONLY)

# For Unix systems, assign the installation prefix. If it's not explicitly set,
# we use /usr on Linux and /usr/local on FreeBSD.
if(NOT WIN32 AND NOT APPLE AND NOT ANDROID)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        if(CMAKE_SYSTEM_NAME MATCHES Linux)
            set(CMAKE_INSTALL_PREFIX /usr CACHE INTERNAL CMAKE_INSTALL_PREFIX)
        else()
            set(CMAKE_INSTALL_PREFIX /usr/local CACHE INTERNAL CMAKE_INSTALL_PREFIX)
        endif()
    endif()
    message("-- Installation prefix is set to " ${CMAKE_INSTALL_PREFIX})
    add_compile_definitions(ES_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif()

# For Windows, set the minimum OS version to Windows 7.
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    add_compile_definitions(WINVER=0x0601)
endif()

# $$ /end ES-DE





#-------------------------------------------------------------------------------
#set up compiler flags and excutable names
if(DEFINED BCMHOST OR RPI)
    add_definitions(-D_RPI_)
endif()

if(DEFINED VERO4K)
    add_definitions(-D_VERO4K_)
endif()

if(CEC)
  if(DEFINED libCEC_FOUND)
    add_definitions(-DHAVE_LIBCEC)
  endif()
endif()

if(DEFINED SCREENSCRAPER_SOFTNAME)
    add_definitions(-DSCREENSCRAPER_SOFTNAME="${SCREENSCRAPER_SOFTNAME}")
endif()

if(DEFINED SCREENSCRAPER_DEV_LOGIN)
    add_definitions(-DSCREENSCRAPER_DEV_LOGIN="${SCREENSCRAPER_DEV_LOGIN}")
endif()

if(DEFINED GAMESDB_APIKEY)
    add_definitions(-DGAMESDB_APIKEY="${GAMESDB_APIKEY}")
endif()

if(DEFINED CHEEVOS_DEV_LOGIN)
    add_definitions(-DCHEEVOS_DEV_LOGIN="${CHEEVOS_DEV_LOGIN}")
endif()

if(DEFINED HFS_DEV_LOGIN)
    add_definitions(-DHFS_DEV_LOGIN="${HFS_DEV_LOGIN}")
endif()

#-------------------------------------------------------------------------------

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 4.7+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 4.7)
            message(SEND_ERROR "You need at least G++ 4.7 to compile EmulationStation!")
    endif()

    #set up compiler flags for GCC
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-attributes -O0") #support C++14 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-attributes -O3") #support C++14 for std::, optimize
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")  #-s = strip binary
endif()
endif()

if(${GLSYSTEM} MATCHES "Desktop OpenGL")
    add_definitions(-DUSE_OPENGL_21)
    MESSAGE("DEBUG: added -DUSE_OPENGL_21")
elseif(${GLSYSTEM} MATCHES "OpenGL ES 2.0")
    add_definitions(-DUSE_OPENGLES_20)
else()
    add_definitions(-DUSE_OPENGLES_10)
endif()

#-------------------------------------------------------------------------------
#add include directories


# $$ begin ES-DE

#---------------------------------------------------------------------------------------------------
# Include files.

set(COMMON_INCLUDE_DIRS ${CURL_INCLUDE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/CImg
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/lunasvg/include
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson/include
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/rlottie/inc
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/utfcpp/source
                        ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src
                        ${CMAKE_CURRENT_SOURCE_DIR}/es-pdf-converter/src
			# $$ pugi in common!
                        ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src
			)

if(APPLE)
    set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/FFmpeg
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freeimage/FreeImage/Source
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/gettext/gettext-runtime/intl
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/libgit2/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/harfbuzz/src
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/icu/icu4c/source/common
#                            ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src

			    ${RAPIDJSON_INCLUDE_DIRS}

			    # $$ allow <pugixml/src/pugixml.hpp> to resolve
			    ${CMAKE_CURRENT_SOURCE_DIR}/external

#			    ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src

#			    ${SDL2_INCLUDE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL/include

#			    ${SDLMIXER_INCLUDE_DIR}
			    ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_mixer/include

			    ${CMAKE_CURRENT_SOURCE_DIR}/external/vlc/include

                            ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL)
elseif(WIN32)
    set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/FreeImage/Dist/x64
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/gettext/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/harfbuzz/src
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/icu/icu4c/source/common
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/libgit2/include
#                            ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2)
elseif(EMSCRIPTEN)
    set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg.wasm-core
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/FreeImage-CMake/FreeImage/Source
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
#                            ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src
			    )
elseif(ANDROID)
    set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/curl/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg-kit/src/ffmpeg
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freeimage/FreeImage/Source
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/gettext/gettext-runtime/intl
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/harfbuzz/src
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/icu/icu4c/source/common
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/libgit2/include
#                            ${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml/src
                            ${CMAKE_CURRENT_SOURCE_DIR}/external/SDL_Android)
else()
    # $$ merge
    set(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS}
                            ${FFMPEG_INCLUDE_DIRS}
                            ${FreeImage_INCLUDE_DIRS}
                            ${FREETYPE_INCLUDE_DIRS}
                            ${GIT2_INCLUDE_PATH}
                            ${HarfBuzz_INCLUDE_DIRS}
                            ${ICU_INCLUDE_DIRS}
                            ${PUGIXML_INCLUDE_DIRS}
                            ${SDL2_INCLUDE_DIR}
			    # batocera / background musics
			    ${SDLMIXER_INCLUDE_DIR}
			    # batocera / customisations    
			    ${CURL_INCLUDE_DIR}
			    ${VLC_INCLUDE_DIR}
			    ${RAPIDJSON_INCLUDE_DIRS}
			    ${CMAKE_CURRENT_SOURCE_DIR}/external
			    ${CMAKE_CURRENT_SOURCE_DIR}/es-core/src)

endif()
# $$ end ES-DE




if(ENABLE_PULSE)
 LIST(APPEND COMMON_INCLUDE_DIRS
    ${PULSEAUDIO_INCLUDE_DIR}
      )
endif()

if(USE_SYSTEM_PUGIXML)
  MESSAGE("Use system pugixml")
  find_package(pugixml REQUIRED)
endif()

#add libCEC_INCLUDE_DIR
if(CEC)
  if(DEFINED libCEC_FOUND)
    LIST(APPEND COMMON_INCLUDE_DIRS
      ${libCEC_INCLUDE_DIR}
      )
  endif()
endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_INCLUDE_DIRS
        ${ALSA_INCLUDE_DIRS}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_INCLUDE_DIRS
        "${CMAKE_FIND_ROOT_PATH}/opt/vc/include"
        "${CMAKE_FIND_ROOT_PATH}/opt/vc/include/interface/vcos"
        "${CMAKE_FIND_ROOT_PATH}/opt/vc/include/interface/vmcs_host/linux"
        "${CMAKE_FIND_ROOT_PATH}/opt/vc/include/interface/vcos/pthreads"
    )
#add include directory for Vero4K
elseif(DEFINED VERO4K)
    LIST(APPEND COMMON_INCLUDE_DIRS
        "${CMAKE_FIND_ROOT_PATH}/opt/vero3/include"
    )
else()
    if(${GLSYSTEM} MATCHES "Desktop OpenGL")
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGL_INCLUDE_DIR}
        )
	MESSAGE("DEBUG: Added Desktop OpenGL include directories")
    elseif(${GLSYSTEM} MATCHES "OpenGL ES 2.0")
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGLES2_INCLUDE_DIR}
        )
    else()
        LIST(APPEND COMMON_INCLUDE_DIRS
            ${OPENGLES_INCLUDE_DIR}
        )
    endif()
endif()

#-------------------------------------------------------------------------------
#define libraries and directories
if(DEFINED BCMHOST)
    link_directories(
        # batocera / customisations
        "${CMAKE_FIND_ROOT_PATH}/opt/vc/lib"
    )
elseif(DEFINED VERO4K)
    link_directories(
        "${CMAKE_FIND_ROOT_PATH}/opt/vero3/lib"
    )
endif()

# $$ begin ES-DE
#
#---------------------------------------------------------------------------------------------------
# Dependency libraries.

if(APPLE)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES}
                         ${CURL_LIBRARIES}
                         ${PROJECT_SOURCE_DIR}/libavcodec.60.dylib
                         ${PROJECT_SOURCE_DIR}/libavfilter.9.dylib
                         ${PROJECT_SOURCE_DIR}/libavformat.60.dylib
                         ${PROJECT_SOURCE_DIR}/libavutil.58.dylib
                         ${PROJECT_SOURCE_DIR}/libswresample.4.dylib
                         ${PROJECT_SOURCE_DIR}/libswscale.7.dylib
                         ${PROJECT_SOURCE_DIR}/libfreeimage.a
                         ${PROJECT_SOURCE_DIR}/libfreetype.6.dylib
                         ${PROJECT_SOURCE_DIR}/libgit2.1.7.dylib
                         ${PROJECT_SOURCE_DIR}/libharfbuzz.dylib
                         ${PROJECT_SOURCE_DIR}/libicudata.75.dylib
                         ${PROJECT_SOURCE_DIR}/libicui18n.75.dylib
                         ${PROJECT_SOURCE_DIR}/libicuuc.75.dylib
                         ${PROJECT_SOURCE_DIR}/libintl.8.dylib
                         ${PROJECT_SOURCE_DIR}/libpugixml.a
                         ${PROJECT_SOURCE_DIR}/libSDL2-2.0.0.dylib

                         ${PROJECT_SOURCE_DIR}/libSDL2_mixer-2.0.0.dylib
                         ${PROJECT_SOURCE_DIR}/libvlc.12.dylib			
                         ${PROJECT_SOURCE_DIR}/libnanosvg.a
                         ${PROJECT_SOURCE_DIR}/libid3v2.a
                         ${PROJECT_SOURCE_DIR}/liblibcheevos.a
			)
elseif(WIN32)
    set(COMMON_LIBRARIES ${PROJECT_SOURCE_DIR}/avcodec.lib
                            ${PROJECT_SOURCE_DIR}/avfilter.lib
                            ${PROJECT_SOURCE_DIR}/avformat.lib
                            ${PROJECT_SOURCE_DIR}/avutil.lib
                            ${PROJECT_SOURCE_DIR}/swresample.lib
                            ${PROJECT_SOURCE_DIR}/swscale.lib
                            ${PROJECT_SOURCE_DIR}/FreeImage.lib
                            ${PROJECT_SOURCE_DIR}/git2.lib
                            ${PROJECT_SOURCE_DIR}/glew32.lib
                            ${PROJECT_SOURCE_DIR}/harfbuzz.lib
                            ${PROJECT_SOURCE_DIR}/icudt.lib
                            ${PROJECT_SOURCE_DIR}/icuin.lib
                            ${PROJECT_SOURCE_DIR}/icuuc.lib
                            ${PROJECT_SOURCE_DIR}/libcurl-x64.lib
                            ${PROJECT_SOURCE_DIR}/libintl-8.lib
                            ${PROJECT_SOURCE_DIR}/freetype.lib
                            ${PROJECT_SOURCE_DIR}/lunasvg.lib
                            ${PROJECT_SOURCE_DIR}/pugixml.lib
                            ${PROJECT_SOURCE_DIR}/rlottie.lib
                            ${PROJECT_SOURCE_DIR}/SDL2main.lib
                            ${PROJECT_SOURCE_DIR}/SDL2.lib
                            Winmm.dll)
elseif(ANDROID)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES}
                        # FFmpeg libraries.
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libavcodec.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libavfilter.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libavformat.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libavutil.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libswresample.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libswscale.so
                        # Other dependencies.
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libcurl.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libcrypto.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libfreeimage.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libfreetype.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libgit2.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libharfbuzz.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libicudata.a
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libicui18n.a
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libicuuc.a
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libintl.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libjpeg.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libpoppler.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libpugixml.a
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libSDL2.so
                        ${PROJECT_SOURCE_DIR}/android/libs/${ANDROID_CPU_ARCH}/libssl.so)
elseif(EMSCRIPTEN)
    set(COMMON_LIBRARIES ${COMMON_LIBRARIES}
                         # FFmpeg core libraries.
                         ${PROJECT_SOURCE_DIR}/libavcodec.a
                         ${PROJECT_SOURCE_DIR}/libavfilter.a
                         ${PROJECT_SOURCE_DIR}/libavformat.a
                         ${PROJECT_SOURCE_DIR}/libavutil.a
                         ${PROJECT_SOURCE_DIR}/libpostproc.a
                         ${PROJECT_SOURCE_DIR}/libswresample.a
                         ${PROJECT_SOURCE_DIR}/libswscale.a
                         # FFmpeg third party libraries.
                         ${PROJECT_SOURCE_DIR}/libx264.a
                         ${PROJECT_SOURCE_DIR}/libx265.a
                         ${PROJECT_SOURCE_DIR}/libass.a
                         ${PROJECT_SOURCE_DIR}/libfdk-aac.a
                         ${PROJECT_SOURCE_DIR}/libfribidi.a
                         ${PROJECT_SOURCE_DIR}/libharfbuzz.a
                         ${PROJECT_SOURCE_DIR}/libmp3lame.a
                         ${PROJECT_SOURCE_DIR}/libogg.a
                         ${PROJECT_SOURCE_DIR}/libopus.a
                         ${PROJECT_SOURCE_DIR}/libtheoradec.a
                         ${PROJECT_SOURCE_DIR}/libtheoraenc.a
                         ${PROJECT_SOURCE_DIR}/libvorbis.a
                         ${PROJECT_SOURCE_DIR}/libvorbisenc.a
                         ${PROJECT_SOURCE_DIR}/libvorbisfile.a
                         ${PROJECT_SOURCE_DIR}/libvpx.a
                         ${PROJECT_SOURCE_DIR}/libwavpack.a
                         ${PROJECT_SOURCE_DIR}/libz.a
                         # Other dependencies.
                         ${PROJECT_SOURCE_DIR}/libcurl.a
                         ${PROJECT_SOURCE_DIR}/libFreeImage.a
                         ${PROJECT_SOURCE_DIR}/libfreetype.a
                         ${PROJECT_SOURCE_DIR}/libpugixml.a)
else()
    # $$ merge batocera
    set(COMMON_LIBRARIES ${CURL_LIBRARIES}
                         ${FFMPEG_LIBRARIES}
                         ${FreeImage_LIBRARIES}
                         ${FREETYPE_LIBRARIES}
                         ${GIT2_LIBRARY}
                         ${HarfBuzz_LIBRARIES}
                         ${ICU_LIBRARIES}
                         ${PUGIXML_LIBRARIES}
                         ${SDL2_LIBRARY}
			 # batocera / background musics
			 ${SDLMIXER_LIBRARY}
			 # batocera / customisations    
			 ${VLC_LIBRARIES}
			 pugixml
			 nanosvg
			 id3v2
			 libcheevos	)
    if(Intl_LIBRARY)
        # On Unix systems where the gettext functions are not part of the C standard library
        # we need to explicitly link with the libintl library.
        set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${Intl_LIBRARY})
    endif()
endif()

if(NOT WIN32)
    # SVG rendering library LunaSVG and Lottie animation library rlottie.
    if(ANDROID)
        set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${PROJECT_SOURCE_DIR}/android_${ANDROID_ABI}/liblunasvg.a)
        set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${PROJECT_SOURCE_DIR}/android_${ANDROID_ABI}/librlottie.a)
    else()
        set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${PROJECT_SOURCE_DIR}/liblunasvg.a)
        set(COMMON_LIBRARIES ${COMMON_LIBRARIES} ${PROJECT_SOURCE_DIR}/librlottie.a)
    endif()
endif()

if(APPLE)
    # See es-app/CMakeLists.txt for an explation for why an extra "Resources" directory
    # has been added to the install prefix.
    set(CMAKE_INSTALL_PREFIX "/Applications/ES-Batocera.app/Contents/Resources")

    # Set the same rpath links for the install executable as for the build executable.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set(CMAKE_INSTALL_RPATH @executable_path)
endif()

# $$ end ES-DE

if(UDEV_FOUND)
 LIST(APPEND COMMON_LIBRARIES
    ${UDEV_LIBRARY}
      )
endif()

if(ENABLE_PULSE)
 LIST(APPEND COMMON_LIBRARIES
    ${PULSEAUDIO_LIBRARY}
      )
endif()

if(ENABLE_TTS)
 LIST(APPEND COMMON_LIBRARIES
    ${LIBESPEAK_LIBRARIES}
      )
endif()

#add libCEC_LIBRARIES
if(CEC)
  if(DEFINED libCEC_FOUND)
    if(DEFINED BCMHOST)
      LIST(APPEND COMMON_LIBRARIES
        vchiq_arm
	vchostif
	)
    endif()
    # $$ ES-DE default
    LIST(APPEND COMMON_LIBRARIES
      dl
      ${libCEC_LIBRARIES}
      )
  endif()
endif()

#add ALSA for Linux
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    LIST(APPEND COMMON_LIBRARIES
        ${ALSA_LIBRARY}
    )
endif()

if(DEFINED BCMHOST)
    LIST(APPEND COMMON_LIBRARIES
        bcm_host
        brcmEGL
	)
    if(GLES)
      LIST(APPEND COMMON_LIBRARIES ${OPENGLES_LIBRARIES})
    else()
      LIST(APPEND COMMON_LIBRARIES ${OPENGLES2_LIBRARIES})
    endif()
elseif(DEFINED VERO4K)
    LIST(APPEND COMMON_LIBRARIES
        EGL
        ${OPENGLES_LIBRARIES}
    )
else()
    if(MSVC)
        LIST(APPEND COMMON_LIBRARIES
            winmm
        )
    endif()

    # $$ merge w ES-DE


    if(${GLSYSTEM} MATCHES "Desktop OpenGL")
	# $$ ES-DE default
        LIST(APPEND COMMON_LIBRARIES
            ${OPENGL_LIBRARIES}
        )
	MESSAGE("DEBUG: Added Desktop OpenGL libraries")
    elseif(${GLSYSTEM} MATCHES "OpenGL ES 2.0")
        LIST(APPEND COMMON_LIBRARIES
            EGL
            ${OPENGLES2_LIBRARIES}
        )
    elseif(GLES AND ANDROID)
        list(APPEND COMMON_LIBRARIES ${OPENGLES3_LIBRARIES})
    elseif(GLES)
        list(APPEND COMMON_LIBRARIES ${OPENGLES2_LIBRARIES})
    else()
        LIST(APPEND COMMON_LIBRARIES
            EGL
            ${OPENGLES_LIBRARIES}
        )
    endif()

endif()

# $$ commenting out i18n support
#if(NOT WIN32)
#	# batocera / i18n
#	# i18n
#	find_program (MSGFMT_EXECUTABLE msgfmt)
#	find_program (MSGMERGE_EXECUTABLE msgmerge)
#	find_program (XGETTEXT_EXECUTABLE xgettext)
#	find_package(Intl)
#
#	if(MSGFMT_EXECUTABLE AND MSGMERGE_EXECUTABLE AND XGETTEXT_EXECUTABLE AND Intl_FOUND)
#	  message (STATUS "Native language support enabled.")
#	  add_definitions(-DHAVE_INTL)
#	else()
#	  message (STATUS "Native language support disabled.")
#	endif()
#endif()

#-------------------------------------------------------------------------------
# set up build directories

if(WIN32)		
	# $$ no subdir in ES-DE Win32 build
	set(dir ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_GENERATOR_PLATFORM})
else()
	set(dir ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

# $$ ES-DE
if(COMPILE_LOCALIZATIONS)
    MESSAGE("Compiling localizations")
    add_subdirectory(locale)
endif()

if(NOT HAIKU)
    add_subdirectory(es-pdf-converter)
endif()
# $$ end ES-DE

#-------------------------------------------------------------------------------
# add each component

add_subdirectory("external")
add_subdirectory("es-core")
add_subdirectory("es-app")

# $$ ES-DE
# Make sure that es-pdf-convert is built first, and then that rlottie is built before es-core.
# Also set lottie2gif to not be built.
if(NOT HAIKU)
    add_dependencies(lunasvg es-pdf-convert)
endif()

if(COMPILE_LOCALIZATIONS)
    if(NOT HAIKU)
	# $$$ can't figure out where localization target is defined
        add_dependencies(es-pdf-convert localization)
    else()
        add_dependencies(lunasvg localization)
    endif()
endif()

add_dependencies(es-core rlottie)
set_target_properties(lottie2gif PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
# $$ end ES-DE

if(MSGFMT_EXECUTABLE AND MSGMERGE_EXECUTABLE AND XGETTEXT_EXECUTABLE AND Intl_FOUND)
  add_subdirectory (locale)
endif()

