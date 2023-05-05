cmake_minimum_required(VERSION 3.10)

set(TARGET "test_app")
set(CMAKE_C_STANDARD 11)
project(${TARGET} LANGUAGES ASM C)

# ==========================================================================
# Build options
# ==========================================================================

# ==========================================================================
# C source files
# ==========================================================================
set(C_SOURCES "src/main.c")

# ==========================================================================
# Assembly source files
# ==========================================================================
set(ASM_SOURCES "src/boot.S")

add_executable(${TARGET} ${ASM_SOURCES} ${C_SOURCES})
target_include_directories(${TARGET} PRIVATE "src")

# ==========================================================================
# Linker script
# ==========================================================================
set(LINKER_SCRIPT "src/linker.ld")

set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
target_link_options(${TARGET} PRIVATE "-T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")
target_link_options(${TARGET} PRIVATE "-static")

# ==========================================================================
# Set compiler options
# ==========================================================================
set(COMPILE_FLAGS "-Wall -nostartfiles -nostdlib -nolibc -nodefaultlibs -fno-builtin -ffreestanding -mstrict-align")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${COMPILE_FLAGS} -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILE_FLAGS} -DDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS} -DNDEBUG")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

# ==========================================================================
# Custom commands after builds
# ==========================================================================
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
