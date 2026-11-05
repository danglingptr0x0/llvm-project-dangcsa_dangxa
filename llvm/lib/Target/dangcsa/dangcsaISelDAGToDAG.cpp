#include "dangcsa.h"
#include "dangcsaISelLowering.h"
#include "dangcsaSubtarget.h"
#include "dangcsaTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-isel"
#define PASS_NAME "dangcsa DAG to DAG pattern instruction selection"

namespace {

class dangcsaDAGToDAGISel : public SelectionDAGISel {
public:
  dangcsaDAGToDAGISel() = delete;

  explicit dangcsaDAGToDAGISel(dangcsaTargetMachine &TM,
                               CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  void Select(SDNode *Node) override;

  bool SelectAddrRegImm(SDValue Addr, SDValue &Base, SDValue &Offset);

#include "dangcsaGenDAGISel.inc"
};

class dangcsaDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;

  explicit dangcsaDAGToDAGISelLegacy(dangcsaTargetMachine &TM,
                                     CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<dangcsaDAGToDAGISel>(TM, OptLevel)) {}
};

} // namespace

char dangcsaDAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(dangcsaDAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

bool dangcsaDAGToDAGISel::SelectAddrRegImm(SDValue Addr, SDValue &Base,
                                           SDValue &Offset) {
  SDLoc DL(Addr);
  MVT VT = Addr.getSimpleValueType();

  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
    Offset = CurDAG->getTargetConstant(0, DL, VT);
    return true;
  }

  if (CurDAG->isBaseWithConstantOffset(Addr)) {
    int64_t Imm = cast<ConstantSDNode>(Addr.getOperand(1))->getSExtValue();

    if (isInt<32>(Imm)) {
      SDValue Op0 = Addr.getOperand(0);

      if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Op0))
        Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
      else
        Base = Op0;

      Offset = CurDAG->getTargetConstant(Imm, DL, VT);
      return true;
    }
  }

  Base = Addr;
  Offset = CurDAG->getTargetConstant(0, DL, VT);
  return true;
}

void dangcsaDAGToDAGISel::Select(SDNode *Node) {
  if (Node->isMachineOpcode()) {
    Node->setNodeId(-1);
    return;
  }

  if (Node->getOpcode() == ISD::FrameIndex) {
    SDLoc DL(Node);
    int FI = cast<FrameIndexSDNode>(Node)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i64);
    SDValue Zero = CurDAG->getTargetConstant(0, DL, MVT::i64);

    ReplaceNode(Node, CurDAG->getMachineNode(dangcsa::FRAMEADDR, DL, MVT::i64,
                                             TFI, Zero));
    return;
  }

  SelectCode(Node);
}

FunctionPass *llvm::createdangcsaISelDag(dangcsaTargetMachine &TM,
                                         CodeGenOptLevel OptLevel) {
  return new dangcsaDAGToDAGISelLegacy(TM, OptLevel);
}
