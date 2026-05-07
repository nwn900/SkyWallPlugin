cmake_minimum_required(VERSION 3.26)
project(WallWalkSKSE VERSION 0.1.0 LANGUAGES CXX)

set(SOURCES
    src/main.cpp
    src/Core/Settings.cpp
    src/Core/Service.cpp
    src/Hooks/InputHandler.cpp
    src/Physics/SurfaceScanner.cpp
    src/Physics/LocomotionController.cpp
    src/Debug/DebugDraw.cpp
)

find_package(CommonLibSSE REQUIRED)

add_commonlibsse_plugin(${PROJECT_NAME}
    SOURCES ${SOURCES}
    NAME "WallWalkSKSE"
    AUTHOR "nwn900"
    DESCRIPTION "Wall and ceiling walking locomotion plugin for Skyrim SE/AE"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE include)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION SKSE/Plugins
    LIBRARY DESTINATION SKSE/Plugins
)
