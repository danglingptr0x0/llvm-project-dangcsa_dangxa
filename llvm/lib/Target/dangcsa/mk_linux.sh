set -e

SRC=${SRC:-/home/dangling/git/llvm-project_misc}
BUILD=${BUILD:-$SRC/build}
HERE=$(cd "$(dirname "$0")" && pwd)

python3 "$HERE/apply_core_patches.py"

cmake -S "$SRC/llvm" -B "$BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DLLVM_TARGETS_TO_BUILD="X86;Sparc" \
    -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=dangcsa \
    -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_DOCS=OFF

cmake --build "$BUILD" -j "$(nproc)" --target \
    clang lld llvm-mc llc llvm-objdump llvm-objcopy llvm-nm llvm-readobj llvm-ar \
    llvm-symbolizer llvm-dwarfdump llvm-addr2line llvm-reademiru

echo "done: $BUILD/bin"
