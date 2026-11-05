#ifndef LLVM_LIB_TARGET_DANGCSA_DANGCSAISELLOWERING_H
#define LLVM_LIB_TARGET_DANGCSA_DANGCSAISELLOWERING_H

#include "dangcsa.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class dangcsaSubtarget;

namespace dangcsaISD {

enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_GLUE,
  CALL,
  BR_CC,
  ABS
};

}

class dangcsaTargetLowering : public TargetLowering {
public:
  explicit dangcsaTargetLowering(const TargetMachine &TM,
                                 const dangcsaSubtarget &STI);

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  bool isCheapToSpeculateCtlz(Type *Ty) const override { return true; }

  bool isCheapToSpeculateCttz(Type *Ty) const override { return true; }

private:
  const dangcsaSubtarget &Subtarget;

  SDValue lowerSymbol(SDValue Sym, const SDLoc &DL, SelectionDAG &DAG) const;
  SDValue lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerJumpTable(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerBRCOND(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;
};

} // namespace llvm

#endif
