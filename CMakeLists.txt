cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0077 NEW)

include(FetchContent)

# Because libpqxx make doesn't ship with cmake ;w;
include(FindPkgConfig)

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

set(DPP_USE_EXTERNAL_JSON ON)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

# DPP Discord C++ API Library
fetchcontent_declare(
    dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
    GIT_TAG 472f5231ef0d6394b400b36f62b5f612e583931b # Latest Git commit with new username support
    FIND_PACKAGE_ARGS CONFIG
)

# argparse C++ Argument parsing library
fetchcontent_declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG 997da9255618311d1fcb0135ce86022729d1f1cb # Release v2.9.0
    FIND_PACKAGE_ARGS CONFIG
)

# Check for libpqxx
pkg_check_modules(
    PQXX
    libpqxx
)


# libpq PostgreSQL library (required for libpqxx)
fetchcontent_declare(
    PostgreSQL
    GIT_REPOSITORY https://git.postgresql.org/git/postgresql.git
    GIT_TAG 8382864eb5c9f9ebe962ac20b3392be5ae304d23
    FIND_PACKAGE_ARGS
)

fetchcontent_declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG bc889afb4c5bf1c0d8ee29ef35eaaf4c8bef8a5d
    FIND_PACKAGE_ARGS CONFIG
)

if(NOT PQXX_FOUND)
    # libpqxx C++ PostgreSQL library
    fetchcontent_declare(
        libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG 221ddc8be329bafb376a3d83b9cd257fd52fc7b7 # Release v7.6.0
    )
    fetchcontent_makeavailable(dpp argparse PostgreSQL nlohmann_json libpqxx)
else()
    add_library(libpqxx::libpqxx INTERFACE IMPORTED)
    set_target_properties(libpqxx::libpqxx PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${PQXX_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${PQXX_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${PQXX_CFLAGS}"
        VERSION "${PQXX_VERSION}"
    )
    fetchcontent_makeavailable(dpp argparse PostgreSQL nlohmann_json)
endif()

# ---- Declare executable ----

add_executable(deathcounter_irl_binary source/main.cpp source/bot.cpp)
add_executable(deathcounter_irl::binary ALIAS deathcounter_irl_binary)

configure_file(${CMAKE_SOURCE_DIR}/include/config.hpp.in ${CMAKE_SOURCE_DIR}/include/config.hpp @ONLY)

target_include_directories(deathcounter_irl_binary PUBLIC ${CMAKE_SOURCE_DIR}/include)

set_property(TARGET deathcounter_irl_binary PROPERTY OUTPUT_NAME deathcounter_irl)

target_compile_features(deathcounter_irl_binary PRIVATE cxx_std_20)

target_link_libraries(deathcounter_irl_binary PRIVATE dpp::dpp libpqxx::libpqxx nlohmann_json::nlohmann_json PostgreSQL::PostgreSQL)

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
