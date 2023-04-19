cmake_minimum_required(VERSION 3.25)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(psxdev
	LANGUAGES ASM C CXX
	DESCRIPTION "PS1 Development Experiment Repo Thing"
	VERSION 0.1
	)

# Set the default C++ standard for (bad) projects
# which do not use compile features.
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(Policies)
include(Project)

if(NOT PS1)
    message(FATAL_ERROR "you fucked it up")
endif()

add_subdirectory(balls)