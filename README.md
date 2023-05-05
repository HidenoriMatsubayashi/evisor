# eVisor
![Build](https://github.com/HidenoriMatsubayashi/evisor/workflows/Build/badge.svg)

`eVisor` is a light-weight Bare Metal Hypervisor (Type 1) written in C++. This project is aimed for embedded use (ARM64 devices) and educational purposes.

https://user-images.githubusercontent.com/62131389/236361865-c02a18ce-667f-489a-a9b2-cd72a2d21952.mov

## Supported devices

- Raspberry Pi 4 Model B (BCM2711 / aarch64 only)
- QEMU (ARM64)

## TODOs

- Switch to clang/llvm from gcc
- Support multi CPU cores
- Fix temporary implementation of memory management APIs like kmm_malloc
- Add Hypervisor system call APIs like KVM
- Support GIC version 3 and 4
- Improve scheduling algorithm
- Improve mmc device driver (SD data transfer speed)
- Support Linux OS
- Restructuring / refactoring source code

## System requrements

```shell
$ sudo apt install cmake g++-aarch64-linux-gnu
```

## How to build

### Cross-building for ARM64 on x86_64

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64-elf-gcc.cmake \
      -DCMAKE_BUILD_TYPE={Debug|Release} \
      -DBOARD={raspi4|qemu} \
      -DTEST_GUEST={serial|test_app|nuttx|linux}
```

## Examples

### Raspberry Pi4 + NuttX (Guest OS)

#### Building

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64-elf-gcc.cmake \
         -DCMAKE_BUILD_TYPE=Release -DBOARD=raspi4 -DTEST_GUEST=nuttx
cmake --build .
```

#### Running

Setup SD card for Rasberry Pi4 (See config/raspi4/config.txt for the detail settings):
```shell
cp config/raspi4/config.txt <path_to_sdcard>/boot
```

Copy NuttX image file to the SD card:
```shell
cp examples/nuttx/nuttx.bin <path_to_sdcard>/boot
```

Copy eVisor image file to the SD card:
```shell
cp build/kernel.bin <path_to_sdcard>/boot/kernel.bin
```

### QEMU + NuttX (Guest OS)

#### Install QEMU

```shell
sudo apt install qemu-system-arm 
```

#### Building

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/aarch64-elf-gcc.cmake \
         -DCMAKE_BUILD_TYPE=Release -DBOARD=qemu -DTEST_GUEST=nuttx
cmake --build .
```

#### Running

```shell
qemu-system-aarch64 -cpu cortex-a53 -smp 4 -nographic \
  -machine virt,virtualization=on,gic-version=2 -net none \
  -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline \
  -kernel ./kernel.elf -m 1G
```

#### How to debug on QEMU

```shell
qemu-system-aarch64 -cpu cortex-a53 -smp 4 -nographic \
  -machine virt,virtualization=on,gic-version=2 -net none \
  -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline \
  -kernel ./kernel.elf -m 1G \
  -d mmu,in_asm,guest_errors,int,exec,page -D qemu_trace.log
```

## Special thanks!

Special thanks the followings since I particularly referred to them in the early stage of development.

- [raspvisor](https://github.com/matsud224/raspvisor) 
