# Native (non-Windows) port scaffold for the Gurumin decompilation.
#
#   cmake -S port -B build/port -G Ninja
#   cmake --build build/port
#   ctest --test-dir build/port --output-on-failure
#
# gurumin_core compiles matched files from decomp/src UNCHANGED, with port/compat/gurumin_compat.h force-included
# and the shared headers from decomp/include. See port/README.md for how files join the list.
cmake_minimum_required(VERSION 3.16)
project(gurumin_port LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

get_filename_component(GURUMIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
set(GURUMIN_DECOMP "${GURUMIN_ROOT}/decomp")

# Matched decomp files that compile unchanged and are semantically safe on 64-bit little-endian hosts.
# A file joins when (1) it has matched functions in progress/jp/report.json, (2) it compiles here without edits,
# (3) it does not depend on 32-bit pointers, absolute game addresses, game data globals, inline asm, Win32 or D3D,
# and (4) everything it calls links (from this list or from a stand-in in compat/standins.cpp).
set(GURUMIN_CORE_DECOMP_FILES
  # maths: vectors, matrices, quaternions, planes, overlap tests
  vec4.cpp
  vec_math.cpp
  vec_ops.cpp
  point_transform.cpp
  box_overlap.cpp
  math_float.cpp
  # containers: vertex packs, index lists, key-frame tracks and banks, motion list
  containers.cpp
  vertex.cpp
  # formats and text
  rle_decode.cpp
  text_fullwidth.cpp
  ability_text.cpp
  file_check.cpp
  # small state objects
  input_state.cpp
  flag_list.cpp
)
list(TRANSFORM GURUMIN_CORE_DECOMP_FILES PREPEND "${GURUMIN_DECOMP}/src/")

add_library(gurumin_core STATIC
  ${GURUMIN_CORE_DECOMP_FILES}
  compat/engine_memory.cpp
  compat/standins.cpp
)
target_include_directories(gurumin_core
  PUBLIC "${GURUMIN_DECOMP}/include"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/compat")

if(MSVC)
  target_compile_options(gurumin_core PRIVATE
    "/FI${CMAKE_CURRENT_SOURCE_DIR}/compat/gurumin_compat.h" /fp:precise /W3)
  target_compile_definitions(gurumin_core PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(gurumin_core PRIVATE
    -include "${CMAKE_CURRENT_SOURCE_DIR}/compat/gurumin_compat.h")
  # Public: everything that includes the decomp headers must agree with how the library was built.
  target_compile_options(gurumin_core PUBLIC
    -fsigned-char          # MSVC's char is signed; aarch64's default is unsigned
    -fno-strict-aliasing   # the decomp reads words through char pointers (rle_decode, text_fullwidth)
    -ffp-contract=off)     # no fused multiply-add, so x86_64 and aarch64 give the same float results
  target_compile_options(gurumin_core PRIVATE
    -Wall
    -Werror=int-to-pointer-cast   # (a pointer-to-int cast that loses bits is already an error in C++)
    -Werror=return-type
    # Style the decomp keeps on purpose to match the original bytes; not hazards.
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-sign-compare
    -Wno-parentheses
    -Wno-misleading-indentation
    -Wno-unused-function)
endif()

# MSVC extension the decomp relies on: taking the address of a temporary, e.g. Vec3_LengthSquared(&(a - *this)) in
# vec4.cpp. The temporary lives to the end of the full expression, so the code is correct; gcc and clang only need
# to be told to accept it, for this file alone.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_source_files_properties("${GURUMIN_DECOMP}/src/vec4.cpp" PROPERTIES COMPILE_OPTIONS "-fpermissive")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set_source_files_properties("${GURUMIN_DECOMP}/src/vec4.cpp" PROPERTIES COMPILE_OPTIONS "-Wno-address-of-temporary")
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(gurumin_core_tests tests/core_tests.cpp)
  target_link_libraries(gurumin_core_tests PRIVATE gurumin_core)
  add_test(NAME gurumin_core_tests COMMAND gurumin_core_tests)
endif()

# A small reader for game file formats documented in docs/ (port code, not decomp code).
add_library(gurumin_formats STATIC formats/itm.cpp formats/game_folder.cpp)
target_include_directories(gurumin_formats PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/formats")
if(BUILD_TESTING)
  add_executable(gurumin_formats_tests tests/formats_tests.cpp)
  target_link_libraries(gurumin_formats_tests PRIVATE gurumin_formats)
  add_test(NAME gurumin_formats_tests COMMAND gurumin_formats_tests)
endif()

# The native shell. SDL2 is optional: without it the shell is not built.
option(GURUMIN_PORT_APP "Build the gurumin_port shell (needs SDL2)" ON)
if(GURUMIN_PORT_APP)
  find_package(SDL2 QUIET CONFIG)
  if(NOT SDL2_FOUND)
    find_package(SDL2 QUIET)
  endif()
  if(SDL2_FOUND)
    add_executable(gurumin_port app/main.cpp)
    target_link_libraries(gurumin_port PRIVATE gurumin_core gurumin_formats)
    if(TARGET SDL2::SDL2)
      target_link_libraries(gurumin_port PRIVATE SDL2::SDL2)
    else()
      target_include_directories(gurumin_port PRIVATE ${SDL2_INCLUDE_DIRS})
      target_link_libraries(gurumin_port PRIVATE ${SDL2_LIBRARIES})
    endif()
    if(BUILD_TESTING)
      add_test(NAME gurumin_port_help COMMAND gurumin_port --help)
      add_test(NAME gurumin_port_probe COMMAND gurumin_port --probe)
    endif()
  else()
    message(STATUS "SDL2 not found: gurumin_port (the window shell) will not be built")
  endif()
endif()
