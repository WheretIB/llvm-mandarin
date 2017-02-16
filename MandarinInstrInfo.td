//===-- MandarinInstrInfo.td - Target Description for Mandarin Target -----===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file describes the Mandarin instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//

include "MandarinInstrFormats.td"

//===----------------------------------------------------------------------===//
// Immediate contraints.
//===----------------------------------------------------------------------===//

def simm14  : PatLeaf<(imm), [{ return isInt<14>(N->getSExtValue()); }]>;
def uimm14  : PatLeaf<(imm), [{ return isUInt<14>(N->getZExtValue()); }]>;

def simm19  : PatLeaf<(imm), [{ return isInt<19>(N->getSExtValue()); }]>;
def uimm19  : PatLeaf<(imm), [{ return isUInt<19>(N->getZExtValue()); }]>;

def simm24  : PatLeaf<(imm), [{ return isInt<24>(N->getSExtValue()); }]>;
def uimm24  : PatLeaf<(imm), [{ return isUInt<24>(N->getZExtValue()); }]>;

def LO19 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() & 524287,
                                   MVT::i32);
}]>;

def HI13 : SDNodeXForm<imm, [{
  // Transformation function: shift the immediate value down into the low bits.
  return CurDAG->getTargetConstant((unsigned)N->getZExtValue() >> 19, MVT::i32);
}]>;

//===----------------------------------------------------------------------===//
// Type Profiles.
//===----------------------------------------------------------------------===//

// SDTCisVT - The specified operand has exactly this VT.
// SDTCisSameAs - The two specified operands have identical types.
// SDTypeProfile - This profile describes the type requirements of a Selection DAG node.
//      class SDTypeProfile<int numresults, int numoperands, list<SDTypeConstraint> constraints>
// class SDCallSeqStart<list<SDTypeConstraint> constraints> : SDTypeProfile<0, 1, constraints>;

def SDT_MDCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_MDCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_MDCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_MDCmp          : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDT_MDFcmp         : SDTypeProfile<0, 2, [SDTCisVT<0, f32>]>;
def SDT_MDBrcc         : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>, SDTCisVT<1, i32>]>;
def SDT_MDselectcc     : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>]>;

//===----------------------------------------------------------------------===//
// Specific Node Definitions.
//===----------------------------------------------------------------------===//

// class SDNode<string opcode, SDTypeProfile typeprof, list<SDNodeProperty> props = [], string sdclass = "SDNode">
// def SDNPHasChain    : SDNodeProperty;   // R/W chain operand and result
// def SDNPOutGlue     : SDNodeProperty;   // Write a flag result
// def SDNPInGlue      : SDNodeProperty;   // Read a flag operand
// def SDNPOptInGlue   : SDNodeProperty;   // Optionally read a flag operand
// def SDNPMayStore    : SDNodeProperty;   // May write to memory, sets 'mayStore'.
// def SDNPMayLoad     : SDNodeProperty;   // May read memory, sets 'mayLoad'.
// def SDNPSideEffect  : SDNodeProperty;   // Sets 'HasUnmodelledSideEffects'.
// def SDNPMemOperand  : SDNodeProperty;   // Touches memory, has assoc MemOperand
// def SDNPVariadic    : SDNodeProperty;   // Node has variable arguments.

def MDretflag  : SDNode<"MDISD::RET_FLAG", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def MDcmp     : SDNode<"MDISD::CMP", SDT_MDCmp, [SDNPOutGlue]>;
def MDicmp    : SDNode<"MDISD::ICMP", SDT_MDCmp, [SDNPOutGlue]>;

def MDfcmp    : SDNode<"MDISD::FCMP", SDT_MDFcmp, [SDNPOutGlue]>;

def MDbrcc    : SDNode<"MDISD::BR_CC", SDT_MDBrcc,
                            [SDNPHasChain, SDNPInGlue]>;

def MDselectcc : SDNode<"MDISD::SELECT_CC", SDT_MDselectcc, [SDNPInGlue]>;

