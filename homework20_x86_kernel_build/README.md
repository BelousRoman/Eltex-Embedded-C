# Eltex's academy homework #20 for lecture 48 "Building Linux kernel under X86 system"

Build was performed on the [Linux Kernel](https://github.com/torvalds/linux) v4.15 using working config file from /boot/ directory.

To build a kernel for x86 system the following steps should be made:
1)  Hop onto required kernel version by calling 'git checkout v{your kernel version}'. The version usually specified in the config file's name, i.e.:

        config-4.15.0-142-generic-generic
    The version, as stated in example is 4.15, so 'git checkout v4.15' must be called;
2)  Copy your config file from boot directory, i.e.:

        cp /boot/config-4.15.0-142-generic-generic ./
3)  Rename config file to '.config', this can be done with 'mv' utility:

        mv config-4.15.0-142-generic-generic .config

**NOTE**: To successfully install the Linux Kernel onto the system, target 'bzImage' must be called, alternatively, target 'all' may be called to skip to 7th step.

4)  Build the bare kernel:

        make vmlinux

**NOTE**: During build there may appear problems with config file, that will lead to configuration questions with options like y/n/m, answer arbitrary, in this example, answering *`N`* to all this questions didn't interfered building and installing the kernel.

5)  Build all modules:

        make modules
6)  Compress kernel image:

        make bzImage
7)  Install modules as superuser:

        sudo make modules_install
8) Install kernel as superuser:

        sudo make install

After completing above steps system must be rebooted to start using built kernel by switching to it in boot manager (In VirtualBox Ubuntu boot manager is called by holding *`Shift`* during startup)

Logs during build is provided in [logs](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework20_x86_kernel_build/logs) directory with warning messages, received by particular commands.
