# x64-mingw-static-clang.cmake - Triplet for LLVM-MinGW with UCRT (Clang compiler) based on x64-mingw-static
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_ENV_PASSTHROUGH PATH)

set(VCPKG_CMAKE_SYSTEM_NAME MinGW)

# Use clang/clang++ from LLVM-MinGW distribution
set(VCPKG_C_COMPILER  clang)
set(VCPKG_CXX_COMPILER clang++)

# Set environment variables for compiler detection
set(ENV{CC} clang)
set(ENV{CXX} clang++)

# Flags for position independent code
set(VCPKG_C_FLAGS "-fPIC")
set(VCPKG_CXX_FLAGS "-stdlib=libc++ -fPIC")

# Linker flags
set(VCPKG_LINKER_FLAGS "")

# Note: VCPKG will automatically use MinGW toolchain for this triplet
