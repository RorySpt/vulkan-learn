set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# 指定编译器
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# C 标志：保持干净，只带上位置无关代码标志
set(VCPKG_C_FLAGS "-fPIC")

# 强制要求使用 libc++ 而不是默认的 libstdc++
set(VCPKG_CXX_FLAGS "-stdlib=libc++ -fPIC")

# Clang++ 驱动程序在处理 C++ 文件时会自动根据 CXX_FLAGS 处理链接。
set(VCPKG_LINKER_FLAGS "-stdlib=libc++ -lc++abi -fuse-ld=lld -lanl")
