cmake_minimum_required(VERSION 2.8.12.2)

project(hart)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(FLATBUFFERC_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/data/builder/flatc")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(BUILD_PLATFORM "windows")
  set(PLATFORM_WINDOWS true)
endif()

function(FLATBUFFER_GENERATE_BINDINGS SRCS DEST_FOLDER FBS_INCLUDES)    
  set(LFBS_INCLUDES)
  set(FBS_SRCS "${SRCS}")
  foreach(FIL ${FBS_SRCS})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    set(LFBS_INCLUDES "${LFBS_INCLUDES}" "${DEST_FOLDER}/${FIL_WE}_generated.h")
    set_source_files_properties(${FIL} PROPERTIES HEADER_FILE_ONLY TRUE)

    add_custom_command(
      OUTPUT "${DEST_FOLDER}/${FIL_WE}_generated.h"
      COMMAND ${FLATBUFFERC_EXECUTABLE}
      ARGS -o ${DEST_FOLDER} --cpp ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "flatc ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${LFBS_INCLUDES} PROPERTIES GENERATED TRUE)
  set(FBS_INCLUDES ${LFBS_INCLUDES} PARENT_SCOPE)
endfunction()


set( BGFX_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/bgfx/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/bx/include"
)
set( BGFX_LIBS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/external/bgfx/.build/win64_vs2015/bin"
)
set( BGFX_LIBS_DEBUG
  bgfxDebug
)
set( BGFX_LIBS_RELEASE
  bgfxRelease
)

set( SDL2_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/include"
)
set( SDL2_LIB_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/lib/x64"
)
set( SDL2_LIBS
  SDL2
)

set( FLATBUFFERS_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/flatbuffers/include"
)

set( GETOPT_PORT_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/getopt_port/include"
)

set( MINFS_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/minfs/include"
)

# Or "${CMAKE_CURRENT_SOURCE_DIR}/external/sce_vectormath/include/vectormath/scalar" for debugging
set( VECTORMATH_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/sce_vectormath/include/vectormath/SSE"
)

set( RAPIDJSON_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson/include"
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_ITERATOR_DEBUG_LEVEL=0)
# BGFX uses the static runtime so link to that
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

add_subdirectory ("external/getopt_port")
add_subdirectory ("external/minfs")
add_subdirectory ("hart")
add_subdirectory ("game")