def MDhigh   : SDNode<"MDISD::HIGH", SDTIntUnaryOp>;
def MDlow    : SDNode<"MDISD::LOW", SDTIntUnaryOp>;

def MDcall   : SDNode<"MDISD::CALL", SDT_MDCall,
                     [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def MDcallseq_start :
                 SDNode<"ISD::CALLSEQ_START", SDT_MDCallSeqStart,
                        [SDNPHasChain, SDNPOutGlue]>;
def MDcallseq_end :
                 SDNode<"ISD::CALLSEQ_END",   SDT_MDCallSeqEnd,
                        [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def jmptarget : Operand<OtherVT> {
  let PrintMethod = "printBasicBlock";
}

def cc : Operand<i32> {
  let PrintMethod = "printCCOperand";
}

def addr : ComplexPattern<iPTR, 1, "SelectAddr", [], []>;

//===----------------------------------------------------------------------===//
// Strange llvm instructions.
//===----------------------------------------------------------------------===//

let Defs = [R31], Uses = [R31] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt),
                               "!ADJCALLSTACKDOWN $amt",
                               [(MDcallseq_start timm:$amt)]>;
def ADJCALLSTACKUP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                            "!ADJCALLSTACKUP $amt1",
                            [(MDcallseq_end timm:$amt1, timm:$amt2)]>;
}

let Uses = [CC_FLAG], usesCustomInserter = 1 in {

  def SELECT_CC_Int
   : Pseudo<(outs GenericRegs:$dst), (ins GenericRegs:$T, GenericRegs:$F, i32imm:$Cond),
            "// SELECT_CC_Int PSEUDO!",
            [(set i32:$dst, (MDselectcc i32:$T, i32:$F, imm:$Cond))]>;
  def SELECT_CC_Float
   : Pseudo<(outs GenericRegs:$dst), (ins GenericRegs:$T, GenericRegs:$F, i32imm:$Cond),
            "; SELECT_CC_Float PSEUDO!",
            [(set f32:$dst, (MDselectcc f32:$T, f32:$F, imm:$Cond))]>;

}

//===----------------------------------------------------------------------===//
// Instructions.
//===----------------------------------------------------------------------===//

def ADDrr : Inst32MD3R<0,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "add $dst, $src1, $src2",
                  [(set i32:$dst, (add i32:$src1, i32:$src2))]>;

def ADDri : Inst32MD3I<0,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "add $dst, $src1, $src2",
                  [(set i32:$dst, (add i32:$src1, uimm14:$src2))]>;

def SUBrr : Inst32MD3R<1,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "sub $dst, $src1, $src2",
                  [(set i32:$dst, (sub i32:$src1, i32:$src2))]>;

def SUBri : Inst32MD3I<1,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "sub $dst, $src1, $src2",
                  [(set i32:$dst, (sub i32:$src1, uimm14:$src2))]>;

def MULrr : Inst32MD3R<2,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "mul $dst, $src1, $src2",
                  [(set i32:$dst, (mul i32:$src1, i32:$src2))]>;

def MULri : Inst32MD3I<2,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "mul $dst, $src1, $src2",
                  [(set i32:$dst, (mul i32:$src1, uimm14:$src2))]>;

def DIVrr : Inst32MD3R<3,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "div $dst, $src1, $src2",
                  [(set i32:$dst, (sdiv i32:$src1, i32:$src2))]>;

def DIVri : Inst32MD3I<3,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "div $dst, $src1, $src2",
                  [(set i32:$dst, (sdiv i32:$src1, simm14:$src2))]>;

def MODrr : Inst32MD3R<4,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "mod $dst, $src1, $src2",
                  [(set i32:$dst, (srem i32:$src1, i32:$src2))]>;

def MODri : Inst32MD3I<4,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "mod $dst, $src1, $src2",
                  [(set i32:$dst, (srem i32:$src1, simm14:$src2))]>;

def FADDrr : Inst32MD3R<5,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "fadd $dst, $src1, $src2",
                  [(set f32:$dst, (fadd f32:$src1, f32:$src2))]>;

