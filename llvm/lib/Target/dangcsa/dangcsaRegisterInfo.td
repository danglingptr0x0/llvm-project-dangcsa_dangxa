class dangcsaReg<bits<6> Enc, string n, list<string> alt = []> : Register<n> {
    let HWEncoding{5-0} = Enc;
    let Namespace = "dangcsa";
    let AltNames = alt;
}

foreach i = 0-15 in {
    def DR#i : dangcsaReg<i, "dr"#i>, DwarfRegNum<[i]>;
}

foreach i = 0-5 in {
    def TDR#i : dangcsaReg<!add(i, 16), "tdr"#i>, DwarfRegNum<[!add(i, 16)]>;
}

def LOC : dangcsaReg<22, "loc">, DwarfRegNum<[22]>;
def SP : dangcsaReg<23, "sp">, DwarfRegNum<[23]>;

def GPR : RegisterClass<"dangcsa", [i64], 64, (add (sequence "DR%u", 0, 15), (sequence "TDR%u", 0, 5), SP, LOC)>;

def CARRY : Register<"carry"> {
    let Namespace = "dangcsa";
}

def CCR : RegisterClass<"dangcsa", [i64], 64, (add CARRY)> {
    let CopyCost = -1;
    let isAllocatable = 0;
}

def SPReg : RegisterClass<"dangcsa", [i64], 64, (add SP)>;
