# Linux artifacts for eVisor's tests

## How to build Linux image

See also https://zenn.dev/hidenori3/articles/5db71181a33d56 (Japanese)

### Sync source tree

```shell
git clone --depth=1 -b v6.8 https://github.com/torvalds/linux.git
cd linux
```

### Build 

Create `arch/arm64/configs/qemu-busybox-min.config`:

```
CONFIG_SERIAL_AMBA_PL011=y
CONFIG_SERIAL_AMBA_PL011_CONSOLE=y

CONFIG_GPIOLIB=y
CONFIG_GPIO_PL061=y

CONFIG_KEYBOARD_GPIO=y

CONFIG_CMDLINE="console=ttyAMA0 nokaslr rdinit=/sbin/init"
```

```shell
ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- make allnoconfig
ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- make qemu-busybox-min.config
```

```shell
ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- make -j$(nproc)
```

### Generate ROM file from the Image

```shell
xxd -i arch/arm64/boot/Image > qemu_rom.c
```
