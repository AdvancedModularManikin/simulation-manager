#############################
# CMake Simulation Manager root/src
#############################

set(srcFiles SimManager.cpp SimulationManager.cpp)

add_executable(amm_sim_manager ${srcFiles})

target_link_libraries(amm_sim_manager
        PUBLIC amm_std
        PUBLIC Threads::Threads
        ${Boost_LIBRARIES}
        )

install(TARGETS amm_sim_manager RUNTIME DESTINATION bin)
install(DIRECTORY ../config DESTINATION bin)
