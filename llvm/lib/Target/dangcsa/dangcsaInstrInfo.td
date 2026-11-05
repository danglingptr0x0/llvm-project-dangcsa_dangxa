include "dangcsaInstrFormats.td"

def SDT_dangcsaCall : SDTypeProfile<0, -1, [SDTCisVT<0, i64>]>;
def SDT_dangcsaBrCC : SDTypeProfile<0, 4, [SDTCisVT<0, i64>, SDTCisSameAs<0, 1>, SDTCisVT<2, OtherVT>, SDTCisVT<3, OtherVT>]>;
def SDT_dangcsaAbs : SDTypeProfile<1, 1, [SDTCisVT<0, i64>, SDTCisVT<1, i64>]>;

def dangcsa_retglue : SDNode<"dangcsaISD::RET_GLUE", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def dangcsa_call : SDNode<"dangcsaISD::CALL", SDT_dangcsaCall, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def dangcsa_brcc : SDNode<"dangcsaISD::BR_CC", SDT_dangcsaBrCC, [SDNPHasChain]>;
def dangcsa_abs : SDNode<"dangcsaISD::ABS", SDT_dangcsaAbs>;

def SDT_dangcsaCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>;
def SDT_dangcsaCallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i64>, SDTCisVT<1, i64>]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_dangcsaCallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_dangcsaCallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

class Pseudo<dag outs, dag ins, list<dag> pattern> : dangcsaInst<0x0, outs, ins, "", pattern> {
    let isPseudo = 1;
    let isCodeGenOnly = 1;
}

def QUARTER0 : SDNodeXForm<imm, [{ return CurDAG->getTargetConstant(N->getZExtValue() & 0xFFFF, SDLoc(N), MVT::i64); }]>;
def QUARTER1 : SDNodeXForm<imm, [{ return CurDAG->getTargetConstant((N->getZExtValue() >> 16) & 0xFFFF, SDLoc(N), MVT::i64); }]>;
def QUARTER2 : SDNodeXForm<imm, [{ return CurDAG->getTargetConstant((N->getZExtValue() >> 32) & 0xFFFF, SDLoc(N), MVT::i64); }]>;
def QUARTER3 : SDNodeXForm<imm, [{ return CurDAG->getTargetConstant((N->getZExtValue() >> 48) & 0xFFFF, SDLoc(N), MVT::i64); }]>;

def simm32val : PatLeaf<(imm), [{ return isInt<32>(N->getSExtValue()); }]>;
def uimm6amt : PatLeaf<(imm), [{ return N->getZExtValue() < 64; }]>;
def wideMask : PatLeaf<(imm), [{ return isMask_64(N->getZExtValue()) && !isInt<32>(N->getSExtValue()); }]>;
def MASKSHIFT : SDNodeXForm<imm, [{ return CurDAG->getTargetConstant(64 - llvm::countr_one(N->getZExtValue()), SDLoc(N), MVT::i64); }]>;

def AddrRegImm : ComplexPattern<i64, 2, "SelectAddrRegImm", [frameindex], []>;

def SImm32AsmOperand : AsmOperandClass {
    let Name = "SImm32";
    let RenderMethod = "addImmOperands";
    let DiagnosticType = "InvalidSImm32";
}

def UImm16AsmOperand : AsmOperandClass {
    let Name = "UImm16";
    let RenderMethod = "addImmOperands";
    let DiagnosticType = "InvalidUImm16";
}

def UImm6AsmOperand : AsmOperandClass {
    let Name = "UImm6";
    let RenderMethod = "addImmOperands";
    let DiagnosticType = "InvalidUImm6";
}

def BranchTargetAsmOperand : AsmOperandClass {
    let Name = "BranchTarget";
    let RenderMethod = "addImmOperands";
    let DiagnosticType = "InvalidBranchTarget";
}

def simm32 : Operand<i64> {
    let ParserMatchClass = SImm32AsmOperand;
    let DecoderMethod = "decodeSImm32";
    let PrintMethod = "printSImm32";
}

def uimm16 : Operand<i64> {
    let ParserMatchClass = UImm16AsmOperand;
    let DecoderMethod = "decodeUImm16";
    let PrintMethod = "printUImm16";
}

def uimm6 : Operand<i64> {
    let ParserMatchClass = UImm6AsmOperand;
    let DecoderMethod = "decodeUImm6";
    let PrintMethod = "printUImm6";
}

