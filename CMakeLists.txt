cmake_minimum_required(VERSION 3.10) 

# message(STATUS "foo include dir: ${HEADER_LIST}")

# set the project name 
project(CuteEngine)

# set character set to unicode
ADD_DEFINITIONS(-DUNICODE)

# preset common directory path
set(CODE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)
set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Libraries)

# get all header / source files
file (GLOB_RECURSE HEADER_LIST "Source/*.h")
file (GLOB_RECURSE SOURCE_LIST "Source/*.cpp")

# add executable
add_executable(CuteEngine ${SOURCE_LIST} ${HEADER_LIST})

# group source / header files
source_group(TREE ${CODE_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_LIST})
source_group(TREE ${CODE_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADER_LIST})


# additional include directories
set(
    ADDITIONAL_INCLUDE_DIR
    ${CODE_SOURCE_DIR}
    ${LIBRARY_DIR}/SPIRV-Reflect
    ${LIBRARY_DIR}/FBX/Include
    ${LIBRARY_DIR}/tinyobjloader
    ${LIBRARY_DIR}/glm
    ${LIBRARY_DIR}/glfw/Include
    ${LIBRARY_DIR}/VulkanSDK/Include
    ${LIBRARY_DIR}/stb-master
    ${LIBRARY_DIR}/boost/Include
)
target_include_directories(CuteEngine PUBLIC ${ADDITIONAL_INCLUDE_DIR})

# additional library directories
set(
    ADDITIONAL_LIB_DIR
    ${LIBRARY_DIR}/boost/Lib
    ${LIBRARY_DIR}/glfw/Lib
    ${LIBRARY_DIR}/VulkanSDK/Lib
    ${LIBRARY_DIR}/FBX/Lib
)
target_link_directories(CuteEngine PUBLIC ${ADDITIONAL_LIB_DIR})

# addtional dependencies
set(
    ADDITIONAL_DEP_LIB
    libfbxsdk.lib
    glfw3.lib
    vulkan-1.lib
)
target_link_libraries(CuteEngine ${ADDITIONAL_DEP_LIB})

# other feature
target_compile_features(CuteEngine PRIVATE cxx_std_20)
