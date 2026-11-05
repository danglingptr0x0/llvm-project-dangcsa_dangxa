#include "dangcsaISelLowering.h"
#include "dangcsaMachineFunctionInfo.h"
#include "dangcsaRegisterInfo.h"
#include "dangcsaSubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-lower"

static const int64_t dangcsaReturnAddressSlotSize = 8;
static const int64_t dangcsaSlotBytes = 8;

static void dangcsaCanonCond(ISD::CondCode &CCVal, SDValue &LHS, SDValue &RHS) {
  switch (CCVal) {
  case ISD::SETGT:
    CCVal = ISD::SETLT;
    std::swap(LHS, RHS);
    break;
  case ISD::SETLE:
    CCVal = ISD::SETGE;
    std::swap(LHS, RHS);
    break;
  case ISD::SETUGT:
    CCVal = ISD::SETULT;
    std::swap(LHS, RHS);
    break;
  case ISD::SETULE:
    CCVal = ISD::SETUGE;
    std::swap(LHS, RHS);
    break;
  default:
    break;
  }
}

#define GET_CALLING_CONV_IMPL
#include "dangcsaGenCallingConv.inc"

dangcsaTargetLowering::dangcsaTargetLowering(const TargetMachine &TM,
                                             const dangcsaSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(STI) {
  addRegisterClass(MVT::i64, &dangcsa::GPRRegClass);

  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(dangcsa::SP);

  setBooleanContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);
  setOperationAction(ISD::JumpTable, MVT::i64, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);

  setOperationAction(ISD::BR_CC, MVT::i64, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT, MVT::i64, Custom);

  setOperationAction(ISD::TRAP, MVT::Other, Expand);
  setOperationAction(ISD::DEBUGTRAP, MVT::Other, Expand);

  setOperationAction(ISD::SHL_PARTS, MVT::i64, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i64, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i64, Expand);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);

  setOperationAction(ISD::CTPOP, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ, MVT::i64, Legal);
  setOperationAction(ISD::CTLZ, MVT::i64, Legal);
  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::ROTR, MVT::i64, Expand);
  setOperationAction(ISD::BSWAP, MVT::i64, Expand);

  setOperationAction(ISD::MULHS, MVT::i64, Legal);
  setOperationAction(ISD::MULHU, MVT::i64, Legal);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }

  setOperationAction(ISD::FREM, MVT::f32, LibCall);
  setOperationAction(ISD::FREM, MVT::f64, LibCall);

  setMaxDivRemBitWidthSupported(64);
  setMaxLargeFPConvertBitWidthSupported(64);

  setOperationAction(ISD::ADDC, MVT::i64, Legal);
  setOperationAction(ISD::ADDE, MVT::i64, Legal);
  setOperationAction(ISD::SUBC, MVT::i64, Legal);
  setOperationAction(ISD::SUBE, MVT::i64, Legal);

  MaxStoresPerMemcpy = 16;
  MaxStoresPerMemcpyOptSize = 8;
  MaxStoresPerMemset = 16;
  MaxStoresPerMemsetOptSize = 8;
  MaxStoresPerMemmove = 16;
  MaxStoresPerMemmoveOptSize = 8;

  setMinFunctionAlignment(Align(8));
  setPrefFunctionAlignment(Align(8));
}

const char *dangcsaTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case dangcsaISD::RET_GLUE:
    return "dangcsaISD::RET_GLUE";
  case dangcsaISD::CALL:
    return "dangcsaISD::CALL";
  case dangcsaISD::BR_CC:
    return "dangcsaISD::BR_CC";
  case dangcsaISD::ABS:
    return "dangcsaISD::ABS";
  default:
    return nullptr;
  }
}

SDValue dangcsaTargetLowering::LowerOperation(SDValue Op,
                                              SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return lowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return lowerBlockAddress(Op, DAG);
  case ISD::JumpTable:
    return lowerJumpTable(Op, DAG);
  case ISD::BRCOND:
    return lowerBRCOND(Op, DAG);
  case ISD::VASTART:
    return lowerVASTART(Op, DAG);
  case ISD::SELECT:
    return lowerSELECT(Op, DAG);
  case ISD::BR_CC:
    return lowerBR_CC(Op, DAG);
  default:
    report_fatal_error("unimplemented dangcsa operand lowering");
  }
}

