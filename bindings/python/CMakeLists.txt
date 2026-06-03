cmake_minimum_required(VERSION 3.20)
project(fasttextembed_py C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-O3 -ffp-contract=off)

# SIMD features.
#  - Local installs (default): -march=native / -mcpu=native — fastest, targets THIS machine.
#  - Redistributable wheels (FTE_PORTABLE=1): a conservative per-arch SIMD baseline that runs on
#    any CPU of that arch, not just the build host. x86-64 baseline = AVX2+FMA+F16C (Haswell, 2013+);
#    aarch64 baseline = armv8.2-a+fp16 (Graviton2 / Apple M1+ / modern ARM servers).
include(CheckCCompilerFlag)
if(DEFINED ENV{FTE_PORTABLE})
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
        add_compile_options(-mavx2 -mfma -mf16c)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        add_compile_options(-march=armv8.2-a+fp16)
    endif()
else()
    check_c_compiler_flag("-march=native" HAS_MARCH_NATIVE)
    check_c_compiler_flag("-mcpu=native" HAS_MCPU_NATIVE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64" AND HAS_MARCH_NATIVE)
        add_compile_options(-march=native)
    elseif(HAS_MCPU_NATIVE)
        add_compile_options(-mcpu=native)
    endif()
endif()

# Repo layout when building in-tree; vendored ./csrc layout for sdist / isolated builds.
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
if(NOT EXISTS ${ROOT}/src)
    set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/csrc)
endif()
file(GLOB CORE ${ROOT}/src/*.c ${ROOT}/src/kernels/*.c ${ROOT}/src/tokenizer/*.c)

add_library(fte SHARED ${CORE})
target_include_directories(fte PRIVATE ${ROOT}/include ${ROOT}/src)
target_link_libraries(fte m)

# Drop the compiled library into the importable package directory.
install(TARGETS fte LIBRARY DESTINATION fasttextembed RUNTIME DESTINATION fasttextembed)
