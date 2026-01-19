set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Windows)

# 指定编译器
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# C 标志
set(VCPKG_C_FLAGS "-fPIC")

# C++ 标志：移除 -stdlib=libc++，允许使用默认标准库（通常是 libstdc++ 或 MSVC 标准库）
set(VCPKG_CXX_FLAGS "-fPIC")

# 链接器标志
set(VCPKG_LINKER_FLAGS "")
