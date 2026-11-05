def CC_dangcsa : CallingConv<[
    CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
    CCIfType<[i64], CCAssignToReg<[DR0, DR1, DR2, DR3, DR4, DR5]>>,
    CCIfType<[i64], CCAssignToStack<8, 8>>
]>;

def CC_dangcsa_VarArg : CallingConv<[
    CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
    CCIfType<[i64], CCAssignToStack<8, 8>>
]>;

def RetCC_dangcsa : CallingConv<[
    CCIfType<[i1, i8, i16, i32], CCPromoteToType<i64>>,
    CCIfType<[i64], CCAssignToReg<[DR0, DR1]>>
]>;

def CSR_dangcsa : CalleeSavedRegs<(add DR10, DR11, DR12, DR13, DR14, DR15)>;
