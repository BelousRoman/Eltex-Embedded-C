# Eltex's academy homework #22 for lecture 51 "Cross compiling for ARM system"

Ubuntu 22.04.5 OS was used for cross compiling following utilities:
1) [SSH](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework22_cross_compilation/openssh)

To run a QEMU emulator I've used [files from previous homework](https://github.com/BelousRoman/Eltex-Embedded-C/tree/main/homework21_arm_build/qemu_boot_on_Ubuntu_22) with the exception of *initramfs.cpio.gz*

The commands execution order will be described in corresponding subdirectories since for cross compiling a single package it takes individual course of action e.g. cross compiling dependencies like libraries or providing package-specific arguments for configuration etc, so it should be described individually. But general tip about cross compiling would be to read README and INSTALL documents first.

Typically, every package will have configure executable file, whether self-written or automatically generated with autoconf util. Self-written configure file can be accessed right after downloading the package, while other must be generated, which is usually done by a script or call to autoconf util (ought be specified in INSTALL or README documents).
