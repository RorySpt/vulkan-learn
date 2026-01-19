
add_subdirectory(Vulkan-Headers)

if(NOT TARGET imgui)
    add_library(imgui INTERFACE)
    target_include_directories(imgui INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
            imgui
            imgui/backends
    )

    target_sources(imgui INTERFACE
            imgui/imgui.cpp
            imgui/imgui_demo.cpp
            imgui/imgui_draw.cpp
            imgui/imgui_tables.cpp
            imgui/imgui_widgets.cpp
            imgui/backends/imgui_impl_vulkan.cpp
            imgui/backends/imgui_impl_vulkan.h
            imgui/backends/imgui_impl_glfw.cpp
            imgui/backends/imgui_impl_glfw.h
    )
endif ()
