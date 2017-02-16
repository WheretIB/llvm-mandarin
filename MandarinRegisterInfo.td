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

class MandarinReg<string n> : Register<n> {
  let Namespace = "MD";
}

class MandarinCtrlReg<string n>: Register<n> {
  let Namespace = "MD";
}

// Control Registers
def CC_FLAG : MandarinCtrlReg<"CC_FLAG">;

// Generic registers
def R0 : MandarinReg<"R0">, DwarfRegNum<[0]>;
def R1 : MandarinReg<"R1">, DwarfRegNum<[1]>;
def R2 : MandarinReg<"R2">, DwarfRegNum<[2]>;
def R3 : MandarinReg<"R3">, DwarfRegNum<[3]>;
def R4 : MandarinReg<"R4">, DwarfRegNum<[4]>;
def R5 : MandarinReg<"R5">, DwarfRegNum<[5]>;
def R6 : MandarinReg<"R6">, DwarfRegNum<[6]>;
def R7 : MandarinReg<"R7">, DwarfRegNum<[7]>;
def R8 : MandarinReg<"R8">, DwarfRegNum<[8]>;
def R9 : MandarinReg<"R9">, DwarfRegNum<[9]>;
def R10 : MandarinReg<"R10">, DwarfRegNum<[10]>;
def R11 : MandarinReg<"R11">, DwarfRegNum<[11]>;
def R12 : MandarinReg<"R12">, DwarfRegNum<[12]>;
def R13 : MandarinReg<"R13">, DwarfRegNum<[13]>;
def R14 : MandarinReg<"R14">, DwarfRegNum<[14]>;
def R15 : MandarinReg<"R15">, DwarfRegNum<[15]>;
def R16 : MandarinReg<"R16">, DwarfRegNum<[16]>;
def R17 : MandarinReg<"R17">, DwarfRegNum<[17]>;
def R18 : MandarinReg<"R18">, DwarfRegNum<[18]>;
def R19 : MandarinReg<"R19">, DwarfRegNum<[19]>;
def R20 : MandarinReg<"R20">, DwarfRegNum<[20]>;
def R21 : MandarinReg<"R21">, DwarfRegNum<[21]>;
def R22 : MandarinReg<"R22">, DwarfRegNum<[22]>;
def R23 : MandarinReg<"R23">, DwarfRegNum<[23]>;
def R24 : MandarinReg<"R24">, DwarfRegNum<[24]>;
def R25 : MandarinReg<"R25">, DwarfRegNum<[25]>;
def R26 : MandarinReg<"R26">, DwarfRegNum<[26]>;
def R27 : MandarinReg<"R27">, DwarfRegNum<[27]>;
def R28 : MandarinReg<"R28">, DwarfRegNum<[28]>;
def R29 : MandarinReg<"R29">, DwarfRegNum<[29]>;
def R30 : MandarinReg<"R30">, DwarfRegNum<[30]>; // Stack pointer
def R31 : MandarinReg<"R31">, DwarfRegNum<[31]>; // Frame pointer

def GenericRegs : RegisterClass<"MD", [i32, f32], 32, (sequence "R%u", 0, 31)>;
