#!/bin/bash

# :: Constants ::
BINUTILS_VER='2.31.1'
GCC_VER='8.3.0'

if [ "$1" == "--gen-id" ]; then
    # :: Generate ID (a constant string) to be used in GitHub Actions ::
    # This ID would change when we are targetting a different binutils or GCC
    # version. 

    # The UNIQUE_ID can be used to force change the ID when the rest remains the
    # same.
    UNIQUE_ID="1"
    echo "binutils_${BINUTILS_VER}_gcc_${GCC_VER}-${UNIQUE_ID}"
    exit
elif [ "$1" == "--install-dep" ]; then
    # :: gcc and make and other dependencies ::
    apt-get install -y build-essential                       || exit 1
    apt-get install -y bison flex libgmp3-dev \
                            libmpc-dev libmpfr-dev texinfo   || exit 1
    echo "Dependencies installed. Run script again."
    exit
fi

# :: Download binutils and gcc ::
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz \
     https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz      || exit 1

# :: Extract ::
tar -xf ./binutils-$BINUTILS_VER.tar.xz  || exit 1
tar -xf ./gcc-$GCC_VER.tar.xz            || exit 1

# :: Building and Installation ::
if [ -z "$PREFIX" ]; then
    export PREFIX="$HOME/.local/opt/i686-cross"
fi
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# :: Build and install binutls ::
mkdir build-binutils || exit 1
pushd build-binutils
../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot \
                         --disable-nls --disable-werror || exit 1
make -j4        || exit 1
make install    || exit 1
popd

# :: Build and install gcc ::
# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH
 
mkdir build-gcc || exit 1
pushd build-gcc
../gcc-$GCC_VER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls \
                          --enable-languages=c,c++ --without-headers || exit 1
make all-gcc -j4                     || exit 1
make all-target-libgcc -j4           || exit 1
make install-gcc                  || exit 1
make install-target-libgcc        || exit 1
popd

# :: Cleanup ::
rm -rf build-gcc
rm -rf build-binutils
rm -rf binutils-$BINUTILS_VER*
rm -rf gcc-$GCC_VER*

echo "Installation at $PREFIX complete"
# :: Done ::
