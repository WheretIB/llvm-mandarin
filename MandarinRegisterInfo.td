//===-- MandarinRegisterInfo.td - Mandarin Register defs ------------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Mandarin register file
//===----------------------------------------------------------------------===//

class MandarinReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "MD";
}

class MandarinCtrlReg<string n>: Register<n> {
  let Namespace = "MD";
}

let Namespace = "MD" in {
    def r2sub0 : SubRegIndex<32, 0>;
    def r2sub1 : SubRegIndex<32, 32>;

    def r4sub0 : SubRegIndex<32, 0>;
    def r4sub1 : SubRegIndex<32, 32>;
    def r4sub2 : SubRegIndex<32, 64>;
    def r4sub3 : SubRegIndex<32, 96>;
}

class MandarinReg2<bits<16> Enc, string n, list<Register> subregs> : MandarinReg<Enc, n> {
  let SubRegs = subregs;
  let SubRegIndices = [r2sub0, r2sub1];
  let CoveredBySubRegs = 1;
}

class MandarinReg4<bits<16> Enc, string n, list<Register> subregs> : MandarinReg<Enc, n> {
  let SubRegs = subregs;
  let SubRegIndices = [r4sub0, r4sub1, r4sub2, r4sub3];
  let CoveredBySubRegs = 1;
}


// Control Registers
def CC_FLAG : MandarinCtrlReg<"CC_FLAG">;

// Generic registers
def R0 : MandarinReg<0, "R0">, DwarfRegNum<[0]>;
def R1 : MandarinReg<1, "R1">, DwarfRegNum<[1]>;
def R2 : MandarinReg<2, "R2">, DwarfRegNum<[2]>;
def R3 : MandarinReg<3, "R3">, DwarfRegNum<[3]>;
def R4 : MandarinReg<4, "R4">, DwarfRegNum<[4]>;
def R5 : MandarinReg<5, "R5">, DwarfRegNum<[5]>;
def R6 : MandarinReg<6, "R6">, DwarfRegNum<[6]>;
def R7 : MandarinReg<7, "R7">, DwarfRegNum<[7]>;
def R8 : MandarinReg<8, "R8">, DwarfRegNum<[8]>;
def R9 : MandarinReg<9, "R9">, DwarfRegNum<[9]>;
def R10 : MandarinReg<10, "R10">, DwarfRegNum<[10]>;
def R11 : MandarinReg<11, "R11">, DwarfRegNum<[11]>;
def R12 : MandarinReg<12, "R12">, DwarfRegNum<[12]>;
def R13 : MandarinReg<13, "R13">, DwarfRegNum<[13]>;
def R14 : MandarinReg<14, "R14">, DwarfRegNum<[14]>;
def R15 : MandarinReg<15, "R15">, DwarfRegNum<[15]>;
def R16 : MandarinReg<16, "R16">, DwarfRegNum<[16]>;
def R17 : MandarinReg<17, "R17">, DwarfRegNum<[17]>;
def R18 : MandarinReg<18, "R18">, DwarfRegNum<[18]>;
def R19 : MandarinReg<19, "R19">, DwarfRegNum<[19]>;
def R20 : MandarinReg<20, "R20">, DwarfRegNum<[20]>;
def R21 : MandarinReg<21, "R21">, DwarfRegNum<[21]>;
def R22 : MandarinReg<22, "R22">, DwarfRegNum<[22]>;
def R23 : MandarinReg<23, "R23">, DwarfRegNum<[23]>;
def R24 : MandarinReg<24, "R24">, DwarfRegNum<[24]>;
def R25 : MandarinReg<25, "R25">, DwarfRegNum<[25]>;
def R26 : MandarinReg<26, "R26">, DwarfRegNum<[26]>;
def R27 : MandarinReg<27, "R27">, DwarfRegNum<[27]>;
def R28 : MandarinReg<28, "R28">, DwarfRegNum<[28]>;
def R29 : MandarinReg<29, "R29">, DwarfRegNum<[29]>;
def R30 : MandarinReg<30, "R30">, DwarfRegNum<[30]>; // Stack pointer
def R31 : MandarinReg<31, "R31">, DwarfRegNum<[31]>; // Frame pointer

def RD0 : MandarinReg2<0, "R0", [R0, R1]>, DwarfRegNum<[100]>;
def RD1 : MandarinReg2<2, "R2", [R2, R3]>, DwarfRegNum<[102]>;
def RD2 : MandarinReg2<4, "R4", [R4, R5]>, DwarfRegNum<[104]>;
def RD3 : MandarinReg2<6, "R6", [R6, R7]>, DwarfRegNum<[106]>;
def RD4 : MandarinReg2<8, "R8", [R8, R9]>, DwarfRegNum<[108]>;
def RD5 : MandarinReg2<10, "R10", [R10, R11]>, DwarfRegNum<[110]>;
def RD6 : MandarinReg2<12, "R12", [R12, R13]>, DwarfRegNum<[112]>;
def RD7 : MandarinReg2<14, "R14", [R14, R15]>, DwarfRegNum<[114]>;
def RD8 : MandarinReg2<16, "R16", [R16, R17]>, DwarfRegNum<[116]>;
def RD9 : MandarinReg2<18, "R18", [R18, R19]>, DwarfRegNum<[118]>;
def RD10 : MandarinReg2<20, "R20", [R20, R21]>, DwarfRegNum<[120]>;
def RD11 : MandarinReg2<22, "R22", [R22, R23]>, DwarfRegNum<[122]>;
def RD12 : MandarinReg2<24, "R24", [R24, R25]>, DwarfRegNum<[124]>;
def RD13 : MandarinReg2<26, "R26", [R26, R27]>, DwarfRegNum<[126]>;
def RD14 : MandarinReg2<28, "R28", [R28, R29]>, DwarfRegNum<[128]>;

def RQ0 : MandarinReg4<0, "R0", [R0, R1, R2, R3]>, DwarfRegNum<[200]>;
def RQ1 : MandarinReg4<4, "R4", [R4, R5, R6, R7]>, DwarfRegNum<[204]>;
def RQ2 : MandarinReg4<8, "R8", [R8, R9, R10, R11]>, DwarfRegNum<[208]>;
def RQ3 : MandarinReg4<12, "R12", [R12, R13, R14, R15]>, DwarfRegNum<[212]>;
def RQ4 : MandarinReg4<16, "R16", [R16, R17, R18, R19]>, DwarfRegNum<[216]>;
def RQ5 : MandarinReg4<20, "R20", [R20, R21, R22, R23]>, DwarfRegNum<[220]>;
def RQ6 : MandarinReg4<24, "R24", [R24, R25, R26, R27]>, DwarfRegNum<[224]>;

def GenericRegs : RegisterClass<"MD", [i32, f32], 32, (sequence "R%u", 0, 31)>;
def DoubleRegs : RegisterClass<"MD", [v2i32, v2f32], 32, (sequence "RD%u", 0, 14)>;
def QuadRegs : RegisterClass<"MD", [v4i32, v4f32], 32, (sequence "RQ%u", 0, 6)>;
