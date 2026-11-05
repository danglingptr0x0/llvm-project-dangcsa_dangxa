include "llvm/Target/Target.td"

include "dangcsaRegisterInfo.td"
include "dangcsaCallingConv.td"
include "dangcsaInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def dangcsaInstrInfo : InstrInfo;

def dangcsaAsmParser : AsmParser {
    let ShouldEmitMatchRegisterName = 1;
    let ShouldEmitMatchRegisterAltName = 1;
}

def dangcsaAsmWriter : AsmWriter {
    let PassSubtarget = 0;
}

def : ProcessorModel<"generic-dangcsa", NoSchedModel, []>;

def dangcsa : Target {
    let InstructionSet = dangcsaInstrInfo;
    let AssemblyParsers = [dangcsaAsmParser];
    let AssemblyWriters = [dangcsaAsmWriter];
}
