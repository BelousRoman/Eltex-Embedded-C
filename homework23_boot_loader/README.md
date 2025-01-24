# Eltex's academy homework #23 for lecture 52 "Boot loaders"

Build was performed under Ubuntu 22.04.5 with [U-Boot v2025.01](https://github.com/u-boot/u-boot/releases/tag/v2025.01) for machine vexpress-a9

To successfully build U-boot for ARM system next steps should be followed:
1) Make default config:

        ARCH=arm make vexpress_ca9x4_defconfig
`OPTIONAL: disable 'Use OpenSSL's libcrypto library for host tools' by calling *ARCH=arm make menuconfig* and unchecking fields in 'Tools options`

2) Build files:

        ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make all

   And then start QEMU:

       QEMU_AUDIO_DRV=none qemu-system-arm -M virt -bios u-boot.bin -nographic