def bt32 : Operand<OtherVT> {
    let ParserMatchClass = BranchTargetAsmOperand;
    let EncoderMethod = "getBranchTargetOpValue";
    let DecoderMethod = "decodeBranchTarget";
    let PrintMethod = "printBranchTarget";
    let OperandType = "OPERAND_PCREL";
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {

def CPY : dangcsaRR<0x0, 0b0000, (outs GPR:$rd), (ins GPR:$rs), "cpy $rd, $rs", []>;
def CPY_ri : dangcsaRI<0x0, 0b1000, (outs GPR:$rd), (ins simm32:$imm), "cpy i $rd, $imm", []>;

def CLZ : dangcsaRR<0xB, 0b0000, (outs GPR:$rd), (ins GPR:$rs), "clz $rd, $rs", []>;
def CTZ : dangcsaRR<0xB, 0b1000, (outs GPR:$rd), (ins GPR:$rs), "clz t $rd, $rs", []>;

let Constraints = "$rdin = $rd" in {

def CPYQ0_ri : dangcsaCpyQuarterImm<0, (outs GPR:$rd), (ins GPR:$rdin, uimm16:$imm), "cpy q0 i $rd, $imm", []>;
def CPYQ1_ri : dangcsaCpyQuarterImm<1, (outs GPR:$rd), (ins GPR:$rdin, uimm16:$imm), "cpy q1 i $rd, $imm", []>;
def CPYQ2_ri : dangcsaCpyQuarterImm<2, (outs GPR:$rd), (ins GPR:$rdin, uimm16:$imm), "cpy q2 i $rd, $imm", []>;
def CPYQ3_ri : dangcsaCpyQuarterImm<3, (outs GPR:$rd), (ins GPR:$rdin, uimm16:$imm), "cpy q3 i $rd, $imm", []>;

def CPYQ0_rr : dangcsaCpyQuarterReg<0, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "cpy q0 $rd, $rs", []>;
def CPYQ1_rr : dangcsaCpyQuarterReg<1, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "cpy q1 $rd, $rs", []>;
def CPYQ2_rr : dangcsaCpyQuarterReg<2, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "cpy q2 $rd, $rs", []>;
def CPYQ3_rr : dangcsaCpyQuarterReg<3, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "cpy q3 $rd, $rs", []>;

let Defs = [CARRY] in {
def ADD : dangcsaRR<0x3, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "add $rd, $rs", []>;
def SUB : dangcsaRR<0x4, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "sub $rd, $rs", []>;
def ADD_ri : dangcsaRI<0x3, 0b0100, (outs GPR:$rd), (ins GPR:$rdin, simm32:$imm), "add i $rd, $imm", []>;
def SUB_ri : dangcsaRI<0x4, 0b0100, (outs GPR:$rd), (ins GPR:$rdin, simm32:$imm), "sub i $rd, $imm", []>;
}

let Defs = [CARRY], Uses = [CARRY] in {
def ADDC : dangcsaRR<0x3, 0b1000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "add c $rd, $rs", []>;
def SUBC : dangcsaRR<0x4, 0b1000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "sub c $rd, $rs", []>;
def ADDC_ri : dangcsaRI<0x3, 0b1100, (outs GPR:$rd), (ins GPR:$rdin, simm32:$imm), "add c i $rd, $imm", []>;
def SUBC_ri : dangcsaRI<0x4, 0b1100, (outs GPR:$rd), (ins GPR:$rdin, simm32:$imm), "sub c i $rd, $imm", []>;
}

def MUL : dangcsaRR<0x9, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "mul $rd, $rs", []>;
def MULHU : dangcsaRR<0x9, 0b1000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "mul hu $rd, $rs", []>;
def MULHS : dangcsaRR<0x9, 0b1100, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "mul hs $rd, $rs", []>;

def AND : dangcsaRR<0x5, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "and $rd, $rs", []>;
def OR : dangcsaRR<0x6, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "or $rd, $rs", []>;
def XOR : dangcsaRR<0x7, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "xor $rd, $rs", []>;

def SHL_ri : dangcsaShiftRI<0, 0, (outs GPR:$rd), (ins GPR:$rdin, uimm6:$amt), "sh l i $rd, $amt", []>;
def SHL_rr : dangcsaShiftRR<0, 0, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "sh l $rd, $rs", []>;
def SHR_ri : dangcsaShiftRI<1, 0, (outs GPR:$rd), (ins GPR:$rdin, uimm6:$amt), "sh r i $rd, $amt", []>;
def SHR_rr : dangcsaShiftRR<1, 0, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "sh r $rd, $rs", []>;
def SRA_ri : dangcsaShiftRI<1, 1, (outs GPR:$rd), (ins GPR:$rdin, uimm6:$amt), "sh ra i $rd, $amt", []>;
def SRA_rr : dangcsaShiftRR<1, 1, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "sh ra $rd, $rs", []>;

def UDIV : dangcsaRR<0xA, 0b0000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "div u $rd, $rs", []>;
def SDIV : dangcsaRR<0xA, 0b1000, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "div s $rd, $rs", []>;
def UREM : dangcsaRR<0xA, 0b0100, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "div ur $rd, $rs", []>;
def SREM : dangcsaRR<0xA, 0b1100, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "div sr $rd, $rs", []>;

}

}

multiclass dangcsaLoad<bits<2> width, bit sgn, string flag> {
    let mayLoad = 1, hasSideEffects = 0, mayStore = 0 in
    def NAME : dangcsaMem<0x1, width, sgn, (outs GPR:$rd), (ins GPR:$rs, simm32:$disp), "ld " # flag # " $rd, [$rs + $disp]", []>;
}

multiclass dangcsaStore<bits<2> width, string flag> {
    let mayStore = 1, hasSideEffects = 0, mayLoad = 0 in
    def NAME : dangcsaMem<0x2, width, 0, (outs), (ins GPR:$rd, simm32:$disp, GPR:$rs), "st " # flag # " [$rd + $disp], $rs", []>;
}

defm LDQ : dangcsaLoad<0b11, 0, "q">;
defm LDDS : dangcsaLoad<0b10, 1, "ds">;
defm LDDU : dangcsaLoad<0b10, 0, "du">;
defm LDWS : dangcsaLoad<0b01, 1, "ws">;
defm LDWU : dangcsaLoad<0b01, 0, "wu">;
defm LDBS : dangcsaLoad<0b00, 1, "bs">;
defm LDBU : dangcsaLoad<0b00, 0, "bu">;

defm STQ : dangcsaStore<0b11, "q">;
defm STD : dangcsaStore<0b10, "d">;
defm STW : dangcsaStore<0b01, "w">;
defm STB : dangcsaStore<0b00, "b">;

let isBranch = 1, isTerminator = 1, isBarrier = 1, isIndirectBranch = 1 in
def JMP : dangcsaJmpAbs<0b110, (outs), (ins GPR:$rd), "jmp $rd", []> {
    let rs = 0;
}

multiclass dangcsaJmpAbsCond<bits<3> cond, string flag> {
    let isBranch = 1, isTerminator = 1, isIndirectBranch = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
    def NAME : dangcsaJmpAbs<cond, (outs), (ins GPR:$rd, GPR:$rs), "jmp " # flag # " $rd, $rs", []>;
}

defm JEQ : dangcsaJmpAbsCond<0b000, "eq">;
defm JNE : dangcsaJmpAbsCond<0b001, "ne">;
defm JLT : dangcsaJmpAbsCond<0b010, "lt">;
defm JGE : dangcsaJmpAbsCond<0b011, "ge">;
defm JLTU : dangcsaJmpAbsCond<0b100, "ltu">;
defm JGEU : dangcsaJmpAbsCond<0b101, "geu">;

let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def JREL : dangcsaJmpRel<0b110, (outs), (ins bt32:$offset), "jmp rel al $offset", []> {
    let rd = 0;
    let rs = 0;
}

multiclass dangcsaJmpRelCond<bits<3> cond, string flag> {
    let isBranch = 1, isTerminator = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
    def NAME : dangcsaJmpRel<cond, (outs), (ins GPR:$rd, GPR:$rs, bt32:$offset), "jmp rel " # flag # " $rd, $rs, $offset", []>;
}

defm BEQ : dangcsaJmpRelCond<0b000, "eq">;
defm BNE : dangcsaJmpRelCond<0b001, "ne">;
defm BLT : dangcsaJmpRelCond<0b010, "lt">;
defm BGE : dangcsaJmpRelCond<0b011, "ge">;
defm BLTU : dangcsaJmpRelCond<0b100, "ltu">;
defm BGEU : dangcsaJmpRelCond<0b101, "geu">;

multiclass dangcsaSetCond<bits<3> cond, string flag> {
    let hasSideEffects = 0, mayLoad = 0, mayStore = 0, Constraints = "$rdin = $rd" in
    def NAME : dangcsaSetcc<cond, (outs GPR:$rd), (ins GPR:$rdin, GPR:$rs), "setcc " # flag # " $rd, $rs", []>;
}

defm SET_EQ : dangcsaSetCond<0b000, "eq">;
defm SET_NE : dangcsaSetCond<0b001, "ne">;
defm SET_LT : dangcsaSetCond<0b010, "lt">;
defm SET_GE : dangcsaSetCond<0b011, "ge">;
defm SET_LTU : dangcsaSetCond<0b100, "ltu">;
defm SET_GEU : dangcsaSetCond<0b101, "geu">;
defm SET_AL : dangcsaSetCond<0b110, "al">;

let isCall = 1, Defs = [SP] in
def CALL : dangcsaJ<0xE, (outs), (ins GPR:$rd), "call $rd", []>;

let isReturn = 1, isTerminator = 1, isBarrier = 1, Uses = [SP] in
def RET : dangcsaNoOperand<0xF, (outs), (ins), "ret", []>;

let Uses = [SP] in
def FRAMEADDR : Pseudo<(outs GPR:$rd), (ins GPR:$base, simm32:$disp), []>;

let Uses = [LOC], hasSideEffects = 0, mayLoad = 0, mayStore = 0, Size = 16 in
def PCADDR : Pseudo<(outs GPR:$rd), (ins i64imm:$sym), []>;

let Defs = [SP], Uses = [SP] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2), [(callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2), [(callseq_end timm:$amt1, timm:$amt2)]>;
}

