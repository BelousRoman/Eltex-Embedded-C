# Eltex's academy homework #21 for lecture 49-50 "Building Linux kernel for ARM system" and "Building root filesystem for ARM kernel"

Build was performed, using the [Linux Kernel](https://github.com/torvalds/linux) v4.15, [Busybox v1.24.1](https://git.busybox.net/busybox/commit/?h=1_24_stable&id=b03007455c0cba0571bc01d4257606e61d90d88a) and QEMU 2.5.0 under Ubuntu 16.04.7.

To successfully build and run ARM kernel following steps should be made:
First of all, build Linux kernel for ARM system:
1)  Make default config for ARM architecture:

        ARCH=arm make defconfig
2)  Compile zipped Image:

        ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make -j 4 zImage
3)  Compile device tree binaries, if Linux Kernel version is greater than 3.0?

        ARCH=arm make dtbs

Then, build root filesystem with Busybox util:
1)  Make default config for ARM architecture:

        ARCH=arm make defconfig
2)  In ***Settings/Build Options*** of menuconfig: Set *Cross compiler prefix* to 'arm-linux-gnueabihf-' and check *Build BusyBox as a static binary (no shared libs)* if building with static linkage

        ARCH=arm make menuconfig
3)  Build Busybox executable:

        make -j 2
4)  Install Busybox's executable into CONFIG_PREFIX (./_install):

        make install
5) Remove file linuxrc to treat this filesystem as permanent, not temporary:

        rm _install/linuxrc
If building with static linkage, source code must be compiled with static libs:

      arm-linux-gnueabihf-gcc --static init.c -o init
**NOTE**: For proper work, all programs on target system must be built with static libs

Otherwise, for dynamic linkage the following steps must be done:
1)  Create **lib** directory inside **_install** directory:

        mkdir _install/lib
2)  Get list of required libraries for busybox executable, which contains symbolic links to libraries:

        arm-linux-gnueabihf-readelf -d busybox
3)  Copy symbolic links and libraries, which they are pointing to, to newly created **_install/lib** directory

        cp /usr/arm-linux-gnueabihf/lib/libm.so.6 _install/lib/
        cp /usr/arm-linux-gnueabihf/lib/libm-2.23.so _install/lib/
        cp /usr/arm-linux-gnueabihf/lib/libc.so.6 _install/lib/
        cp /usr/arm-linux-gnueabihf/lib/libc-2.23.so _install/lib/
        cp /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3 _install/lib/
        cp /usr/arm-linux-gnueabihf/lib/ld-2.23.so _install/lib/
**NOTE**: For proper work, /lib/ must contain all libraries, needed by every program on target system

After all above steps, depending on type of linkage on a system, filesystem must be composed to an archive.cpio.gz:

        cd _install
        find . | cpio -o -H newc | gzip > init.cpio.gz

Finally, to boot your kernel QEMU must be installed and provided with following arguments:

        sudo apt install qemu
        QEMU_AUDIO_DRV=none qemu-system-arm -M <Your Machine> -kernel <Your zImage> -dtb <Your file.dtb> -initrd <Your filesystem.cpio.gz> -append "console=ttyAMA0 rdinit=/bin/ash" -nographic

**NOTE**: Step with building filesystem with Busybox could be avoided if goal is not to build working system based on build system (i.e. Busybox, Buildroot, Yocto Project), but to do some researches, educational work or create own filesystem from blank
In this case, just simply compile statically linked binary:
        
        arm-linux-gnueabihf-gcc --static init.c -o init
And compose an archive.cpio.gz

        echo init | cpio -o -H newc | gzip > init.cpio.gz
Then pass `init.cpio.gz` to qemu as initrd argument

I've performed 4 various qemu boots, logs can be viewed in this homework's subdirectories:
1)  [Boot with basic filesystem, containing only init process binary](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework21_arm_build/qemu_boot_with_basic_rootfs);
2)  [Boot with filesystem with dynamic linkage](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework21_arm_build/qemu_boot_with_dynamic_linkage);
3)  [Boot with filesystem with dynamic linkage](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework21_arm_build/qemu_boot_with_static_linkage);
4)  [Boot without filesystem](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework21_arm_build/qemu_boot_without_rootfs).
