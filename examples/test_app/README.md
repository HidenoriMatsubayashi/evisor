# test_app

## How to build

```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../../cmake/cross-toolchain-clang-aarch64.cmake
cmake --build .
```
