set(target_name vulkan-test)

#set(CMAKE_GENERATOR_TOOLSET "version=14.38.33130")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_executable(${target_name}  ${HEADER_FILES} ${SOURCE_FILES})


# 设置运行时库为 /MDd（动态调试版）
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")

target_include_directories(${target_name} PRIVATE streamer)

find_package(libdatachannel CONFIG REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(${target_name} PRIVATE
         LibDataChannel::LibDataChannel
        #Threads::Threads
)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil)
#
#target_link_libraries(${target_name} PRIVATE PkgConfig::FFmpeg)

find_package(FFMPEG REQUIRED)
target_include_directories(${target_name} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${target_name} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${target_name} PRIVATE ${FFMPEG_LIBRARIES})
if(FFMPEG_FOUND)
    message(STATUS "FFmpeg found:")
    message(STATUS "  Includes: ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${FFMPEG_LIBRARIES}")
    message(STATUS "  Library dirs: ${FFMPEG_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "FFmpeg not found!")
endif()