# eVisor
![Build](https://github.com/HidenoriMatsubayashi/evisor/workflows/Build/badge.svg)

## Book (Japanese)

Step-1 for [ゼロからのハイパーバイザ自作入門](https://zenn.dev/hidenori3/books/55ce98070299db).

## How to build

### Cross-building for ARM64 on x86_64

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/cross-toolchain-clang-aarch64.cmake \
      -DBOARD={raspi4|qemu}
```

### Self-building for ARM64 on ARM64

```shell
mkdir build && cd build
cmake -DBOARD={raspi4|qemu}
```

## How to run on QEMU

```shell
qemu-system-aarch64 \
  -machine virt,virtualization=on,gic-version=2 \
  -cpu cortex-a72 -smp 4 \
  -m 4G \
  -nographic -net none \
  -chardev stdio,id=con,mux=on -serial chardev:con -mon chardev=con,mode=readline \
  -kernel ./kernel.elf
```
