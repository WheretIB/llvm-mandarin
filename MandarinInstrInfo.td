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

def i32imm32  : ImmLeaf<i32, [{ return true; }]>;

def simm14  : PatLeaf<(i32imm32), [{ return isInt<14>(N->getSExtValue()); }]>;
def uimm14  : PatLeaf<(i32imm32), [{ return isUInt<14>(N->getZExtValue()); }]>;

def simm19  : PatLeaf<(i32imm32), [{ return isInt<19>(N->getSExtValue()); }]>;
def uimm19  : PatLeaf<(i32imm32), [{ return isUInt<19>(N->getZExtValue()); }]>;

def simm24  : PatLeaf<(i32imm32), [{ return isInt<24>(N->getSExtValue()); }]>;
def uimm24  : PatLeaf<(i32imm32), [{ return isUInt<24>(N->getZExtValue()); }]>;

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
            [(set i32:$dst, (MDselectcc i32:$T, i32:$F, i32imm32:$Cond))]>;
  def SELECT_CC_Float
   : Pseudo<(outs GenericRegs:$dst), (ins GenericRegs:$T, GenericRegs:$F, i32imm:$Cond),
            "; SELECT_CC_Float PSEUDO!",
            [(set f32:$dst, (MDselectcc f32:$T, f32:$F, i32imm32:$Cond))]>;

}

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

