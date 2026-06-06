cmake_minimum_required(VERSION 3.22)

# ──────────────────────────────────────────────────────────────────────────────
# macOS: deve essere impostato PRIMA di project() per avere effetto
# ──────────────────────────────────────────────────────────────────────────────
if(APPLE)
    # Universal binary: gira nativo su Apple Silicon (M1/M2/M3/M4) e Intel
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    # macOS 11.0 = minimo richiesto da Logic Pro 11 (ultima versione)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
endif()

project(StereoCompressor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Scarica JUCE automaticamente
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(StereoCompressor
    COMPANY_NAME            "MyPlugins"
    BUNDLE_ID               "com.myplugins.stereocompressor"
    PLUGIN_MANUFACTURER_CODE Mypl
    PLUGIN_CODE             Scmp
    FORMATS                 AU VST3
    PRODUCT_NAME            "Stereo Compressor"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    # macOS: categoria AU corretta per dynamics processor
    AU_MAIN_TYPE            kAudioUnitType_Effect
    AU_SANDBOX_SAFE         TRUE
    # Hardened runtime OFF in dev (altrimenti macOS blocca il plugin non firmato)
    HARDENED_RUNTIME_ENABLED FALSE
)

# Genera JuceHeader.h (incluso da PluginProcessor.h e PluginEditor.h).
# Senza questa riga, la compilazione fallisce con:
#   fatal error: 'JuceHeader.h' file not found
juce_generate_juce_header(StereoCompressor)

# Asset grafici dello skin NEMO (immagini embeddate → BinaryData.h)
juce_add_binary_data(NemoAssets SOURCES
    source/assets/nemo_background.jpg
    source/assets/nemo_knob.png
    source/assets/nemo_fader.png
    source/assets/nemo_switch.png
)

target_sources(StereoCompressor PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/LookAndFeel.cpp
)

target_compile_definitions(StereoCompressor PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(StereoCompressor PRIVATE
    NemoAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
