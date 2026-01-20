# x64-windows-clang-cl.cmake - Triplet for Clang-CL on Windows (MSVC-compatible mode)
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Windows)

# Use clang-cl compiler (Clang in MSVC-compatible mode)
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)

# Set platform toolset to ClangCL for MSBuild compatibility
set(VCPKG_PLATFORM_TOOLSET "ClangCL")

# Clang-cl uses MSVC runtime and standard library, so no libc++ flags needed
# It's compatible with MSVC flags and environment
set(VCPKG_C_FLAGS "")
set(VCPKG_CXX_FLAGS "")
set(VCPKG_LINKER_FLAGS "")
