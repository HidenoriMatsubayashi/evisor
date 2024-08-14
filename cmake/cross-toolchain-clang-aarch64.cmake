set(CMAKE_CROSSCOMPILING TRUE)

# toolchain
set(triple aarch64-linux-gnu)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER_TARGET ${triple})
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})

set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostartfiles -nostdlib")
