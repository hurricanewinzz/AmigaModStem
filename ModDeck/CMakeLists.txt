cmake_minimum_required(VERSION 3.22)

project(ModDeck VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# JUCE (fetched automatically)
# ----------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ----------------------------------------------------------------------------
# libopenmpt  (the Amiga .mod playback engine)
#   - macOS/Linux : install via Homebrew/apt, found through pkg-config
#   - Windows     : install via vcpkg, found through find_package(libopenmpt)
# ----------------------------------------------------------------------------
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OPENMPT IMPORTED_TARGET libopenmpt)
endif()

if(OPENMPT_FOUND)
    set(OPENMPT_LINK PkgConfig::OPENMPT)
    message(STATUS "ModDeck: libopenmpt found via pkg-config")
else()
    find_package(libopenmpt CONFIG REQUIRED)
    set(OPENMPT_LINK libopenmpt::libopenmpt)
    message(STATUS "ModDeck: libopenmpt found via find_package (vcpkg)")
endif()

# ----------------------------------------------------------------------------
# The plugin / standalone target
# ----------------------------------------------------------------------------
juce_add_plugin(ModDeck
    COMPANY_NAME            "NeonLabs"
    PLUGIN_MANUFACTURER_CODE Nlab
    PLUGIN_CODE             Mdck
    FORMATS                 Standalone VST3
    PRODUCT_NAME            "ModDeck"
    DESCRIPTION             "Amiga MOD player deck with BPM lock, beatgrid & beatjump"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(ModDeck PRIVATE
    Source/ModEngine.cpp
    Source/FxHost.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/gui/AmigaLookAndFeel.cpp
    Source/gui/BeatGridComponent.cpp
    Source/gui/VuMeter.cpp
    Source/gui/PlayerPage.cpp
    Source/gui/FxPage.cpp)

target_include_directories(ModDeck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${OPENMPT_INCLUDE_DIRS})

target_compile_definitions(ModDeck PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(ModDeck PRIVATE
    ${OPENMPT_LINK}
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
