install(
    TARGETS deathcounter_irl_binary
    RUNTIME COMPONENT deathcounter_irl_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
