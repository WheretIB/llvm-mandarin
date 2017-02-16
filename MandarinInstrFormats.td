//===-- MandarinInstrFormats.td - Mandarin Instruction Formats ------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//

class InstMD<dag outs, dag ins, string asmstr, list<dag> pattern>
          : Instruction {
  let Namespace = "MD";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
}

// 32-bit instruction
class Inst32MD<dag outs, dag ins, string asmstr, list<dag> pattern>
   : InstMD<outs, ins, asmstr, pattern> {
  field bits<32> Inst;

  bits<1>  flagInstSize;
  bits<7>  operation;

  let flagInstSize = 1;

  let Inst{7-1} = operation;
  let Inst{0}  = flagInstSize;
}

// 16-bit instruction
class Inst16MD<dag outs, dag ins, string asmstr, list<dag> pattern>
   : InstMD<outs, ins, asmstr, pattern> {
  field bits<16> Inst;

  bits<1>  flagInstSize;
  bits<3>  operation;

  let flagInstSize = 0;

  let Inst{3-1} = operation;
  let Inst{0}  = flagInstSize;
}

// 32 bit 3 operand register instruction
class Inst32MD3R<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  repCount = 0;
  bits<5>  regA;
  bits<5>  regB;
  bits<5>  regC;

  let operation = operationVal;

  let Inst{12-8} = repCount;
  let Inst{17-13} = regA;
  let Inst{22-18} = regB;
  let Inst{27-23} = regC;
}

// 32 bit 3 operand integer instruction
class Inst32MD3I<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  regA;
  bits<5>  regB;
  bits<14>  imm14;

  let operation = operationVal;

  let Inst{12-8} = regA;
  let Inst{17-13} = regB;
  let Inst{31-18} = imm14;
}

// 32 bit 2 operand register instruction
class Inst32MD2R<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  repCount = 0;
  bits<5>  regA;
  bits<5>  regB;

  let operation = operationVal;

  let Inst{12-8} = repCount;
  let Inst{17-13} = regA;
  let Inst{22-18} = regB;
}

// 32 bit 2 operand integer instruction
class Inst32MD2I<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  regA;
  bits<19>  imm19;

  let operation = operationVal;

  let Inst{12-8} = regA;
  let Inst{31-13} = imm19;
}

// 32 bit 1 operand register instruction
class Inst32MD1R<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  regA;

  let operation = operationVal;

  let Inst{12-8} = regA;
}

// 32 bit 1 operand integer instruction
class Inst32MD1I<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<24>  imm24;

  let operation = operationVal;

  let Inst{31-8} = imm24;
}

// 32 bit 2 operand memory register instruction
class Inst32MD2MR<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  repCount = 0;
  bits<5>  argA;
  bits<5>  argB;

  let operation = operationVal;

  let Inst{12-8} = repCount;
  let Inst{17-13} = argA;
  let Inst{22-18} = argB;
}

// 32 bit 2 operand memory integer instruction
class Inst32MD2MI<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  bits<5>  repCount = 0;
  bits<5>  argA;
  bits<14>  imm14;

  let operation = operationVal;

  let Inst{12-8} = repCount;
  let Inst{17-13} = argA;
  let Inst{31-18} = imm14;
}

// 32 bit 0 operand instruction
class Inst32MD0<bits<7> operationVal, dag outs, dag ins, string asmstr, list<dag> pattern>
   : Inst32MD<outs, ins, asmstr, pattern> {
  let operation = operationVal;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Inst32MD<outs, ins, asmstr, pattern> {
  let Inst{31-0} = 0;
}
