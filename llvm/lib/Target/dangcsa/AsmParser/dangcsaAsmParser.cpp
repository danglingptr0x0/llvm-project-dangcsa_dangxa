#include "MCTargetDesc/dangcsaMCAsmInfo.h"
#include "MCTargetDesc/dangcsaMCTargetDesc.h"
#include "TargetInfo/dangcsaTargetInfo.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

#define DEBUG_TYPE "dangcsa-asm-parser"

namespace {

class dangcsaOperand : public MCParsedAsmOperand {
  enum KindTy { Token, Register, Immediate } Kind;

  SMLoc StartLoc;
  SMLoc EndLoc;

  StringRef Tok;
  MCRegister Reg;
  const MCExpr *Imm = nullptr;

public:
  dangcsaOperand(KindTy K) : Kind(K) {}

  bool isToken() const override { return Kind == Token; }

  bool isReg() const override { return Kind == Register; }

  bool isImm() const override { return Kind == Immediate; }

  bool isMem() const override { return false; }

  MCRegister getReg() const override {
    assert(Kind == Register && "not a register");
    return Reg;
  }

  const MCExpr *getImm() const {
    assert(Kind == Immediate && "not an immediate");
    return Imm;
  }

  StringRef getToken() const {
    assert(Kind == Token && "not a token");
    return Tok;
  }

  SMLoc getStartLoc() const override { return StartLoc; }

  SMLoc getEndLoc() const override { return EndLoc; }

  bool isUImm16() const {
    if (Kind != Immediate)
      return false;

    const auto *CE = dyn_cast<MCConstantExpr>(Imm);
    if (CE == nullptr)
      return true;

    int64_t V = CE->getValue();
    return V >= 0 && V <= 0xFFFF;
  }

  bool isUImm6() const {
    if (Kind != Immediate)
      return false;

    const auto *CE = dyn_cast<MCConstantExpr>(Imm);
    if (CE == nullptr)
      return false;

    int64_t V = CE->getValue();
    return V >= 0 && V <= 63;
  }

  bool isBranchTarget() const { return Kind == Immediate; }

  bool isSImm32() const {
    if (Kind != Immediate)
      return false;

    const auto *CE = dyn_cast<MCConstantExpr>(Imm);
    if (CE == nullptr)
      return true;

    return isInt<32>(CE->getValue());
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "invalid operand cunt");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "invalid operand cunt");
    const MCExpr *E = getImm();
    if (const auto *CE = dyn_cast<MCConstantExpr>(E))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(E));
  }

  void print(raw_ostream &OS, const MCAsmInfo &MAI) const override {
    switch (Kind) {
    case Token:
      OS << "'" << getToken() << "'";
      break;
    case Register:
      OS << "<reg " << getReg().id() << ">";
      break;
    case Immediate:
      OS << "<imm>";
      break;
    }
  }

  static std::unique_ptr<dangcsaOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<dangcsaOperand>(Token);
    Op->Tok = Str;
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<dangcsaOperand> createReg(MCRegister R, SMLoc S,
                                                   SMLoc E) {
    auto Op = std::make_unique<dangcsaOperand>(Register);
    Op->Reg = R;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<dangcsaOperand> createImm(const MCExpr *Val, SMLoc S,
                                                   SMLoc E) {
    auto Op = std::make_unique<dangcsaOperand>(Immediate);
    Op->Imm = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }
};

class dangcsaAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;

#define GET_ASSEMBLER_HEADER
#include "dangcsaGenAsmMatcher.inc"

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;

  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;

  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool parseOperand(OperandVector &Operands);

  bool isFlagKeyword(StringRef Name) const;

  const MCExpr *parseExprWithSpecifier(SMLoc &EndLoc);

public:
  enum dangcsaMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "dangcsaGenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };

  dangcsaAsmParser(const MCSubtargetInfo &STI, MCAsmParser &P,
                   const MCInstrInfo &MII)
      : MCTargetAsmParser(STI, MII), Parser(P) {
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

} // namespace

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "dangcsaGenAsmMatcher.inc"

bool dangcsaAsmParser::isFlagKeyword(StringRef Name) const {
  return StringSwitch<bool>(Name)
      .Cases({"b", "w", "d", "q"}, true)
      .Cases({"bs", "bu", "ws", "wu", "ds", "du"}, true)
      .Cases({"i", "c", "l", "r", "ra", "t"}, true)
      .Cases({"hu", "hs", "u", "s", "ur", "sr"}, true)
      .Cases({"rel", "al", "eq", "ne", "lt", "ge", "ltu", "geu"}, true)
      .Cases({"q0", "q1", "q2", "q3"}, true)
      .Default(false);
}

ParseStatus dangcsaAsmParser::tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                               SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();

  if (Tok.isNot(AsmToken::Identifier))
    return ParseStatus::NoMatch;

  StringRef Name = Tok.getString();
  Reg = MatchRegisterName(Name);
  if (!Reg)
    Reg = MatchRegisterAltName(Name);

  if (!Reg)
    return ParseStatus::NoMatch;

  getParser().Lex();
  return ParseStatus::Success;
}

bool dangcsaAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                     SMLoc &EndLoc) {
  return !tryParseRegister(Reg, StartLoc, EndLoc).isSuccess();
}

