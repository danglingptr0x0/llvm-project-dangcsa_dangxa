#ifndef LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAFIXUPKINDS_H
#define LLVM_LIB_TARGET_DANGCSA_MCTARGETDESC_DANGCSAFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace dangcsa {

enum Fixups {
  fixup_dangcsa_imm32 = FirstTargetFixupKind,
  fixup_dangcsa_pcrel32,
  fixup_dangcsa_branch32,
  fixup_dangcsa_q0,
  fixup_dangcsa_q1,
  fixup_dangcsa_q2,
  fixup_dangcsa_q3,
  fixup_dangcsa_invalid,
  NumTargetFixupKinds = fixup_dangcsa_invalid - FirstTargetFixupKind
};

} // namespace dangcsa
} // namespace llvm

#endif
