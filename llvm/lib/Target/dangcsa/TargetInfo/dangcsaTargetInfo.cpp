#include "TargetInfo/dangcsaTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

Target &llvm::getThedangcsa64Target() {
  static Target Thedangcsa64Target;
  return Thedangcsa64Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaTargetInfo() {
  RegisterTarget<Triple::dangcsa64, false> X(
      getThedangcsa64Target(), "dangcsa64",
      "DANGCSA/DANGXA 64-bit (DANGLING Logic Unit)", "dangcsa");
}
