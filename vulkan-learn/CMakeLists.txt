set(target_name vulkan_learn)
project(${target_name})

file(GLOB_RECURSE project_headers CONFIGURE_DEPENDS *.h *.hpp)
file(GLOB_RECURSE project_sources CONFIGURE_DEPENDS *.c *.cpp)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=4)
add_executable(${target_name}
        ${project_headers}
        ${project_sources})

target_compile_options(${target_name} PRIVATE -Wno-unused-variable)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #std::print and std::println (requires linking with -lstdc++exp on Windows). by: https://gcc.gnu.org/gcc-14/changes.html#:~:text=std%3A%3Aprint%20and%20std%3A%3Aprintln%20(requires%20linking%20with%20%2Dlstdc%2B%2Bexp%20on%20Windows).
    target_link_libraries(${target_name} PRIVATE -lstdc++exp )
endif (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

find_package(Vulkan REQUIRED)
target_link_libraries(${target_name} PRIVATE Vulkan::Vulkan)
target_link_libraries(${target_name} PRIVATE Vulkan::Headers)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_compile_definitions(${target_name} PRIVATE MAGIC_ENUM_RANGE_MAX=2048)
target_link_libraries(${target_name} PRIVATE imgui::imgui)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE glm::glm)

message(STATUS "All targets in current directory: ${all_targets}")

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE magic_enum::magic_enum)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE fmt::fmt)

find_package(range-v3 CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE range-v3::range-v3)

