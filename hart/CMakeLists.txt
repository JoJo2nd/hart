cmake_minimum_required(VERSION 2.8)

# hart include directories
set( HART_INCLUDE_DIRS
    ${BGFX_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FLATBUFFERS_INCLUDE_DIRS}
    ${VECTORMATH_INCLUDE_DIRS}
    ${REMOTERY_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui"
)


#platform headers
if (PLATFORM_WINDOWS)
    set(HART_INCLUDE_DIRS
        "${HART_INCLUDE_DIRS}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/win32"
    )
    file(GLOB_RECURSE HART_PLATFORM_HDR_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/win32/*.h"
    )
    file(GLOB_RECURSE HART_PLATFORM_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/win32/*.cpp"
    )
#    file(GLOB HART_FBS_PLATFORM_FILES
#        "${CMAKE_SOURCE_DIR}/data/assets/hart/fbs/*.fbs"
#    )
#
#    FLATBUFFER_GENERATE_BINDINGS(${HART_FBS_PLATFORM_FILES}, "${CMAKE_CURRENT_SOURCE_DIR}/include/win32/hart/fbs", FBS_INCLUDES)
#    set(FBS_PLATFORM_INCLUDES ${FBS_INCLUDES})
    add_definitions(/WX) # Warnings as errors
endif()

file(GLOB HART_FBS_CMN_FILES
    "${CMAKE_SOURCE_DIR}/data/assets/hart/fbs/*.fbs"
)

FLATBUFFER_GENERATE_BINDINGS("${HART_FBS_CMN_FILES}" "${CMAKE_CURRENT_SOURCE_DIR}/include/hart/fbs" FBS_INCLUDES)
set(FBS_CMN_INCLUDES ${FBS_INCLUDES})

file(GLOB_RECURSE HART_CMN_HDR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(GLOB_RECURSE HART_CMN_SRC_FILES    
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common/*.cpp"
)

set(HART_HDR_FILES
    ${HART_CMN_HDR_FILES}
    ${HART_PLATFORM_HDR_FILES}
    ${FBS_CMN_INCLUDES}
    ${FBS_PLATFORM_INCLUDES}
)
set(HART_SRC_FILES
    "${HART_CMN_SRC_FILES}"
    "${HART_PLATFORM_SRC_FILES}"
    "${HART_FBS_CMN_FILES}"
    "${HART_FBS_PLATFORM_FILES}"
    "${REMOTERY_SRC_FILES}"
)

#source_group(base FILES ${HART_BASE_FILES})

set(HART_LIBRARIES_CMN
    ${SDL2_LIBS}
)
set(HART_LIBRARIES_DEBUG 
    ${BGFX_LIBS_DEBUG}
    ${HART_LIBRARIES_CMN}
)
set(HART_LIBRARIES_RELEASE
    ${BGFX_LIBS_RELEASE}
    ${HART_LIBRARIES_CMN}
)
set(HART_LIB_DIRS
    ${BGFX_LIBS_DIR}
    ${SDL2_LIB_DIRS}
)

include_directories(${HART_INCLUDE_DIRS})
link_directories(${HART_LIB_DIRS})

add_library(hart 
    ${HART_HDR_FILES}
    ${HART_SRC_FILES}
)

target_link_libraries(hart debug ${HART_LIBRARIES_DEBUG})
target_link_libraries(hart optimized ${HART_LIBRARIES_RELEASE})

