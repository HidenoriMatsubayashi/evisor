# NuttX artifacts for eVisor's tests

## How to build NuttX

Source code: https://github.com/apache/nuttx
Used SHA (commit id): b8ef55d201fa8d8a8c5a93fe7b7fc428908b335f

See also https://zenn.dev/hidenori3/articles/35c71a2e512027 (Japanese)

### Install build tools

```shell
sudo apt install kconfig-frontends

wget https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz
xz -d gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz
sudo mkdir -p /opt/arm/linaro-toolchain
sudo tar xf gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar -C /opt/arm/linaro-toolchain
export PATH=$PATH:/opt/arm/linaro-toolchain/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf/bin
```

### Sync source tree

```shell
git clone https://github.com/apache/incubator-nuttx.git nuttx
git clone https://github.com/apache/incubator-nuttx-apps.git apps
cd nuttx
```

### Build nuttx.bin

```shell
./tools/configure.sh -l qemu-armv8a:nsh
```

```shell
make menuconfig

Select GIC version 2
  System Type -> GIC Version
```

```shell
make
```

### Generate ROM file from nuttx.bin

```shell
xxd -i nuttx.bin > qemu_rom.c
```
