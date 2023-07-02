cmake_minimum_required(VERSION 3.24)

include(FetchContent)

include(cmake/prelude.cmake)

project(
    deathcounter_irl
    VERSION 0.1.0
    DESCRIPTION "A death counter for the unofficial furry_irl discord server"
    HOMEPAGE_URL "github.com/proatgram/deathcounter_irl"
    LANGUAGES CXX
)

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE OPT_IN)
set(FETCHCONTENT_QUIET OFF)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

# DPP Discord C++ API Library
fetchcontent_declare(
    dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
    GIT_TAG 63ac3fed8bdd92e3122c6079b944f70fc9546dc2 # Release v10.0.24
    FIND_PACKAGE_ARGS
)

# argparse C++ Argument parsing library
fetchcontent_declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG 997da9255618311d1fcb0135ce86022729d1f1cb # Release v2.9.0
    FIND_PACKAGE_ARGS
)

fetchcontent_makeavailable(dpp argparse)

# ---- Declare executable ----

add_executable(deathcounter_irl_binary source/main.cpp source/bot.cpp)
add_executable(deathcounter_irl::binary ALIAS deathcounter_irl_binary)

configure_file(${CMAKE_SOURCE_DIR}/include/config.hpp.in ${CMAKE_SOURCE_DIR}/include/config.hpp @ONLY)

target_include_directories(deathcounter_irl_binary PUBLIC ${CMAKE_SOURCE_DIR}/include)

set_property(TARGET deathcounter_irl_binary PROPERTY OUTPUT_NAME deathcounter_irl)

target_compile_features(deathcounter_irl_binary PRIVATE cxx_std_17)

target_link_libraries(deathcounter_irl_binary PRIVATE dpp)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT deathcounter_irl_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of deathcounter_irl"
  )
endif()

include(cmake/dev-mode.cmake)