SDValue dangcsaTargetLowering::lowerSymbol(SDValue Sym, const SDLoc &DL,
                                           SelectionDAG &DAG) const {
  if (isPositionIndependent())
    return SDValue(DAG.getMachineNode(dangcsa::PCADDR, DL, MVT::i64, Sym), 0);

  return DAG.getNode(dangcsaISD::ABS, DL, MVT::i64, Sym);
}

SDValue dangcsaTargetLowering::lowerGlobalAddress(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc DL(Op);
  GlobalAddressSDNode *Node = cast<GlobalAddressSDNode>(Op);
  SDValue Sym = DAG.getTargetGlobalAddress(
      Node->getGlobal(), DL, MVT::i64, Node->getOffset(), dangcsaII::MO_NONE);

  return lowerSymbol(Sym, DL, DAG);
}

SDValue dangcsaTargetLowering::lowerBlockAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  SDLoc DL(Op);
  BlockAddressSDNode *Node = cast<BlockAddressSDNode>(Op);
  SDValue Sym = DAG.getTargetBlockAddress(Node->getBlockAddress(), MVT::i64, 0,
                                          dangcsaII::MO_NONE);

  return lowerSymbol(Sym, DL, DAG);
}

SDValue dangcsaTargetLowering::lowerJumpTable(SDValue Op,
                                              SelectionDAG &DAG) const {
  SDLoc DL(Op);
  JumpTableSDNode *Node = cast<JumpTableSDNode>(Op);
  SDValue Sym =
      DAG.getTargetJumpTable(Node->getIndex(), MVT::i64, dangcsaII::MO_NONE);

  return lowerSymbol(Sym, DL, DAG);
}

SDValue dangcsaTargetLowering::lowerBRCOND(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  SDValue Cond = Op.getOperand(1);
  SDValue Dest = Op.getOperand(2);

  if (Cond.getOpcode() != ISD::SETCC)
    return SDValue();

  SDValue LHS = Cond.getOperand(0);
  SDValue RHS = Cond.getOperand(1);
  ISD::CondCode CCVal = cast<CondCodeSDNode>(Cond.getOperand(2))->get();

  dangcsaCanonCond(CCVal, LHS, RHS);

  SDValue CC = DAG.getCondCode(CCVal);

  return DAG.getNode(dangcsaISD::BR_CC, DL, MVT::Other, Chain, LHS, RHS, CC,
                     Dest);
}

SDValue dangcsaTargetLowering::lowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CCVal = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);

  dangcsaCanonCond(CCVal, LHS, RHS);

  SDValue CC = DAG.getCondCode(CCVal);

  return DAG.getNode(dangcsaISD::BR_CC, DL, MVT::Other, Chain, LHS, RHS, CC,
                     Dest);
}

SDValue dangcsaTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, IsVarArg ? CC_dangcsa_VarArg : CC_dangcsa);

  for (CCValAssign &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      Register VReg = RegInfo.createVirtualRegister(&dangcsa::GPRRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue Value = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i64);
      InVals.push_back(Value);
      continue;
    }

    int FI = MFI.CreateFixedObject(
        dangcsaSlotBytes, VA.getLocMemOffset() + dangcsaReturnAddressSlotSize,
        true);
    SDValue Addr = DAG.getFrameIndex(FI, MVT::i64);
    SDValue Value = DAG.getLoad(MVT::i64, DL, Chain, Addr,
                                MachinePointerInfo::getFixedStack(MF, FI));
    InVals.push_back(Value);
  }

  if (IsVarArg) {
    dangcsaMachineFunctionInfo *FuncInfo =
        MF.getInfo<dangcsaMachineFunctionInfo>();
    int VaFI = MFI.CreateFixedObject(
        dangcsaSlotBytes, CCInfo.getStackSize() + dangcsaReturnAddressSlotSize,
        true);
    FuncInfo->setVarArgsFrameIndex(VaFI);
  }

  return Chain;
}

