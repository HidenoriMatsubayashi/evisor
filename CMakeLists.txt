cmake_minimum_required(VERSION 3.10)

# target
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(TARGET "kernel")
project(${TARGET} LANGUAGES ASM C CXX)

# ==========================================================================
# Build options
# ==========================================================================
option(BOARD      "Select target platform: {raspi4 | qemu}" raspi4)
option(TEST_GUEST "Select vCPU test program: {test_app | serial | nuttx | linux}" test_app)

# ==========================================================================
# Source files
# ==========================================================================
add_definitions(-DENABLE_MMU)

if (${TEST_GUEST} STREQUAL "test_app")
  add_definitions(-DTEST_GUEST_IS_TEST_APP)
  elseif(${TEST_GUEST} STREQUAL "serial")
  add_definitions(-DTEST_GUEST_IS_SERIAL)
elseif (${TEST_GUEST} STREQUAL "nuttx")
  add_definitions(-DTEST_GUEST_IS_NUTTX)
else()
  add_definitions(-DTEST_GUEST_IS_LINUX)
endif()

set(SOURCES_BOARD "")
if (${BOARD} STREQUAL "raspi4")
  add_definitions(-DBOARD_IS_RASPI4)
  set(SOURCES_BOARD
    "src/drivers/clkrst/clkrst_bcm2711.cc"
    "src/drivers/gpio/gpio_bcm2711.cc"
    "src/drivers/mailbox/mailbox_bcm2711.cc"
    "src/platforms/bcm2711/board_bcm2711.cc"
    "src/platforms/bcm2711/system_timer.cc"
    )
else()
  add_definitions(-DBOARD_IS_QEMU)
  set(SOURCES_BOARD
    "src/drivers/clkrst/clkrst_bcm2711.cc" # FIXME
    "src/drivers/gpio/gpio_bcm2711.cc" # FIXME
    "src/drivers/mailbox/mailbox_bcm2711.cc" # FIXME
    "src/platforms/qemu/board_qemu.cc"
    "src/platforms/bcm2711/system_timer.cc" # FIXME
  )
endif()
set(SOURCES
  "${SOURCES_BOARD}"
  "src/arch/arm64/arm_generic_timer.cc"
  "src/arch/arm64/boot_el2.cc"
  "src/arch/arm64/cpu_regs.cc"
  "src/arch/arm64/irq/cpu_irq.cc"
  "src/arch/arm64/irq/gic.cc"
  "src/arch/arm64/irq/gic_v2.cc"
  "src/arch/arm64/irq/trap.cc"
  "src/arch/arm64/mmu.cc"
  "src/common/cctype/isdigit.cc"
  "src/common/cstdio/dummy.cc"
  "src/common/cstdio/printf.cc"
  "src/common/cstdio/putc.cc"
  "src/common/cstdio/puts.cc"
  "src/common/cstring/memcpy.cc"
  "src/common/cstring/memset.cc"
  "src/common/cstring/memzero.cc"
  "src/common/cstring/strlen.cc"
  "src/common/cstring/strncmp.cc"
  "src/common/functional/dummy.cc"
  "src/common/queue.cc"
  "src/drivers/common.cc"
  "src/drivers/mmc/mmc.cc"
  "src/drivers/mmc/mmc_card.cc"
  "src/drivers/mmc/mmc_clock.cc"
  "src/drivers/mmc/mmc_irq.cc"
  "src/drivers/uart/pl011_uart.cc"
  "src/kernel/kernel_main.cc"
  "src/kernel/sched/sched_core.cc"
  "src/kernel/sched/sched_create_task.cc"
  "src/kernel/sched/sched_task_context.cc"
  "src/kernel/sched/sched_task_console.cc"
  "src/kernel/sched/sched_virq.cc"
  "src/kernel/vm/vm.cc"
  "src/fs/fat/fat32.cc"
  "src/fs/loader.cc"
  "src/mm/heap/kmm_malloc.cc"
  "src/mm/heap/kmm_zalloc.cc"
  "src/mm/uncached/kmm_uncached_malloc.cc"
  "src/mm/uncached/kmm_uncached_zalloc.cc"
  "src/mm/user_heap/umm_malloc.cc"
  "src/mm/user_heap/umm_zalloc.cc"
  "src/mm/pgtable_stage1.cc"
  "src/mm/pgtable_stage2.cc"
  "src/mm/kmm_trap.cc"
  "src/mm/new.cc"
  "src/platforms/platform.cc"
  "src/platforms/serial.cc"
  "src/platforms/timer.cc"
  "src/platforms/virtio/virtio_pl011_uart.cc"
  "src/platforms/virtio/virtio_gic.cc"
)

# ==========================================================================
# assembly source files
# ==========================================================================
set(ASM_SOURCES
  "src/arch/arm64/boot.S"
  "src/arch/arm64/kernel/sched_vcpu_switch.S"
  "src/arch/arm64/irq/vector_table.S"
  "src/arch/arm64/irq/vectors.S"
  )

add_executable(${TARGET} ${ASM_SOURCES} ${SOURCES})
target_include_directories(${TARGET} PRIVATE "src")

# ==========================================================================
# Linker script
# ==========================================================================
if (${BOARD} STREQUAL "raspi4")
  set(LINKER_SCRIPT "src/arch/arm64/boot/raspberry-pi4/linker.ld")
else()
  set(LINKER_SCRIPT "src/arch/arm64/boot/qemu/cortex-a72/linker.ld")
endif()
set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
target_link_options(${TARGET} PRIVATE "-T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")
target_link_options(${TARGET} PRIVATE "-static")

# ==========================================================================
# Compiler options
# ==========================================================================
set(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS} -DNDEBUG")
set(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} -DDEBUG")

set(COMPILE_FLAGS "-Wall -nostdlib -nodefaultlibs -fno-builtin -ffreestanding -mstrict-align")

# Fix an issue using std::function: undefined reference to `__dso_handle'
set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-use-cxa-atexit")

# Fix undefined reference to `__cxa_guard_acquire'
set(COMPILE_FLAGS "${COMPILE_FLAGS} -fno-threadsafe-statics")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
endif()
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${COMPILE_FLAGS} -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILE_FLAGS} -DDEBUG")

set(CXX_COMMON_FLAGS "-fno-exceptions -fno-unwind-tables -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CXX_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CXX_COMMON_FLAGS}")

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
