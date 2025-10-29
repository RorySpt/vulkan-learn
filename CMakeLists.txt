cmake_minimum_required(VERSION 4.0)
set(target_name vulkan_learn)
project(${target_name})

set(CMAKE_CXX_STANDARD 26)

add_executable(${target_name} main.cpp  )
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #std::print and std::println (requires linking with -lstdc++exp on Windows). by: https://gcc.gnu.org/gcc-14/changes.html#:~:text=std%3A%3Aprint%20and%20std%3A%3Aprintln%20(requires%20linking%20with%20%2Dlstdc%2B%2Bexp%20on%20Windows).
    target_link_libraries(${target_name} PRIVATE -lstdc++exp )
endif (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE glfw)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE imgui::imgui)

find_package(glm CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE glm::glm)

find_package(Vulkan REQUIRED)
target_link_libraries(${target_name} PRIVATE Vulkan::Vulkan)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE magic_enum::magic_enum)

find_package(libdatachannel CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE LibDataChannel::LibDataChannel)

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE magic_enum::magic_enum)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE fmt::fmt)