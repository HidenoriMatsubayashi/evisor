set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(A64PREFIX              aarch64-linux-gnu)
set(CMAKE_C_COMPILER       ${A64PREFIX}-gcc     CACHE STRING "C compiler")
set(CMAKE_CXX_COMPILER     ${A64PREFIX}-g++     CACHE STRING "C++ compiler")
set(CMAKE_AR               ${A64PREFIX}-ar      CACHE STRING "Arhiver")
set(CMAKE_RANLIB           ${A64PREFIX}-ranlib  CACHE STRING "Ranlib")
set(CMAKE_AS               ${A64PREFIX}-as      CACHE STRING "AS")
set(CMAKE_NM               ${A64PREFIX}-nm      CACHE STRING "NM")
set(CMAKE_OBJDUMP          ${A64PREFIX}-objdump CACHE STRING "Objdump")
set(CMAKE_OBJCOPY          ${A64PREFIX}-objcopy CACHE STRING "Objcopy")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostartfiles -nostdlib -Wl,--build-id=none -Wl,--gc-sections")
