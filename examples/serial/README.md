# Virtual serial console test app

## How to build

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/cross-toolchain-clang-aarch64.cmake -DBOARD={raspi4|qemu}
cmake --build .
```