def : Pat<(add GPR:$rdin, GPR:$rs), (ADD GPR:$rdin, GPR:$rs)>;
def : Pat<(sub GPR:$rdin, GPR:$rs), (SUB GPR:$rdin, GPR:$rs)>;
def : Pat<(addc GPR:$rdin, GPR:$rs), (ADD GPR:$rdin, GPR:$rs)>;
def : Pat<(adde GPR:$rdin, GPR:$rs), (ADDC GPR:$rdin, GPR:$rs)>;
def : Pat<(subc GPR:$rdin, GPR:$rs), (SUB GPR:$rdin, GPR:$rs)>;
def : Pat<(sube GPR:$rdin, GPR:$rs), (SUBC GPR:$rdin, GPR:$rs)>;

let AddedComplexity = 5 in {
def : Pat<(add GPR:$rdin, simm32val:$imm), (ADD_ri GPR:$rdin, simm32val:$imm)>;
def : Pat<(sub GPR:$rdin, simm32val:$imm), (SUB_ri GPR:$rdin, simm32val:$imm)>;
}

def : Pat<(mul GPR:$rdin, GPR:$rs), (MUL GPR:$rdin, GPR:$rs)>;
def : Pat<(mulhu GPR:$rdin, GPR:$rs), (MULHU GPR:$rdin, GPR:$rs)>;
def : Pat<(mulhs GPR:$rdin, GPR:$rs), (MULHS GPR:$rdin, GPR:$rs)>;
def : Pat<(ctlz GPR:$rs), (CLZ GPR:$rs)>;
def : Pat<(cttz GPR:$rs), (CTZ GPR:$rs)>;
def : Pat<(and GPR:$rdin, GPR:$rs), (AND GPR:$rdin, GPR:$rs)>;
def : Pat<(or GPR:$rdin, GPR:$rs), (OR GPR:$rdin, GPR:$rs)>;
def : Pat<(xor GPR:$rdin, GPR:$rs), (XOR GPR:$rdin, GPR:$rs)>;
def : Pat<(udiv GPR:$rdin, GPR:$rs), (UDIV GPR:$rdin, GPR:$rs)>;
def : Pat<(urem GPR:$rdin, GPR:$rs), (UREM GPR:$rdin, GPR:$rs)>;
def : Pat<(sdiv GPR:$rdin, GPR:$rs), (SDIV GPR:$rdin, GPR:$rs)>;
def : Pat<(srem GPR:$rdin, GPR:$rs), (SREM GPR:$rdin, GPR:$rs)>;
def : Pat<(shl GPR:$rdin, GPR:$rs), (SHL_rr GPR:$rdin, GPR:$rs)>;
def : Pat<(srl GPR:$rdin, GPR:$rs), (SHR_rr GPR:$rdin, GPR:$rs)>;
def : Pat<(sra GPR:$rdin, GPR:$rs), (SRA_rr GPR:$rdin, GPR:$rs)>;

