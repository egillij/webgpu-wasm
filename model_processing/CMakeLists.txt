
cmake_minimum_required(VERSION 3.21)
project(model_maker)
# add_subdirectory(../3pp/glm)
#Set binary directory

SET(SOURCES 
	model_maker.cpp
)

add_executable(model_maker ${SOURCES})

# target_link_libraries(model_maker PUBLIC glm)
target_include_directories(model_maker PUBLIC ../3pp/glm/ ../3pp/ ../utils)