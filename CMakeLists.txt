cmake_minimum_required(VERSION 3.26)
project(WallWalkSKSE VERSION 0.1.0 LANGUAGES CXX)

set(SOURCES
    src/main.cpp
    src/PluginEntry.cpp
    src/Core/Settings.cpp
    src/Core/Service.cpp
    src/Hooks/InputHandler.cpp
    src/Physics/SurfaceScanner.cpp
    src/Physics/LocomotionController.cpp
    src/Physics/MagickaCost.cpp
    src/Animation/AnimationBridge.cpp
    src/Camera/CameraMediator.cpp
    src/Compatibility/PluginDetector.cpp
    src/Debug/DebugDraw.cpp
)

set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
add_subdirectory(extern/CommonLibSSE-NG EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
        "${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG/include"
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /FI"${CMAKE_SOURCE_DIR}/extern/CommonLibSSE-NG/include/SKSE/Impl/PCH.h"
        /utf-8 /bigobj
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION SKSE/Plugins
)