/*def FADDri : Inst32MD3I<5,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "fadd $dst, $src1, $src2",
                  [(set f32:$dst, (fadd f32:$src1, uimm14:$src2))]>;*/

def FSUBrr : Inst32MD3R<6,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "fsub $dst, $src1, $src2",
                  [(set f32:$dst, (fsub f32:$src1, f32:$src2))]>;

/*def FSUBri : Inst32MD3I<6,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "fsub $dst, $src1, $src2",
                  [(set f32:$dst, (fsub f32:$src1, uimm14:$src2))]>;*/

def FMULrr : Inst32MD3R<7,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "fmul $dst, $src1, $src2",
                  [(set f32:$dst, (fmul f32:$src1, f32:$src2))]>;

/*def FMULri : Inst32MD3I<7,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "fmul $dst, $src1, $src2",
                  [(set f32:$dst, (fmul f32:$src1, uimm14:$src2))]>;*/

def FDIVrr : Inst32MD3R<8,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "fdiv $dst, $src1, $src2",
                  [(set f32:$dst, (fdiv f32:$src1, f32:$src2))]>;

/*def FDIVri : Inst32MD3I<8,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "fdiv $dst, $src1, $src2",
                  [(set f32:$dst, (fdiv f32:$src1, uimm14:$src2))]>;*/

def SHLrr : Inst32MD3R<9,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "shl $dst, $src1, $src2",
                  [(set i32:$dst, (shl i32:$src1, i32:$src2))]>;

def SHLri : Inst32MD3I<9,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "shl $dst, $src1, $src2",
                  [(set i32:$dst, (shl i32:$src1, uimm14:$src2))]>;

def SHRrr : Inst32MD3R<10,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "shr $dst, $src1, $src2",
                  [(set i32:$dst, (srl i32:$src1, i32:$src2))]>;

def SHRri : Inst32MD3I<10,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "shr $dst, $src1, $src2",
                  [(set i32:$dst, (srl i32:$src1, uimm14:$src2))]>;

def ANDrr : Inst32MD3R<11,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "and $dst, $src1, $src2",
                  [(set i32:$dst, (and i32:$src1, i32:$src2))]>;

def ANDri : Inst32MD3I<11,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "and $dst, $src1, $src2",
                  [(set i32:$dst, (and i32:$src1, uimm14:$src2))]>;

def ORrr : Inst32MD3R<12,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "or $dst, $src1, $src2",
                  [(set i32:$dst, (or i32:$src1, i32:$src2))]>;

def ORri : Inst32MD3I<12,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "or $dst, $src1, $src2",
                  [(set i32:$dst, (or i32:$src1, uimm14:$src2))]>;

def XORrr : Inst32MD3R<13,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                  "xor $dst, $src1, $src2",
                  [(set i32:$dst, (xor i32:$src1, i32:$src2))]>;

def XORri : Inst32MD3I<13,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                  "xor $dst, $src1, $src2",
                  [(set i32:$dst, (xor i32:$src1, uimm14:$src2))]>;

// Unsigned compare - custom code logic
let Defs = [CC_FLAG] in {

    def CMPrr : Inst32MD2R<14,
                      (outs), (ins GenericRegs:$src1, GenericRegs:$src2),
                      "cmp $src1, $src2",
                      [(MDcmp i32:$src1, i32:$src2)]>;

    def CMPri : Inst32MD2I<14,
                      (outs), (ins GenericRegs:$src1, i32imm:$src2),
                      "cmp $src1, $src2",
                      [(MDcmp i32:$src1, uimm19:$src2)]>;

    // Signed compare - custom code logic
    def SCMPrr : Inst32MD2R<15,
                      (outs), (ins GenericRegs:$src1, GenericRegs:$src2),
                      "scmp $src1, $src2",
                      [(MDicmp i32:$src1, i32:$src2)]>;

    def SCMPri : Inst32MD2I<15,
                      (outs), (ins GenericRegs:$src1, i32imm:$src2),
                      "scmp $src1, $src2",
                      [(MDicmp i32:$src1, simm19:$src2)]>;

    // Float compare - custom code logic
    def FCMPrr : Inst32MD2R<17,
                      (outs), (ins GenericRegs:$src1, GenericRegs:$src2),
                      "fcmp $src1, $src2",
                      [(MDfcmp f32:$src1, f32:$src2)]>;

    def FCMPri : Inst32MD2I<17,
                      (outs), (ins GenericRegs:$src1, i32imm:$src2),
                      "fcmp $src1, $src2",
                      [(MDfcmp f32:$src1, simm19:$src2)]>;
}

