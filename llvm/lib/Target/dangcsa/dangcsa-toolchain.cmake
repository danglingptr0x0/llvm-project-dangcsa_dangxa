set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR dangcsa64)

set(DLU_ROOT "/home/dangling/git/dangling_logic_unit")
set(DANGCSA_LLVM_BIN "/home/dangling/git/llvm-project_misc/build/bin")
set(DANGCSA_LDG_INC "/home/dangling/git/libdangling/include/dangling/none/none")
set(DANGCSA_LDG_INC_ARCH "/home/dangling/git/libdangling/include/dangling/MISC/none")

set(CMAKE_C_COMPILER "${DANGCSA_LLVM_BIN}/clang")
set(CMAKE_CXX_COMPILER "${DANGCSA_LLVM_BIN}/clang++")
set(CMAKE_ASM_COMPILER "${DANGCSA_LLVM_BIN}/clang")
set(CMAKE_AR "${DANGCSA_LLVM_BIN}/llvm-ar" CACHE FILEPATH "dangcsa ar")
set(CMAKE_RANLIB "${DANGCSA_LLVM_BIN}/llvm-ranlib" CACHE FILEPATH "dangcsa ranlib")

set(DANGCSA_COMMON_FLAGS "--target=dangcsa64 -ffreestanding -nostdlibinc -fno-builtin -fPIC -fno-jump-tables -fno-vectorize -fno-slp-vectorize -fno-stack-protector")
set(DANGCSA_INC_FLAGS "-isystem ${DLU_ROOT}/sysroot/include -isystem ${DANGCSA_LDG_INC} -isystem ${DANGCSA_LDG_INC_ARCH}")

set(CMAKE_C_FLAGS_INIT "${DANGCSA_COMMON_FLAGS} ${DANGCSA_INC_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${DANGCSA_COMMON_FLAGS} ${DANGCSA_INC_FLAGS} -fno-exceptions -fno-rtti")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH "${DLU_ROOT}/sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
