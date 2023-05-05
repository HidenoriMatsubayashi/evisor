# Virtual serial console test app

## How to build

### Rasberry Pi4

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/aarch64-elf-gcc.cmake -DBOARD=raspi4
cmake --build .
```

### QEMU

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/aarch64-elf-gcc.cmake -DBOARD=qemu
cmake --build .
xxd -i serial.bin > ../qemu_rom.c
```
