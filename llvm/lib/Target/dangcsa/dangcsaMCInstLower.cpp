#include "MCTargetDesc/dangcsaMCAsmInfo.h"
#include "dangcsa.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;

  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset() != 0)
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  switch (MO.getTargetFlags()) {
  case dangcsaII::MO_PCREL:
    Expr = MCSpecifierExpr::create(Expr, dangcsa::S_PCREL, Ctx);
    break;
  case dangcsaII::MO_Q0:
    Expr = MCSpecifierExpr::create(Expr, dangcsa::S_Q0, Ctx);
    break;
  case dangcsaII::MO_Q1:
    Expr = MCSpecifierExpr::create(Expr, dangcsa::S_Q1, Ctx);
    break;
  case dangcsaII::MO_Q2:
    Expr = MCSpecifierExpr::create(Expr, dangcsa::S_Q2, Ctx);
    break;
  case dangcsaII::MO_Q3:
    Expr = MCSpecifierExpr::create(Expr, dangcsa::S_Q3, Ctx);
    break;
  case dangcsaII::MO_NONE:
    break;
  default:
    report_fatal_error("unknown dangcsa operand target flag");
  }

  return MCOperand::createExpr(Expr);
}

MCOperand llvm::lowerdangcsaSymbolOperand(const MachineOperand &MO,
                                          AsmPrinter &AP) {
  switch (MO.getType()) {
  case MachineOperand::MO_GlobalAddress:
    return lowerSymbolOperand(MO, AP.getSymbol(MO.getGlobal()), AP);

  case MachineOperand::MO_ExternalSymbol:
    return lowerSymbolOperand(
        MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);

  case MachineOperand::MO_BlockAddress:
    return lowerSymbolOperand(
        MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);

  case MachineOperand::MO_JumpTableIndex:
    return lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP);

  case MachineOperand::MO_ConstantPoolIndex:
    return lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);

  default:
    report_fatal_error("unsupported dangcsa symbol operand");
  }
}

void llvm::lowerdangcsaMachineInstrToMCInst(const MachineInstr *MI,
                                            MCInst &OutMI, AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    switch (MO.getType()) {
    case MachineOperand::MO_Register:
      if (MO.isImplicit())
        continue;

      OutMI.addOperand(MCOperand::createReg(MO.getReg()));
      break;

    case MachineOperand::MO_Immediate:
      OutMI.addOperand(MCOperand::createImm(MO.getImm()));
      break;

    case MachineOperand::MO_MachineBasicBlock:
      OutMI.addOperand(MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), AP.OutContext)));
      break;

    case MachineOperand::MO_GlobalAddress:
      OutMI.addOperand(
          lowerSymbolOperand(MO, AP.getSymbol(MO.getGlobal()), AP));
      break;

    case MachineOperand::MO_ExternalSymbol:
      OutMI.addOperand(lowerSymbolOperand(
          MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP));
      break;

    case MachineOperand::MO_BlockAddress:
      OutMI.addOperand(lowerSymbolOperand(
          MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP));
      break;

    case MachineOperand::MO_JumpTableIndex:
      OutMI.addOperand(
          lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP));
      break;

    case MachineOperand::MO_ConstantPoolIndex:
      OutMI.addOperand(
          lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP));
      break;

    case MachineOperand::MO_RegisterMask:
      break;

    default:
      report_fatal_error("unknown dangcsa machine operand type");
    }
  }
}
