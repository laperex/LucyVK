cmake_minimum_required(VERSION 3.0.0)
project(Lucy VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

include_directories(include src)
file(GLOB_RECURSE TARGET_SRC "src/*.cpp" "src/*.c")

add_executable(main ${TARGET_SRC})

cmake_minimum_required(VERSION 3.7)

find_package(SDL2 REQUIRED)
# include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(main ${SDL2_LIBRARIES})

find_package(Vulkan REQUIRED COMPONENTS glslc)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(main ${Vulkan_LIBRARIES})

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

#==============================================================================
# COMPILE SHADERS
#

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss)

add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(spv_shaders ALL DEPENDS ${SPV_SHADERS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
