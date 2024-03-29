#/bin/sh

# :: gcc and make and other dependencies ::
# apt-get install build-essential
# apt-get install bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# :: Download binutils and gcc ::
BINUTILS_VER='2.31.1'
GCC_VER='8.3.0'

wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-8.3.0/gcc-$GCC_VER.tar.xz

# :: Extract ::
tar -xf ./binutils-$BINUTILS_VER.tar.xz
tar -xf ./gcc-$GCC_VER.tar.xz

# :: Building and Installation ::
export PREFIX="$HOME/.local/opt/i686-cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# :: Build and install binutls ::
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot \
                         --disable-nls --disable-werror
make 
make install

# :: Build and install gcc ::
# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH
 
mkdir build-gcc
cd build-gcc
../gcc-$GCC_VER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls \
                          --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# :: Done ::
