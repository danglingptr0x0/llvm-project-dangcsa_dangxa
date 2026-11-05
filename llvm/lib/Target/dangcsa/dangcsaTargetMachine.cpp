#include "dangcsaTargetMachine.h"
#include "TargetInfo/dangcsaTargetInfo.h"
#include "dangcsa.h"
#include "dangcsaMachineFunctionInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"

using namespace llvm;

static StringRef computeDataLayout(const Triple &TT) {
  return "e-m:e-p:64:64-i8:8-i16:16-i32:32-i64:64-i128:128-f32:32-f64:64-n64-"
         "S64";
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::Static);
}

dangcsaTargetMachine::dangcsaTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT), TT, CPU, FS, Options,
                               getEffectiveRelocModel(RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, CPU, FS, *this) {
  initAsmInfo();
}

namespace {

class dangcsaPassConfig : public TargetPassConfig {
public:
  dangcsaPassConfig(dangcsaTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  dangcsaTargetMachine &getdangcsaTargetMachine() const {
    return getTM<dangcsaTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
};

} // namespace

MachineFunctionInfo *dangcsaTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &Fn,
    const TargetSubtargetInfo *STI) const {
  return dangcsaMachineFunctionInfo::create<dangcsaMachineFunctionInfo>(
      Allocator, Fn, STI);
}

void dangcsaPassConfig::addIRPasses() {
  addPass(createdangcsaFloatExpandPass());
  TargetPassConfig::addIRPasses();
}

bool dangcsaPassConfig::addInstSelector() {
  addPass(createdangcsaISelDag(getdangcsaTargetMachine(), getOptLevel()));
  return false;
}

TargetPassConfig *dangcsaTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new dangcsaPassConfig(*this, PM);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaTarget() {
  RegisterTargetMachine<dangcsaTargetMachine> X(getThedangcsa64Target());

  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializedangcsaDAGToDAGISelLegacyPass(PR);
  initializedangcsaFloatExpandPass(PR);
}
