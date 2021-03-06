cmake_minimum_required(VERSION 2.8)

# hart include directories
set( INCLUDE_DIRS
    ${BGFX_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FLATBUFFERS_INCLUDE_DIRS}
    ${VECTORMATH_INCLUDE_DIRS}
    ${REMOTERY_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/../hart/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../hart/include/hart/fbs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/imgui"
)

#platform headers
if (PLATFORM_WINDOWS)
    set(INCLUDE_DIRS
        "${INCLUDE_DIRS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/../hart/include/win32"
    )
    file(GLOB_RECURSE HART_PLATFORM_HDR_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../hart/include/win32/*.h"
    )

    add_definitions(/WX) # Warnings as errors
endif()

file(GLOB HART_FBS_CMN_FILES
    "${CMAKE_SOURCE_DIR}/data/assets/game/fbs/*.fbs"
)

FLATBUFFER_GENERATE_BINDINGS("${HART_FBS_CMN_FILES}" "${CMAKE_CURRENT_SOURCE_DIR}/src/fbs" FBS_INCLUDES)
set(FBS_CMN_INCLUDES ${FBS_INCLUDES})

file(GLOB_RECURSE HDR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(LIB_DIRS
    ${BGFX_LIBS_DIR}
    ${SDL2_LIB_DIRS}
)

include_directories(${INCLUDE_DIRS})
link_directories(${LIB_DIRS})
add_executable(game 
    ${HDR_FILES}
    ${FBS_CMN_INCLUDES}
    ${SRC_FILES}
)

target_link_libraries(game debug hart)
target_link_libraries(game optimized hart)