def FTOIrr : Inst32MD2R<18,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "ftoi $dst, $src",
                  [(set i32:$dst, (fp_to_sint f32:$src))]>;

/*def FTOIri : Inst32MD2R<18,
                  (outs GenericRegs:$dst), (ins i32imm:$src),
                  "ftoi $dst, $src",
                  [(set i32:$dst, (fp_to_sint simm19:$src))]>;*/

def ITOFrr : Inst32MD2R<19,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "ftoi $dst, $src",
                  [(set f32:$dst, (sint_to_fp i32:$src))]>;

def ITOFri : Inst32MD2R<19,
                  (outs GenericRegs:$dst), (ins i32imm:$src),
                  "ftoi $dst, $src",
                  [(set f32:$dst, (sint_to_fp simm19:$src))]>;

// TODO: call

let isBranch = 1, isTerminator = 1 in {
  
  // TODO: correct bytecode
  let Uses = [CC_FLAG] in {

      def JCCi : Inst32MD1I<0,
                       (outs), (ins jmptarget:$dst, cc:$cc),
                       "j$cc $dst",
                       [(MDbrcc bb:$dst, imm:$cc)]>;
  }

  let isBarrier = 1 in {

      def JMPi : Inst32MD1I<28,
                       (outs), (ins jmptarget:$dst),
                       "jmp $dst",
                       [(br bb:$dst)]>;
  }

  let isIndirectBranch = 1 in {

      let Uses = [CC_FLAG] in {

          // TODO: correct bytecode
          def JCCr : Inst32MD1I<0,
                           (outs), (ins GenericRegs:$dst, cc:$cc),
                           "j$cc $dst",
                           []>;
      }

      let isBarrier = 1 in {

          def JMPr : Inst32MD1I<28,
                           (outs), (ins GenericRegs:$dst),
                           "jmp $dst",
                           [(brind i32:$dst)]>;
      }
    }
}

// Global memory
def LOADrr : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "load $dst, $addr",
                  [(set i32:$dst, (load addr:$addr))]>;

def LOADri : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "load $dst, $addr",
                  [(set i32:$dst, (load addr:$addr))]>;

def LOADfrr : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "load $dst, $addr",
                  [(set f32:$dst, (load addr:$addr))]>;

def LOADfri : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "load $dst, $addr",
                  [(set f32:$dst, (load addr:$addr))]>;

def LOADWrr : Inst32MD2R<30,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "loadw $dst, $addr",
                  [(set i32:$dst, (zextloadi16 addr:$addr))]>;

def LOADWri : Inst32MD2I<30,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "loadw $dst, $addr",
                  [(set i32:$dst, (zextloadi16 addr:$addr))]>;

def LOADBrr : Inst32MD2R<31,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "loadb $dst, $addr",
                  [(set i32:$dst, (zextloadi8 addr:$addr))]>;

def LOADBri : Inst32MD2I<31,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "loadb $dst, $addr",
                  [(set i32:$dst, (zextloadi8 addr:$addr))]>;

def STORErr : Inst32MD2R<32,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "store $src, $addr",
                  [(store i32:$src, addr:$addr)]>;

def STOREri : Inst32MD2I<32,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "store $src, $addr",
                  [(store i32:$src, addr:$addr)]>;

def STOREWrr : Inst32MD2R<33,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "storew $src, $addr",
                  [(truncstorei16 i32:$src, addr:$addr)]>;

