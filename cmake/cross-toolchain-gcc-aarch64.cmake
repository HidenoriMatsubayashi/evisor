set(CMAKE_CROSSCOMPILING TRUE)

# toolchain
set(triple aarch64-linux-gnu)
set(CMAKE_C_COMPILER   ${triple}-gcc)
set(CMAKE_CXX_COMPILER ${triple}-g++)
set(CMAKE_AS           ${triple}-as)
set(CMAKE_OBJCOPY      ${triple}-objcopy)

# TODO: move this to CMakeLists.txt
set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostartfiles -nostdlib -Wl,--build-id=none -Wl,--gc-sections")