def : Pat<(shl GPR:$rdin, uimm6amt:$amt), (SHL_ri GPR:$rdin, uimm6amt:$amt)>;
def : Pat<(srl GPR:$rdin, uimm6amt:$amt), (SHR_ri GPR:$rdin, uimm6amt:$amt)>;
def : Pat<(sra GPR:$rdin, uimm6amt:$amt), (SRA_ri GPR:$rdin, uimm6amt:$amt)>;

def : Pat<(setcc GPR:$rdin, GPR:$rs, SETEQ), (SET_EQ GPR:$rdin, GPR:$rs)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETNE), (SET_NE GPR:$rdin, GPR:$rs)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETLT), (SET_LT GPR:$rdin, GPR:$rs)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETGE), (SET_GE GPR:$rdin, GPR:$rs)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETULT), (SET_LTU GPR:$rdin, GPR:$rs)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETUGE), (SET_GEU GPR:$rdin, GPR:$rs)>;

def : Pat<(setcc GPR:$rdin, GPR:$rs, SETGT), (SET_LT GPR:$rs, GPR:$rdin)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETLE), (SET_GE GPR:$rs, GPR:$rdin)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETUGT), (SET_LTU GPR:$rs, GPR:$rdin)>;
def : Pat<(setcc GPR:$rdin, GPR:$rs, SETULE), (SET_GEU GPR:$rs, GPR:$rdin)>;