const MCExpr *dangcsaAsmParser::parseExprWithSpecifier(SMLoc &EndLoc) {
  SMLoc Loc = getParser().getTok().getLoc();
  getParser().Lex();

  StringRef Identifier;
  if (getParser().parseIdentifier(Identifier)) {
    Error(Loc, "expected relocation specifier");
    return nullptr;
  }

  dangcsa::Specifier Spec = dangcsa::parseSpecifierName(Identifier);
  if (Spec == dangcsa::S_NONE) {
    Error(Loc, "unknown relocation specifier");
    return nullptr;
  }

  if (getLexer().isNot(AsmToken::LParen)) {
    Error(getLexer().getLoc(), "expected '(' after relocation specifier");
    return nullptr;
  }

  getParser().Lex();

  const MCExpr *SubExpr = nullptr;
  if (getParser().parseExpression(SubExpr)) {
    return nullptr;
  }

  if (getLexer().isNot(AsmToken::RParen)) {
    Error(getLexer().getLoc(), "expected ')'");
    return nullptr;
  }

  EndLoc = getLexer().getTok().getEndLoc();
  getParser().Lex();

  return MCSpecifierExpr::create(SubExpr, Spec, getContext(), Loc);
}

bool dangcsaAsmParser::parseOperand(OperandVector &Operands) {
  SMLoc S = getParser().getTok().getLoc();
  SMLoc E = getParser().getTok().getEndLoc();

  if (getParser().getTok().is(AsmToken::Percent)) {
    const MCExpr *Spec = parseExprWithSpecifier(E);
    if (Spec == nullptr)
      return true;

    Operands.push_back(dangcsaOperand::createImm(Spec, S, E));
    return false;
  }

  if (getParser().getTok().is(AsmToken::LBrac)) {
    Operands.push_back(dangcsaOperand::createToken("[", S));
    getParser().Lex();

    MCRegister Reg;
    SMLoc RS;
    SMLoc RE;
    if (!tryParseRegister(Reg, RS, RE).isSuccess())
      return Error(RS, "expected register inside brackets");

    Operands.push_back(dangcsaOperand::createReg(Reg, RS, RE));

    if (getParser().getTok().is(AsmToken::Plus)) {
      Operands.push_back(
          dangcsaOperand::createToken("+", getParser().getTok().getLoc()));
      getParser().Lex();

      SMLoc DS = getParser().getTok().getLoc();
      SMLoc DE = getParser().getTok().getEndLoc();
      const MCExpr *Disp = nullptr;
      if (getParser().parseExpression(Disp))
        return true;

      Operands.push_back(dangcsaOperand::createImm(Disp, DS, DE));
    } else {
      Operands.push_back(dangcsaOperand::createToken("+", RE));
      Operands.push_back(dangcsaOperand::createImm(
          MCConstantExpr::create(0, getContext()), RE, RE));
    }

    if (getParser().getTok().isNot(AsmToken::RBrac))
      return Error(getParser().getTok().getLoc(), "expected ']'");

    Operands.push_back(
        dangcsaOperand::createToken("]", getParser().getTok().getLoc()));
    getParser().Lex();

    return false;
  }

  MCRegister Reg;
  if (tryParseRegister(Reg, S, E).isSuccess()) {
    Operands.push_back(dangcsaOperand::createReg(Reg, S, E));
    return false;
  }

  const MCExpr *Expr = nullptr;
  if (getParser().parseExpression(Expr))
    return Error(S, "expected register or immediate");

  Operands.push_back(dangcsaOperand::createImm(Expr, S, E));
  return false;
}

bool dangcsaAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                        StringRef Name, SMLoc NameLoc,
                                        OperandVector &Operands) {
  Operands.push_back(dangcsaOperand::createToken(Name, NameLoc));

  while (getLexer().is(AsmToken::Identifier)) {
    StringRef Word = getParser().getTok().getString();

    if (!isFlagKeyword(Word))
      break;

    if (MatchRegisterName(Word) || MatchRegisterAltName(Word))
      break;

    Operands.push_back(
        dangcsaOperand::createToken(Word, getParser().getTok().getLoc()));
    getParser().Lex();
  }

  if (getLexer().is(AsmToken::EndOfStatement))
    return false;

  if (parseOperand(Operands))
    return true;

  while (getLexer().is(AsmToken::Comma)) {
    getParser().Lex();
    if (parseOperand(Operands))
      return true;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement))
    return Error(getLexer().getLoc(), "unexpected token in operand list");

  return false;
}

bool dangcsaAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                               OperandVector &Operands,
                                               MCStreamer &Out,
                                               uint64_t &ErrorInfo,
                                               bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned Result =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);

  switch (Result) {
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MissingFeature:
    return Error(IDLoc, "instruction not supported on this subtarget");
  case Match_MnemonicFail:
    return Error(IDLoc, "unrecognized instruction mnemonic");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");

      ErrorLoc = ((dangcsaOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  default:
    break;
  }

  return Error(IDLoc, "unknown match result");
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializedangcsaAsmParser() {
  RegisterMCAsmParser<dangcsaAsmParser> X(getThedangcsa64Target());
}