// Inst32MD3Int multiclass - define 3-argument operations with 1/2/4 regs
multiclass Inst32MD3Int<bits<7> operationVal, string OpcStr, SDNode OpNode> {
  def rr  : Inst32MD3R<operationVal,
                 (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                 !strconcat(OpcStr, " $dst, $src1, $src2"),
                 [(set i32:$dst, (OpNode i32:$src1, i32:$src2))]>;

  def 2rr  : Inst32MD3R<operationVal,
                 (outs DoubleRegs:$dst), (ins DoubleRegs:$src1, DoubleRegs:$src2),
                 !strconcat(OpcStr, "[2] $dst, $src1, $src2"),
                 [(set v2i32:$dst, (OpNode v2i32:$src1, v2i32:$src2))]>;

  def 4rr  : Inst32MD3R<operationVal,
                 (outs QuadRegs:$dst), (ins QuadRegs:$src1, QuadRegs:$src2),
                 !strconcat(OpcStr, "[4] $dst, $src1, $src2"),
                 [(set v4i32:$dst, (OpNode v4i32:$src1, v4i32:$src2))]>;
}

// Inst32MD3IntU multiclass - define 3-argument operations with 1/2/4 regs and an unsigned immediate
multiclass Inst32MD3IntU<bits<7> operationVal, string OpcStr, SDNode OpNode>
    : Inst32MD3Int<operationVal, OpcStr, OpNode>
{
  def ri  : Inst32MD3I<operationVal,
                 (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                 !strconcat(OpcStr, " $dst, $src1, $src2"),
                 [(set i32:$dst, (OpNode i32:$src1, uimm14:$src2))]>;
}

// Inst32MD3IntS multiclass - define 3-argument operations with 1/2/4 regs and a signed immediate
multiclass Inst32MD3IntS<bits<7> operationVal, string OpcStr, SDNode OpNode>
    : Inst32MD3Int<operationVal, OpcStr, OpNode>
{
  def ri  : Inst32MD3I<operationVal,
                 (outs GenericRegs:$dst), (ins GenericRegs:$src1, i32imm:$src2),
                 !strconcat(OpcStr, " $dst, $src1, $src2"),
                 [(set i32:$dst, (OpNode i32:$src1, simm14:$src2))]>;
}

// Inst32MD3Float multiclass - define 3-argument operations with 1/2/4 regs
multiclass Inst32MD3Float<bits<7> operationVal, string OpcStr, SDNode OpNode> {
  def rr  : Inst32MD3R<operationVal,
                 (outs GenericRegs:$dst), (ins GenericRegs:$src1, GenericRegs:$src2),
                 !strconcat(OpcStr, " $dst, $src1, $src2"),
                 [(set f32:$dst, (OpNode f32:$src1, f32:$src2))]>;

  def 2rr  : Inst32MD3R<operationVal,
                 (outs DoubleRegs:$dst), (ins DoubleRegs:$src1, DoubleRegs:$src2),
                 !strconcat(OpcStr, "[2] $dst, $src1, $src2"),
                 [(set v2f32:$dst, (OpNode v2f32:$src1, v2f32:$src2))]>;

  def 4rr  : Inst32MD3R<operationVal,
                 (outs QuadRegs:$dst), (ins QuadRegs:$src1, QuadRegs:$src2),
                 !strconcat(OpcStr, "[4] $dst, $src1, $src2"),
                 [(set v4f32:$dst, (OpNode v4f32:$src1, v4f32:$src2))]>;
}


//===----------------------------------------------------------------------===//
// Instructions.
//===----------------------------------------------------------------------===//

defm ADD    : Inst32MD3IntU<0, "add", add>;
defm SUB    : Inst32MD3IntU<1, "sub", sub>;
defm MUL    : Inst32MD3IntU<2, "mul", mul>;
defm DIV    : Inst32MD3IntS<3, "div", sdiv>; // signed
defm MOD    : Inst32MD3IntS<4, "mod", srem>; // signed

defm FADD    : Inst32MD3Float<5, "fadd", fadd>;
defm FSUB    : Inst32MD3Float<6, "fsub", fsub>;
defm FMUL    : Inst32MD3Float<7, "fmul", fmul>;
defm FDIV    : Inst32MD3Float<8, "fdiv", fdiv>;

defm SHL    : Inst32MD3IntU<9, "shl", shl>;
defm SHR    : Inst32MD3IntU<10, "shr", srl>;
defm AND    : Inst32MD3IntU<11, "and", and>;
defm OR     : Inst32MD3IntU<12, "or", or>;
defm XOR    : Inst32MD3IntU<13, "xor", xor>;

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

def NEGrr : Inst32MD2R<20,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "neg $dst, $src",
                  [(set i32:$dst, (ineg i32:$src))]>;

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

def NOTrr : Inst32MD2R<20,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "not $dst, $src",
                  [(set i32:$dst, (not i32:$src))]>;

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

def LOADri : Inst32MD2I<29,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "load $dst, $addr",
                  [(set i32:$dst, (load addr:$addr))]>;

def LOAD2rr : Inst32MD2R<29,
                  (outs DoubleRegs:$dst), (ins GenericRegs:$addr),
                  "load[2] $dst, $addr",
                  [(set v2i32:$dst, (load addr:$addr))]>;

def LOAD2ri : Inst32MD2I<29,
                  (outs DoubleRegs:$dst), (ins i32imm:$addr),
                  "load[2] $dst, $addr",
                  [(set v2i32:$dst, (load addr:$addr))]>;

def LOAD4rr : Inst32MD2R<29,
                  (outs QuadRegs:$dst), (ins GenericRegs:$addr),
                  "load[4] $dst, $addr",
                  [(set v4i32:$dst, (load addr:$addr))]>;

def LOAD4ri : Inst32MD2I<29,
                  (outs QuadRegs:$dst), (ins i32imm:$addr),
                  "load[4] $dst, $addr",
                  [(set v4i32:$dst, (load addr:$addr))]>;

def LOADfrr : Inst32MD2R<29,
                  (outs GenericRegs:$dst), (ins GenericRegs:$addr),
                  "load $dst, $addr",
                  [(set f32:$dst, (load addr:$addr))]>;

def LOADfri : Inst32MD2I<29,
                  (outs GenericRegs:$dst), (ins i32imm:$addr),
                  "load $dst, $addr",
                  [(set f32:$dst, (load addr:$addr))]>;

def LOAD2frr : Inst32MD2R<29,
                  (outs DoubleRegs:$dst), (ins GenericRegs:$addr),
                  "load[2] $dst, $addr",
                  [(set v2f32:$dst, (load addr:$addr))]>;

def LOAD2fri : Inst32MD2I<29,
                  (outs DoubleRegs:$dst), (ins i32imm:$addr),
                  "load[2] $dst, $addr",
                  [(set v2f32:$dst, (load addr:$addr))]>;

def LOAD4frr : Inst32MD2R<29,
                  (outs QuadRegs:$dst), (ins GenericRegs:$addr),
                  "load[4] $dst, $addr",
                  [(set v4f32:$dst, (load addr:$addr))]>;

def LOAD4fri : Inst32MD2I<29,
                  (outs QuadRegs:$dst), (ins i32imm:$addr),
                  "load[4] $dst, $addr",
                  [(set v4f32:$dst, (load addr:$addr))]>;

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

//===----------------------------------------------------------------------===//
// Stores
//===----------------------------------------------------------------------===//

def STORErr : Inst32MD2R<32,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "store $src, $addr",
                  [(store i32:$src, addr:$addr)]>;

def STOREri : Inst32MD2I<32,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "store $src, $addr",
                  [(store i32:$src, addr:$addr)]>;

def STORE2rr : Inst32MD2R<32,
                  (outs), (ins DoubleRegs:$src, GenericRegs:$addr),
                  "store[2] $src, $addr",
                  [(store v2i32:$src, addr:$addr)]>;

def STORE2ri : Inst32MD2R<32,
                  (outs), (ins DoubleRegs:$src, i32imm:$addr),
                  "store[2] $src, $addr",
                  [(store v2i32:$src, addr:$addr)]>;

def STORE4rr : Inst32MD2R<32,
                  (outs), (ins QuadRegs:$src, GenericRegs:$addr),
                  "store[4] $src, $addr",
                  [(store v4i32:$src, addr:$addr)]>;

def STORE4ri : Inst32MD2R<32,
                   (outs), (ins QuadRegs:$src, i32imm:$addr),
                  "store[4] $src, $addr",
                  [(store v4i32:$src, addr:$addr)]>;

def STOREfrr : Inst32MD2R<32,
                  (outs), (ins GenericRegs:$src, GenericRegs:$addr),
                  "store $src, $addr",
                  [(store f32:$src, addr:$addr)]>;

def STOREfri : Inst32MD2R<32,
                  (outs), (ins GenericRegs:$src, i32imm:$addr),
                  "store $src, $addr",
                  [(store f32:$src, addr:$addr)]>;

def STORE2frr : Inst32MD2R<32,
                  (outs), (ins DoubleRegs:$src, GenericRegs:$addr),
                  "store[2] $src, $addr",
                  [(store v2f32:$src, addr:$addr)]>;

def STORE2fri : Inst32MD2R<32,
                  (outs), (ins DoubleRegs:$src, i32imm:$addr),
                  "store[2] $src, $addr",
                  [(store v2f32:$src, addr:$addr)]>;

def STORE4frr : Inst32MD2R<32,
                  (outs), (ins QuadRegs:$src, GenericRegs:$addr),
                  "store[4] $src, $addr",
                  [(store v4f32:$src, addr:$addr)]>;

def STORE4fri : Inst32MD2R<32,
                   (outs), (ins QuadRegs:$src, i32imm:$addr),
                  "store[4] $src, $addr",
                  [(store v4f32:$src, addr:$addr)]>;

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

//===----------------------------------------------------------------------===//
// Immediate loads and moves
//===----------------------------------------------------------------------===//

def LDIri : Inst32MD2I<41,
                  (outs GenericRegs:$dst), (ins i32imm:$src),
                  "ldi $dst, $src",
                  [(set i32:$dst, uimm19:$src)]>;

def MOVrr : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  []>;

def MOV2rr : Inst32MD2R<42,
                  (outs DoubleRegs:$dst), (ins DoubleRegs:$src),
                  "mov[2] $dst, $src",
                  []>;

def MOV4rr : Inst32MD2R<42,
                  (outs QuadRegs:$dst), (ins QuadRegs:$src),
                  "mov[4] $dst, $src",
                  []>;

//===----------------------------------------------------------------------===//
// Vector operation wrappings
//===----------------------------------------------------------------------===//

def SCALAR_TO_VECTOR2i : Inst32MD2R<42,
                  (outs DoubleRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  [(set v2i32:$dst, (scalar_to_vector i32:$src))]>;

def SCALAR_TO_VECTOR4i : Inst32MD2R<42,
                  (outs QuadRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  [(set v4i32:$dst, (scalar_to_vector i32:$src))]>;

def SCALAR_TO_VECTOR2f : Inst32MD2R<42,
                  (outs DoubleRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  [(set v2f32:$dst, (scalar_to_vector f32:$src))]>;

def SCALAR_TO_VECTOR4f : Inst32MD2R<42,
                  (outs QuadRegs:$dst), (ins GenericRegs:$src),
                  "mov $dst, $src",
                  [(set v4f32:$dst, (scalar_to_vector f32:$src))]>;
/*
class QuadRegOff<string off> : Operand<v4i32> {
  let PrintMethod =  !strconcat("printQuadRegOff", off);
}
def QuadRegOff1 : QuadRegOff<"1">{}

def EXTRACTi_AT_0 : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins QuadRegs:$src),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v4i32:$src, 0))]>;

def EXTRACTi_AT_1 : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins QuadRegOff<"<1>">:$src),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v4i32:$src, 1))]>;

def EXTRACTi_AT_2 : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins QuadRegOff<"<2>">:$src),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v4i32:$src, 2))]>;

def EXTRACTi_AT_3 : Inst32MD2R<42,
                  (outs GenericRegs:$dst), (ins QuadRegOff<"<3>">:$src),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v4i32:$src, 3))]>;
*/

def QuadRegOffI : Operand<v4i32> {
  let PrintMethod =  "printRegOff";
}

def QuadRegOffF : Operand<v4f32> {
  let PrintMethod =  "printRegOff";
}

def DoubleRegOffI : Operand<v2i32> {
  let PrintMethod =  "printRegOff";
}

def DoubleRegOffF : Operand<v2f32> {
  let PrintMethod =  "printRegOff";
}

def EXTRACT_VECTOR_ELT2i : Pseudo<
                  (outs GenericRegs:$dst), (ins DoubleRegOffI:$src, i32imm:$offset),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v2i32:$src, imm:$offset))]>;

def EXTRACT_VECTOR_ELT4i : Pseudo<
                  (outs GenericRegs:$dst), (ins QuadRegOffI:$src, i32imm:$offset),
                  "mov $dst, $src",
                  [(set i32:$dst, (extractelt v4i32:$src, imm:$offset))]>;

def EXTRACT_VECTOR_ELT2f : Pseudo<
                  (outs GenericRegs:$dst), (ins DoubleRegOffF:$src, i32imm:$offset),
                  "mov $dst, $src",
                  [(set f32:$dst, (extractelt v2f32:$src, imm:$offset))]>;

def EXTRACT_VECTOR_ELT4f : Pseudo<
                  (outs GenericRegs:$dst), (ins QuadRegOffF:$src, i32imm:$offset),
                  "mov $dst, $src",
                  [(set f32:$dst, (extractelt v4f32:$src, imm:$offset))]>;

//===----------------------------------------------------------------------===//
// Function return and call
//===----------------------------------------------------------------------===//

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
          (ORrr (LDIri (LO19 imm:$val)), (SHLri (LDIri (HI13 imm:$val)), (i32 19)))>;

// Global addresses, constant pool entries
def : Pat<(MDhigh tglobaladdr:$in), (SHLri (LDIri tglobaladdr:$in), 16)>;
def : Pat<(MDlow tglobaladdr:$in), (LDIri tglobaladdr:$in)>;
def : Pat<(MDhigh tconstpool:$in), (SHLri (LDIri tconstpool:$in), 16)>;
def : Pat<(MDlow tconstpool:$in), (LDIri tconstpool:$in)>;

def : Pat<(add iPTR:$hi, (MDlow tglobaladdr:$lo)), (ADDrr $hi, tglobaladdr:$lo)>;
def : Pat<(add iPTR:$hi, (MDlow tconstpool:$lo)), (ADDrr $hi, tconstpool:$lo)>;