def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETEQ, bb:$offset), (BEQ GPR:$rd, GPR:$rs, bb:$offset)>;
def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETNE, bb:$offset), (BNE GPR:$rd, GPR:$rs, bb:$offset)>;
def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETLT, bb:$offset), (BLT GPR:$rd, GPR:$rs, bb:$offset)>;
def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETGE, bb:$offset), (BGE GPR:$rd, GPR:$rs, bb:$offset)>;
def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETULT, bb:$offset), (BLTU GPR:$rd, GPR:$rs, bb:$offset)>;
def : Pat<(dangcsa_brcc GPR:$rd, GPR:$rs, SETUGE, bb:$offset), (BGEU GPR:$rd, GPR:$rs, bb:$offset)>;

def : Pat<(load (AddrRegImm GPR:$ptr, simm32:$disp)), (LDQ GPR:$ptr, simm32:$disp)>;
def : Pat<(sextloadi32 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDDS GPR:$ptr, simm32:$disp)>;
def : Pat<(zextloadi32 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDDU GPR:$ptr, simm32:$disp)>;
def : Pat<(extloadi32 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDDU GPR:$ptr, simm32:$disp)>;
def : Pat<(sextloadi16 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDWS GPR:$ptr, simm32:$disp)>;
def : Pat<(zextloadi16 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDWU GPR:$ptr, simm32:$disp)>;
def : Pat<(extloadi16 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDWU GPR:$ptr, simm32:$disp)>;
def : Pat<(sextloadi8 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDBS GPR:$ptr, simm32:$disp)>;
def : Pat<(zextloadi8 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDBU GPR:$ptr, simm32:$disp)>;
def : Pat<(extloadi8 (AddrRegImm GPR:$ptr, simm32:$disp)), (LDBU GPR:$ptr, simm32:$disp)>;

def : Pat<(store GPR:$val, (AddrRegImm GPR:$ptr, simm32:$disp)), (STQ GPR:$ptr, simm32:$disp, GPR:$val)>;
def : Pat<(truncstorei32 GPR:$val, (AddrRegImm GPR:$ptr, simm32:$disp)), (STD GPR:$ptr, simm32:$disp, GPR:$val)>;
def : Pat<(truncstorei16 GPR:$val, (AddrRegImm GPR:$ptr, simm32:$disp)), (STW GPR:$ptr, simm32:$disp, GPR:$val)>;
def : Pat<(truncstorei8 GPR:$val, (AddrRegImm GPR:$ptr, simm32:$disp)), (STB GPR:$ptr, simm32:$disp, GPR:$val)>;

def : Pat<(dangcsa_abs tglobaladdr:$sym), (CPY_ri tglobaladdr:$sym)>;
def : Pat<(dangcsa_abs tblockaddress:$sym), (CPY_ri tblockaddress:$sym)>;
def : Pat<(dangcsa_abs tjumptable:$sym), (CPY_ri tjumptable:$sym)>;
def : Pat<(dangcsa_abs texternalsym:$sym), (CPY_ri texternalsym:$sym)>;

let AddedComplexity = 10 in
def : Pat<(i64 simm32val:$imm), (CPY_ri simm32val:$imm)>;

let AddedComplexity = 8 in
def : Pat<(i64 wideMask:$imm), (SHR_ri (CPY_ri -1), (MASKSHIFT imm:$imm))>;

def : Pat<(i64 imm:$imm), (CPYQ0_ri (CPYQ1_ri (CPYQ2_ri (CPYQ3_ri (i64 (IMPLICIT_DEF)), (QUARTER3 imm:$imm)), (QUARTER2 imm:$imm)), (QUARTER1 imm:$imm)), (QUARTER0 imm:$imm))>;

def : Pat<(dangcsa_call GPR:$rd), (CALL GPR:$rd)>;
def : Pat<(dangcsa_retglue), (RET)>;
def : Pat<(brind GPR:$rd), (JMP GPR:$rd)>;
def : Pat<(br bb:$offset), (JREL bb:$offset)>;
