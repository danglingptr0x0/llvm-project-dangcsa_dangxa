class dangcsaInst<bits<4> opcode, dag outs, dag ins, string asmstr, list<dag> pattern> : Instruction {
    field bits<64> Inst;

    let Namespace = "dangcsa";
    let Size = 8;
    let OutOperandList = outs;
    let InOperandList = ins;
    let AsmString = asmstr;
    let Pattern = pattern;

    bits<4> Opcode = opcode;
    let Inst{63-60} = Opcode;
}

class dangcsaRR<bits<4> opcode, bits<4> flags, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<opcode, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47-44} = flags;
    let Inst{43-0} = 0;
}

class dangcsaRI<bits<4> opcode, bits<4> flags, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<opcode, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<32> imm;

    let Inst{59-54} = rd;
    let Inst{53-48} = 0;
    let Inst{47-44} = flags;
    let Inst{43-32} = 0;
    let Inst{31-0} = imm;
}

class dangcsaMem<bits<4> opcode, bits<2> width, bit sgn, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<opcode, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;
    bits<32> disp;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47-46} = width;
    let Inst{45} = sgn;
    let Inst{44-32} = 0;
    let Inst{31-0} = disp;
}

class dangcsaShiftRI<bit right, bit arith, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0x8, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> amt;

    let Inst{59-54} = rd;
    let Inst{53-48} = 0;
    let Inst{47} = right;
    let Inst{46} = arith;
    let Inst{45} = 1;
    let Inst{44-6} = 0;
    let Inst{5-0} = amt;
}

class dangcsaShiftRR<bit right, bit arith, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0x8, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47} = right;
    let Inst{46} = arith;
    let Inst{45-0} = 0;
}

class dangcsaJ<bits<4> opcode, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<opcode, outs, ins, asmstr, pattern> {
    bits<6> rd;

    let Inst{59-54} = rd;
    let Inst{53-0} = 0;
}

class dangcsaNoOperand<bits<4> opcode, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<opcode, outs, ins, asmstr, pattern> {
    let Inst{59-0} = 0;
}

class dangcsaJmpAbs<bits<3> cond, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0xC, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47-45} = cond;
    let Inst{44-0} = 0;
}

class dangcsaJmpRel<bits<3> cond, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0xC, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;
    bits<32> offset;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47-45} = cond;
    let Inst{44} = 1;
    let Inst{43-32} = 0;
    let Inst{31-0} = offset;
}

class dangcsaSetcc<bits<3> cond, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0xD, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47-45} = cond;
    let Inst{44-0} = 0;
}

class dangcsaCpyQuarterImm<bits<2> quarter, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0x0, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<16> imm;

    let Inst{59-54} = rd;
    let Inst{53-48} = 0;
    let Inst{47} = 1;
    let Inst{46} = 1;
    let Inst{45-44} = quarter;
    let Inst{43-16} = 0;
    let Inst{15-0} = imm;
}

class dangcsaCpyQuarterReg<bits<2> quarter, dag outs, dag ins, string asmstr, list<dag> pattern> : dangcsaInst<0x0, outs, ins, asmstr, pattern> {
    bits<6> rd;
    bits<6> rs;

    let Inst{59-54} = rd;
    let Inst{53-48} = rs;
    let Inst{47} = 0;
    let Inst{46} = 1;
    let Inst{45-44} = quarter;
    let Inst{43-0} = 0;
}
