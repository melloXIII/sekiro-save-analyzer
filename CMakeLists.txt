cmake_minimum_required(VERSION 3.14)
project(SekiroShadowsReadTwice)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    # Using FetchContent to pull SDL2 directly if on Windows! This avoids all dependency hell!
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.0
    )
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2-static)
    set(SDL2_INCLUDE_DIRS "") # handled by target_link_libraries for SDL2 target
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
endif()
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

if(WIN32)
    set(WIN_RESOURCES icon.rc)
endif()

add_executable(ShadowsReadTwice 
    src/main.cpp 
    src/analyzer.cpp 
    ${IMGUI_SOURCES}
    ${WIN_RESOURCES}
)

target_include_directories(ShadowsReadTwice PRIVATE 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

if(WIN32)
    target_link_libraries(ShadowsReadTwice PRIVATE 
        ${SDL2_LIBRARIES} 
        opengl32
        setupapi
        winmm
        version
        imm32
    )
else()
    target_link_libraries(ShadowsReadTwice PRIVATE 
        ${SDL2_LIBRARIES} 
        ${OPENGL_LIBRARIES} 
        dl
    )
endif()

# Copia le cartelle assets e data nella cartella di build automaticamente
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# --- RPM & PACKAGING ---
install(TARGETS ShadowsReadTwice DESTINATION /opt/ShadowsReadTwice)
install(DIRECTORY assets data DESTINATION /opt/ShadowsReadTwice)
install(FILES shadowsreadtwice.desktop DESTINATION /usr/share/applications)
install(FILES assets/sekirologo.png DESTINATION /usr/share/icons/hicolor/256x256/apps)

set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "sekiro-shadows-read-twice")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "melloXIII")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Save analyzer for Sekiro: Shadows Die Twice")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
include(CPack)
