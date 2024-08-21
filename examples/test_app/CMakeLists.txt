cmake_minimum_required(VERSION 3.10)

############################################################################
#
# Toolchain / C++ version / Build target, etc
#
############################################################################

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER       clang  )
set(CMAKE_OBJCOPY     llvm-objcopy)

set(CMAKE_C_STANDARD 11)

set(TARGET "test_app")
project(${TARGET} LANGUAGES ASM C)


############################################################################
#
# Build options
#
############################################################################


############################################################################
#
# Source files
#
############################################################################

set(C_SOURCES "src/main.c")


############################################################################
#
# assembly source files
#
############################################################################

set(ASM_SOURCES "src/boot.S")

add_executable(${TARGET} ${ASM_SOURCES} ${C_SOURCES})
target_include_directories(${TARGET} PRIVATE "src")

############################################################################
#
# Compiler flags
#
############################################################################

set(COMPILE_FLAGS "-Wall -nostdlib -nodefaultlibs -fno-builtin -ffreestanding -mstrict-align")

############################################################################
#
# Linker flags
#
############################################################################

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--build-id=none -Wl,--gc-sections -nostartfiles -nostdlib")
set(LINKER_SCRIPT "src/linker.ld")
set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
target_link_options(${TARGET} PRIVATE "-T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")
target_link_options(${TARGET} PRIVATE "-static")

############################################################################
#
# Custom commands after builds
#
############################################################################

add_custom_command(
   TARGET ${TARGET}
   POST_BUILD
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMAND mv ${TARGET} ${TARGET}.elf
)

add_custom_command(
   TARGET ${TARGET}
   POST_BUILD
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
   COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET}.elf ${TARGET}.bin
)