SDValue dangcsaTargetLowering::lowerSELECT(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Cond = Op.getOperand(0);
  SDValue TrueVal = Op.getOperand(1);
  SDValue FalseVal = Op.getOperand(2);

  SDValue CondWide = DAG.getZExtOrTrunc(Cond, DL, MVT::i64);
  SDValue Zero = DAG.getConstant(0, DL, MVT::i64);
  SDValue Mask = DAG.getNode(ISD::SUB, DL, MVT::i64, Zero, CondWide);
  SDValue AntiMask = DAG.getNOT(DL, Mask, MVT::i64);

  SDValue TruePart = DAG.getNode(ISD::AND, DL, MVT::i64, TrueVal, Mask);
  SDValue FalsePart = DAG.getNode(ISD::AND, DL, MVT::i64, FalseVal, AntiMask);

  return DAG.getNode(ISD::OR, DL, MVT::i64, TruePart, FalsePart);
}

SDValue dangcsaTargetLowering::lowerVASTART(SDValue Op,
                                            SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  dangcsaMachineFunctionInfo *FuncInfo =
      MF.getInfo<dangcsaMachineFunctionInfo>();
  SDLoc DL(Op);

  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(), MVT::i64);
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue
dangcsaTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                 SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;

  CLI.IsTailCall = false;

  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, CLI.IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs,
                             CLI.IsVarArg ? CC_dangcsa_VarArg : CC_dangcsa);

  uint32_t NumBytes = CCInfo.getStackSize();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<Register, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  for (uint32_t i = 0; i < ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
      continue;
    }

    SDValue StackPtr = DAG.getRegister(dangcsa::SP, MVT::i64);
    SDValue Offset = DAG.getIntPtrConstant(VA.getLocMemOffset(), DL);
    SDValue Addr = DAG.getNode(ISD::ADD, DL, MVT::i64, StackPtr, Offset);
    MemOpChains.push_back(
        DAG.getStore(Chain, DL, Arg, Addr, MachinePointerInfo()));
  }

  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

  SDValue Glue;
  for (auto &Pair : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Pair.first, Pair.second, Glue);
    Glue = Chain.getValue(1);
  }

  if (GlobalAddressSDNode *GA = dyn_cast<GlobalAddressSDNode>(Callee)) {
    SDValue Sym = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i64, 0,
                                             dangcsaII::MO_NONE);
    Callee = lowerSymbol(Sym, DL, DAG);
  } else if (ExternalSymbolSDNode *ES =
                 dyn_cast<ExternalSymbolSDNode>(Callee)) {
    if (!ES->getSymbol())
      report_fatal_error("dangcsa has no runtime libcall registered for this "
                         "operation; see RuntimeLibcalls.td");

    SDValue Sym = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i64,
                                              dangcsaII::MO_NONE);
    Callee = lowerSymbol(Sym, DL, DAG);
  }

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  for (auto &Pair : RegsToPass)
    Ops.push_back(DAG.getRegister(Pair.first, MVT::i64));

  const dangcsaRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(MF, CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (Glue.getNode())
    Ops.push_back(Glue);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(dangcsaISD::CALL, DL, NodeTys, Ops);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  SmallVector<CCValAssign, 16> RVLocs;
  CCState RetCCInfo(CallConv, CLI.IsVarArg, MF, RVLocs, *DAG.getContext());
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_dangcsa);

  for (CCValAssign &VA : RVLocs) {
    SDValue Value =
        DAG.getCopyFromReg(Chain, DL, VA.getLocReg(), MVT::i64, Glue);
    Chain = Value.getValue(1);
    Glue = Value.getValue(2);
    InVals.push_back(Value);
  }

  return Chain;
}

SDValue
dangcsaTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                   bool IsVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   const SmallVectorImpl<SDValue> &OutVals,
                                   const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_dangcsa);

  SDValue Glue;
  SmallVector<SDValue, 4> RetOps;
  RetOps.push_back(Chain);

  for (uint32_t i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), MVT::i64));
  }

  RetOps[0] = Chain;

  if (Glue.getNode())
    RetOps.push_back(Glue);

  return DAG.getNode(dangcsaISD::RET_GLUE, DL, MVT::Other, RetOps);
}
