add_subdirectory(fifo EXCLUDE_FROM_ALL)

add_subdirectory(lv2_host)

############################
#  spdlog compile options  #
############################
# Build spdlog as a static library - improves compile times a lot
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
# Disable unused features
set(SPDLOG_NO_THREAD_ID ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_ATOMIC_LEVELS ON CACHE BOOL "" FORCE)

add_subdirectory(spdlog)

##########################
#  VST3SDK and examples  #
##########################
if (${WITH_VST3})
# Exclude VstGui, hosting samples and skip validation
set(SMTG_ADD_VSTGUI OFF CACHE BOOL "" FORCE)
set(SMTG_ADD_VST3_PLUGINS_SAMPLES ON CACHE BOOL "" FORCE)
set(SMTG_ADD_VST3_HOSTING_SAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "" FORCE)

# Don't install example plugins in ~/.vst3, keep them in the build directory
set(SMTG_PLUGIN_TARGET_PATH ${CMAKE_CURRENT_BINARY_DIR}/vst3sdk/VST3 CACHE STRING "" FORCE)
set(SMTG_LINUX ON)

add_subdirectory(vst3sdk)

# Since vstsdk overrides SMTG_ADD_VST3_PLUGINS_SAMPLES if the sdk included as a
# submodule. We need to manually add adelay as it is used by sushi unit tests
# We also need to redo some Steinberg initialisation.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk/cmake/modules")
include(SMTG_VST3_SDK)

smtg_setup_platform_toolset()
set(SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/vst3sdk)
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)
set(VST_SDK TRUE)
set(SDK_IDE_PLUGIN_EXAMPLES_FOLDER FOLDER "PlugInExamples")

add_subdirectory(vst3sdk/public.sdk/samples/vst)

# Suppress warnings to keep our build output clean
set_target_properties(base PROPERTIES COMPILE_FLAGS "-Wno-deprecated -Wno-cpp")
endif()

#####################################################
#  External Projects definitions (non-Cmake based)  #
#####################################################

# Add here custom build steps for third-party libraries
# that need to be linked statically,
# example:
#
# include(ExternalProject)
#
# ExternalProject_Add(static_library
#         SOURCE_DIR ${PROJECT_SOURCE_DIR}/third-party/static_library
#         BUILD_IN_SOURCE 1
#         CONFIGURE_COMMAND ./autogen.sh COMMAND ./configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/install/static_library
#         BUILD_COMMAND make
#         INSTALL_COMMAND make install
# )