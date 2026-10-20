# CMake toolchain file: cross-compile Windows x64 from Linux with llvm-mingw
# (https://github.com/mstorsjo/llvm-mingw, UCRT build).
#
#   cmake -S . -B build-win -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/llvm-mingw.cmake \
#     -DLLVM_MINGW_ROOT=/opt/llvm-mingw-<version>-ucrt-ubuntu-22.04-x86_64 \
#     -DPSX_ENABLE_VULKAN=OFF
#
# LLVM_MINGW_ROOT may also come from the environment.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

if(NOT LLVM_MINGW_ROOT)
  set(LLVM_MINGW_ROOT "$ENV{LLVM_MINGW_ROOT}")
endif()
if(NOT LLVM_MINGW_ROOT OR NOT EXISTS "${LLVM_MINGW_ROOT}/bin")
  message(FATAL_ERROR "Set LLVM_MINGW_ROOT (cache or environment) to an llvm-mingw install root")
endif()
# Try-compile projects re-read this file; pass the root through to them.
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES LLVM_MINGW_ROOT)
set(ENV{LLVM_MINGW_ROOT} "${LLVM_MINGW_ROOT}")

set(_t x86_64-w64-mingw32)
set(CMAKE_C_COMPILER   "${LLVM_MINGW_ROOT}/bin/${_t}-clang")
set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/${_t}-clang++")
set(CMAKE_RC_COMPILER  "${LLVM_MINGW_ROOT}/bin/${_t}-windres")
set(CMAKE_AR           "${LLVM_MINGW_ROOT}/bin/${_t}-ar")
set(CMAKE_RANLIB       "${LLVM_MINGW_ROOT}/bin/${_t}-ranlib")
set(CMAKE_STRIP        "${LLVM_MINGW_ROOT}/bin/${_t}-strip")

set(CMAKE_FIND_ROOT_PATH "${LLVM_MINGW_ROOT}/${_t}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
# Never let host pkg-config results leak into the Windows build.
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_LIBDIR} "/nonexistent")
