# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hello_world)
# Add your components directories here
set(COMPONENTS_DIR components)
# set(EXTRA_COMPONENT_DIRS
#     ${COMPONENTS_DIR}/esp_idf_lib_helpers
#     ${COMPONENTS_DIR}/i2cdev
#     ${COMPONENTS_DIR}/ds1307
# )
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/components)