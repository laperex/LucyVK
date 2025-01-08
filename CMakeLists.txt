cmake_minimum_required(VERSION 3.0.0)
project(Lucy VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

include(/home/laperex/.local/share/vcpkg/scripts/buildsystems/vcpkg.cmake)
include_directories(include src /home/laperex/.local/share/vcpkg/installed/x64-linux/include)

file(GLOB_RECURSE TARGET_SRC "src/*.cpp" "src/*.c")

add_executable(main ${TARGET_SRC})

# cmake_minimum_required(VERSION 3.7)

include_directories(${Vulkan_INCLUDE_DIRS})
find_package(Vulkan REQUIRED COMPONENTS glslc)
target_link_libraries(main PRIVATE ${Vulkan_LIBRARIES})

find_package(Vulkan) # https://cmake.org/cmake/help/latest/module/FindVulkan.html, CMake 3.21+
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(main PRIVATE Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)

# find_package(SDL2 REQUIRED)
# # include_directories(${SDL2_INCLUDE_DIRS})
# target_link_libraries(main PRIVATE ${SDL2_LIBRARIES})
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(main
	PRIVATE
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt-header-only)
	
find_package(fastgltf CONFIG REQUIRED)
target_link_libraries(main PRIVATE fastgltf::fastgltf)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(main PRIVATE imgui::imgui)

# find_package(imguizmo CONFIG REQUIRED)
# target_link_libraries(main PRIVATE imguizmo::imguizmo)
# find_package(unofficial-imguizmo CONFIG REQUIRED)
# target_link_libraries(main PRIVATE unofficial::imguizmo::imguizmo)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(main PRIVATE assimp::assimp)

find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(main PRIVATE yaml-cpp::yaml-cpp)
# find_package(yaml-cpp CONFIG REQUIRED)
# target_link_libraries(main PRIVATE yaml-cpp)

# find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
# target_include_directories(main PRIVATE ${STB_INCLUDE_DIRS})
find_package(Stb REQUIRED)
target_include_directories(main PRIVATE ${Stb_INCLUDE_DIR})

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(main PRIVATE EnTT::EnTT)

find_package(glm CONFIG REQUIRED)
target_link_libraries(main PRIVATE glm::glm)

find_package(glad CONFIG REQUIRED)
target_link_libraries(main PRIVATE glad::glad)

find_package(stduuid CONFIG REQUIRED)
target_link_libraries(main PRIVATE stduuid)
# find_package(PNG REQUIRED)
# target_link_libraries(main PRIVATE PNG::PNG)
# find_package(OpenCV CONFIG REQUIRED)
set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-linux/share/opencv2")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(main PRIVATE ${OpenCV_LIBS})

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

#==============================================================================
# COMPILE SHADERS
#

set(SHADER_SOURCE_DIR "src/shaders")
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(GLOB_RECURSE SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.glsl
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
