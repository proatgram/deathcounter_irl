# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(deathcounter_irlTests LANGUAGES CXX)

# ---- Tests ----

add_executable(deathcounter_irl_test source/deathcounter_irl_test.cpp)
target_link_libraries(deathcounter_irl_test PRIVATE deathcounter_irl_lib)
target_compile_features(deathcounter_irl_test PRIVATE cxx_std_17)

add_test(NAME deathcounter_irl_test COMMAND deathcounter_irl_test)

# ---- End-of-file commands ----

add_folders(Test)
