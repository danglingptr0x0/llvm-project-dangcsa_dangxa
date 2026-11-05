#include "dangcsa.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"

using namespace clang;
using namespace clang::targets;

void dangcsaTargetInfo::adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
                               const TargetInfo *Aux) {
  TargetInfo::adjust(Diags, Opts, Aux);
  Opts.MathErrno = false;
}

void dangcsaTargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  Builder.defineMacro("__dangcsa__");
  Builder.defineMacro("__DANGCSA__");
  Builder.defineMacro("__dangcsa64__");
  Builder.defineMacro("__DANGCSA64__");
  Builder.defineMacro("__DLU__");

  if (getTriple().getOS() == llvm::Triple::DANGLING) {
    Builder.defineMacro("__dangling__");
    Builder.defineMacro("__DANGLING__");

    return;
  }

  Builder.defineMacro("__ELF__");
}