def STOREWri : Inst32MD2I<33,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "storew $src, $addr",
                  [(truncstorei16 i32:$src, addr:$addr)]>;

def STOREBrr : Inst32MD2R<34,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "storeb $src, $addr",
                  [(truncstorei8 i32:$src, addr:$addr)]>;

def STOREBri : Inst32MD2I<34,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "storeb $src, $addr",
                  [(truncstorei8 i32:$src, addr:$addr)]>;

// Local memory
let neverHasSideEffects=1 in {

let mayLoad = 1 in {

def LOADLrr : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "loadl $dst, $addr",
                  []>;

def LOADLri : Inst32MD2I<29,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "loadl $dst, $addr",
                  []>;

def LOADLWrr : Inst32MD2R<30,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "loadlw $dst, $addr",
                  []>;

def LOADLWri : Inst32MD2I<30,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "loadlw $dst, $addr",
                  []>;

def LOADLBrr : Inst32MD2R<31,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "loadlb $dst, $addr",
                  []>;

def LOADLBri : Inst32MD2I<31,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "loadlb $dst, $addr",
                  []>;

}

let mayStore = 1 in {

def STORELrr : Inst32MD2R<32,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "storel $src, $addr",
                  []>;

def STORELri : Inst32MD2I<32,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "storel $src, $addr",
                  []>;

def STORELWrr : Inst32MD2R<33,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "storelw $src, $addr",
                  []>;

def STORELWri : Inst32MD2I<33,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "storelw $src, $addr",
                  []>;

def STORELBrr : Inst32MD2R<34,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "storelb $src, $addr",
                  []>;

def STORELBri : Inst32MD2I<34,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "storelb $src, $addr",
                  []>;

}

}

def LDIri : Inst32MD2I<41,
                  (outs GenericRegs:$dst), (ins i32imm:$src),
                  "ldi $dst, $src",
                  [(set i32:$dst, uimm19:$src)]>;

def MOVrr : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  []>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET  : Inst32MD0<45,
                  (outs), (ins),
                  "ret",
                  [(MDretflag)]>;
}

let isCall = 1 in {
  // All calls clobber the non-callee saved registers. SPW is marked as
  // a use to prevent stack-pointer assignments that appear immediately
  // before calls from potentially appearing dead. Uses for argument
  // registers are added manually.
  let Defs = [R0, R1, R2, R3, R31],
      Uses = [R30] in {
    def CALLi     : Inst32MD1I<21,
                          (outs), (ins i32imm:$dst),
                          "call $dst",
                          [(MDcall uimm19:$dst)]>;

    def CALLr     : Inst32MD1R<21,
                          (outs), (ins GenericRegs:$dst),
                          "call $dst",
                          [(MDcall GenericRegs:$dst)]>;
  }
}

//===----------------------------------------------------------------------===//
// Non-Instruction Patterns
//===----------------------------------------------------------------------===//

// Small immediates.
def : Pat<(i32 uimm19:$val),
          (LDIri imm:$val)>;

// Huge immediates.
def : Pat<(i32 imm:$val),
          (ORrr (LDIri (LO19 imm:$val)), (SHLri (LDIri (HI13 imm:$val)), (i32 13)))>;

// Global addresses, constant pool entries
def : Pat<(MDhigh tglobaladdr:$in), (SHLri (LDIri tglobaladdr:$in), 16)>;
def : Pat<(MDlow tglobaladdr:$in), (LDIri tglobaladdr:$in)>;
def : Pat<(MDhigh tconstpool:$in), (SHLri (LDIri tconstpool:$in), 16)>;
def : Pat<(MDlow tconstpool:$in), (LDIri tconstpool:$in)>;

def : Pat<(add iPTR:$hi, (MDlow tglobaladdr:$lo)), (ADDrr $hi, tglobaladdr:$lo)>;
def : Pat<(add iPTR:$hi, (MDlow tconstpool:$lo)), (ADDrr $hi, tconstpool:$lo)>;
