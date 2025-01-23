# A [OpenSSH](git@github.com:openssh/openssh-portable.git) cross-platform build for ARM system

[openssh-portable V9_9](https://github.com/openssh/openssh-portable/tree/V_9_9) was chosen for cross compilation, the dependencies, whose was used to build binaries, is:
1) [OpenSSL 3.4.0](https://github.com/openssl/openssl/releases/tag/openssl-3.4.0)
2) [Zlib 1.3.1](https://zlib.net/)

To compile OpenSSH for ARM platform there must be pre-compiled libraries - libcrypto and libz.

Compiling Zlib of version 1.3.1:
1) Zlib has self-written configure executable, so no calls to autoconf or simillar scripts required;
2) Configure makefile for ARM build:

        CROSS_PREFIX=arm-linux-gnueabihf- ./configure --prefix=$PWD/_install
3) Build lib:

        make
4) Install lib:

        make install

Compiling OpenSSL of version 3.4.0:
1) OpenSSL also has self-written configure executable and it has no --help parameters, so for details on build see documents e.g. INSTALL.md
2) Configure makefile for ARM build:

        ./Configure gcc --prefix=$PWD/_install --cross-compile-prefix=arm-linux-gnueabihf-
3) Build lib:

        make
4) Install lib:

        make install

Compiling OpenSSH from branch V9_9:
1) OpenSSH's configure executable is created from configure.ac by calling autoconf util:

       autoconf
2) Configure makefile for ARM build with disabled cstrip to avoid errors during `make install`:

        ./configure --host=arm-linux-gnueabihf --prefix=$PWD/_install --with-zlib=/home/roman/hw22/zlib-1.3.1/_install/ -with-ssl-dir=/home/roman/hw22/openssl-3.4.0/_install --disable-strip
3) Build fiies:

        make
**OPTIONAL**: Remove 'host-key' and 'check-config' targets from `Makefile` at line 391 (in target 'install') to avoid error messages at make install, although all files will be installed;

4) Install files:

        make install

Then copy ssh and sshd executables to /bin/ and /sbin/ directories of filesystem, libz.so.1 and libz.so.1.3.1 to /lib/ directory.

Compress changed filesystem and run emulator.

**NOTE**: although openssh was compiled successfully, call of ssh or sshd on emulated system still gives error because of missing file '/dev/null'
