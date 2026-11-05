set -e

SRC=${SRC:-/home/dangling/git/llvm-project_misc}
BUILD=${BUILD:-$SRC/build-win}
NATIVE=${NATIVE:-$SRC/build/bin}
WIN_PREFIX=${WIN_PREFIX:-$HOME/.local/x86_64-w64-mingw32}
HERE=$(cd "$(dirname "$0")" && pwd)

if [ ! -x "$NATIVE/llvm-tblgen" ]; then
    echo "missing native tablegen at $NATIVE; build the linux tree first"
    exit 1
fi

python3 "$HERE/apply_core_patches.py"

cmake -S "$SRC/llvm" -B "$BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DLLVM_TARGETS_TO_BUILD="X86;Sparc" \
    -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=dangcsa \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
    -DCMAKE_FIND_ROOT_PATH="$WIN_PREFIX;/usr/x86_64-w64-mingw32" \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_PREFIX_PATH="$WIN_PREFIX" \
    -DLLVM_NATIVE_TOOL_DIR="$NATIVE" \
    -DLLVM_HOST_TRIPLE=x86_64-w64-windows-gnu \
    -DLLVM_ENABLE_ZLIB=OFF \
    -DLLVM_ENABLE_ZSTD=OFF \
    -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_ENABLE_LIBEDIT=OFF \
    -DLLVM_ENABLE_PLUGINS=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_DOCS=OFF

cmake --build "$BUILD" -j "$(nproc)" --target \
    clang lld llvm-mc llc llvm-objdump llvm-objcopy llvm-nm llvm-readobj llvm-ar \
    llvm-symbolizer llvm-dwarfdump llvm-addr2line llvm-reademiru

echo "done: $BUILD/bin"
