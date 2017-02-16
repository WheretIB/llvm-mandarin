//===-- MandarinISelLowering.cpp - Mandarin DAG Lowering Implementation ---------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file implements the interfaces that Mandarin uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "MandarinISelLowering.h"
#include "MandarinMachineFunctionInfo.h"
#include "MandarinRegisterInfo.h"
#include "MandarinTargetMachine.h"
#include "MCTargetDesc/MandarinBaseInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

#include "MandarinGenCallingConv.inc"

/// For each argument in a function store the number of pieces it is composed
/// of.
template<typename ArgT>
static void ParseFunctionArgs(const SmallVectorImpl<ArgT> &Args,
                              SmallVectorImpl<unsigned> &Out) {
  unsigned CurrentArgIndex = ~0U;
  for (unsigned i = 0, e = Args.size(); i != e; i++) {
    if (CurrentArgIndex == Args[i].OrigArgIndex) {
      Out.back()++;
    } else {
      Out.push_back(1);
      CurrentArgIndex++;
    }
  }
}

static void AnalyzeVarArgs(CCState &State,
                           const SmallVectorImpl<ISD::OutputArg> &Outs) {
  State.AnalyzeCallOperands(Outs, CC_Mandarin_AssignStack);
}

static void AnalyzeVarArgs(CCState &State,
                           const SmallVectorImpl<ISD::InputArg> &Ins) {
  State.AnalyzeFormalArguments(Ins, CC_Mandarin_AssignStack);
}

template<typename ArgT>
static void AnalyzeArguments(CCState &State,
                             SmallVectorImpl<CCValAssign> &ArgLocs,
                             const SmallVectorImpl<ArgT> &Args) {
  static const uint16_t RegList[] = {
    MD::R0, MD::R1, MD::R2, MD::R3
  };
  static const unsigned NbRegs = array_lengthof(RegList);

  if (State.isVarArg()) {
    AnalyzeVarArgs(State, Args);
    return;
  }

  SmallVector<unsigned, 4> ArgsParts;
  ParseFunctionArgs(Args, ArgsParts);

  unsigned RegsLeft = NbRegs;
  bool UseStack = false;
  unsigned ValNo = 0;

  for (unsigned i = 0, e = ArgsParts.size(); i != e; i++) {
    MVT ArgVT = Args[ValNo].VT;
    ISD::ArgFlagsTy ArgFlags = Args[ValNo].Flags;
    MVT LocVT = ArgVT;
    CCValAssign::LocInfo LocInfo = CCValAssign::Full;

    // Handle byval arguments
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo++, ArgVT, LocVT, LocInfo, 2, 2, ArgFlags);
      continue;
    }

    unsigned Parts = ArgsParts[i];

    if (!UseStack && Parts <= RegsLeft) {
      unsigned FirstVal = ValNo;
      for (unsigned j = 0; j < Parts; j++) {
        unsigned Reg = State.AllocateReg(RegList, NbRegs);
        State.addLoc(CCValAssign::getReg(ValNo++, ArgVT, Reg, LocVT, LocInfo));
        RegsLeft--;
      }
    } else {
      UseStack = true;
      for (unsigned j = 0; j < Parts; j++)
	  {
		  if (ArgFlags.isByVal()) {
			State.HandleByVal(ValNo++, ArgVT, LocVT, LocInfo, 4, 4, ArgFlags);
			continue;
		  }

		  if (LocVT == MVT::i32 || LocVT == MVT::f32) {
			unsigned Offset1 = State.AllocateStack(4, 4);
			State.addLoc(CCValAssign::getMem(ValNo++, ArgVT, Offset1, LocVT, LocInfo));
			continue;
		  }
	  }
    }
  }
}

SDValue
MandarinTargetLowering::LowerReturn(SDValue Chain,
                                 CallingConv::ID CallConv, bool IsVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 SDLoc DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();

  // CCValAssign - represent the assignment of the return value to locations.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                 DAG.getTarget(), RVLocs, *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Mandarin);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(),
                             OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(MDISD::RET_FLAG, DL, MVT::Other, &RetOps[0], RetOps.size());
}

SDValue MandarinTargetLowering::
LowerFormalArguments(SDValue Chain,
                     CallingConv::ID CallConv,
                     bool IsVarArg,
                     const SmallVectorImpl<ISD::InputArg> &Ins,
                     SDLoc DL,
                     SelectionDAG &DAG,
                     SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  MandarinMachineFunctionInfo *FuncInfo = MF.getInfo<MandarinMachineFunctionInfo>();

  /*printf("LowerFormalArguments()\n");
  printf("CallConv %d\n", CallConv);
  printf("IsVarArg %d\n", IsVarArg);

  printf("Ins size %d\n", Ins.size());
  for(unsigned i = 0; i < Ins.size(); i++)
  {
	  printf("Ins[%d].ArgVT %d\n", i, Ins[i].ArgVT);
	  printf("Ins[%d].Flags %d\n", i, Ins[i].Flags);
	  printf("Ins[%d].OrigArgIndex %d\n", i, Ins[i].OrigArgIndex);
	  printf("Ins[%d].PartOffset %d\n", i, Ins[i].PartOffset);
	  printf("Ins[%d].Used %d\n", i, Ins[i].Used);
	  printf("Ins[%d].VT %d\n", i, Ins[i].VT);
  }

  DL.getDebugLoc().dump(*DAG.getContext());
  printf("InVals size %d\n", InVals.size());
  for(unsigned i = 0; i < InVals.size(); i++)
	  InVals[i].dump();*/

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  AnalyzeArguments(CCInfo, ArgLocs, Ins);

  /*printf("ArgLocs size %d\n", ArgLocs.size());
  for(unsigned i = 0; i < ArgLocs.size(); i++)
  {
	  printf("ArgLocs[%d].getLocVT %d\n", i, ArgLocs[i].getLocVT());
	  printf("ArgLocs[%d].getLocReg %d\n", i, ArgLocs[i].getLocReg());
	  printf("ArgLocs[%d].isMemLoc %d\n", i, ArgLocs[i].isMemLoc());
	  printf("ArgLocs[%d].isRegLoc %d\n", i, ArgLocs[i].isRegLoc());
  }*/

  // Create frame index for the start of the first vararg value
  if (IsVarArg) {
    assert(!"implemented");
  }

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();

	  unsigned VReg;
	  SDValue ArgValue;

      switch (RegVT.getSimpleVT().SimpleTy) {
      default:
          llvm_unreachable(0);
      case MVT::i32:
	  case MVT::f32:
        VReg = RegInfo.createVirtualRegister(&MD::GenericRegsRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        InVals.push_back(ArgValue);
		break;
	  case MVT::v2i32:
	  case MVT::v2f32:
        VReg = RegInfo.createVirtualRegister(&MD::DoubleRegsRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        InVals.push_back(ArgValue);
		break;
	  case MVT::v4i32:
	  case MVT::v4f32:
        VReg = RegInfo.createVirtualRegister(&MD::QuadRegsRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        InVals.push_back(ArgValue);
		break;
      }
    } else {
      // Sanity check
      assert(VA.isMemLoc());

      SDValue InVal;
      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) {
        int FI = MFI->CreateFixedObject(Flags.getByValSize(),
                                        VA.getLocMemOffset(), true);
        InVal = DAG.getFrameIndex(FI, getPointerTy());
      } else {
        // Load the argument to a virtual register
        unsigned ObjSize = VA.getLocVT().getSizeInBits() / 32;
        if (ObjSize > 1) {
            llvm_unreachable(0);
        }
        // Create the frame index object for this incoming parameter...
        int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

        // Create the SelectionDAG nodes corresponding to a load
        // from this parameter
        SDValue FIN = DAG.getFrameIndex(FI, VA.getLocVT());
        InVal = DAG.getLoad(VA.getLocVT(), DL, Chain, FIN,
                            MachinePointerInfo::getFixedStack(FI),
                            false, false, false, 0);
      }

      InVals.push_back(InVal);
    }
  }

  return Chain;
}

SDValue
MandarinTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc &dl                             = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  // Mandarin target does not yet support tail call optimization.
  isTailCall = false;

  // functions arguments are copied from virtual regs to (physical regs)/(stack frame)
  // CALLSEQ_START and CALLSEQ_END are emitted.
  // TODO: sret.

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  AnalyzeArguments(CCInfo, ArgLocs, Outs);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain ,DAG.getConstant(NumBytes, getPointerTy(), true), dl);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl, MD::R30, getPointerTy());

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
                                   StackPtr,
                                   DAG.getIntPtrConstant(VA.getLocMemOffset()));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) {
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i32);
        MemOp = DAG.getMemcpy(Chain, dl, PtrOff, Arg, SizeNode,
                              Flags.getByValAlign(),
                              /*isVolatile*/false,
                              /*AlwaysInline=*/true,
                              MachinePointerInfo(),
                              MachinePointerInfo());
      } else {
        MemOp = DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo(),
                             false, false, 0);
      }

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
                        &MemOpChains[0], MemOpChains.size());

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(MDISD::CALL, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag, dl);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl,
                         DAG, InVals);
}

static void AnalyzeRetResult(CCState &State,
                             const SmallVectorImpl<ISD::InputArg> &Ins) {
  State.AnalyzeCallResult(Ins, RetCC_Mandarin);
}

static void AnalyzeRetResult(CCState &State,
                             const SmallVectorImpl<ISD::OutputArg> &Outs) {
  State.AnalyzeReturn(Outs, RetCC_Mandarin);
}

template<typename ArgT>
static void AnalyzeReturnValues(CCState &State,
                                SmallVectorImpl<CCValAssign> &RVLocs,
                                const SmallVectorImpl<ArgT> &Args) {
  AnalyzeRetResult(State, Args);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue
MandarinTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                      CallingConv::ID CallConv, bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                      SDLoc dl, SelectionDAG &DAG,
                                      SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  //CCInfo.AnalyzeReturn(Ins, RetCC_Mandarin);
  AnalyzeReturnValues(CCInfo, RVLocs, Ins);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
// TargetLowering Implementation
//===----------------------------------------------------------------------===//

// IntCondCCodeToICC - Convert a DAG integer condition code to a Mandarin CC condition.
static MDCC::CondCodes DAGIntCCToMDCC(ISD::CondCode CC) {
  switch (CC) {
  default:
	  // TODO: should return signed/unsigned operation flag
	  llvm_unreachable("Unknown integer condition code!");
  case ISD::SETEQ:  return MDCC::COND_EQ;
  case ISD::SETNE:  return MDCC::COND_NE;
  case ISD::SETLT:  return MDCC::COND_LS;
  case ISD::SETGT:  return MDCC::COND_GR;
  case ISD::SETLE:  return MDCC::COND_LE;
  case ISD::SETGE:  return MDCC::COND_GE;
  }
}

// FPCondCCodeToFCC - Convert a DAG floatingp oint condition code to a Mandarin CC condition.
static MDCC::CondCodes DAGFloatCCToMDCC(ISD::CondCode CC) {
  switch (CC) {
  default:
	  llvm_unreachable("Unknown fp condition code!");
  case ISD::SETEQ:
  case ISD::SETOEQ: return MDCC::COND_EQ;
  case ISD::SETNE:
  case ISD::SETUNE: return MDCC::COND_NE;
  case ISD::SETLT:
  case ISD::SETOLT: return MDCC::COND_LS;
  case ISD::SETGT:
  case ISD::SETOGT: return MDCC::COND_GR;
  case ISD::SETLE:
  case ISD::SETOLE: return MDCC::COND_LE;
  case ISD::SETGE:
  case ISD::SETOGE: return MDCC::COND_GE;
  }
}

MandarinTargetLowering::MandarinTargetLowering(TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileCOFF()) {
  Subtarget = &TM.getSubtarget<MandarinSubtarget>();

  // Set up the register classes.
  addRegisterClass(MVT::i32, &MD::GenericRegsRegClass);
  addRegisterClass(MVT::f32, &MD::GenericRegsRegClass);
  addRegisterClass(MVT::v2i32, &MD::DoubleRegsRegClass);
  addRegisterClass(MVT::v2f32, &MD::DoubleRegsRegClass);
  addRegisterClass(MVT::v4i32, &MD::QuadRegsRegClass);
  addRegisterClass(MVT::v4f32, &MD::QuadRegsRegClass);

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, Expand);

  // Mandarin doesn't have i1 loads
  setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);

  // Turn FP truncstore into trunc + store.
  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::f128, MVT::f32, Expand);

  setOperationAction(ISD::GlobalAddress, getPointerTy(), Custom);
  setOperationAction(ISD::ConstantPool, getPointerTy(), Custom);
  //setOperationAction(ISD::BlockAddress, getPointerTy(), Custom);

  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8 , Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1 , Expand);

  setOperationAction(ISD::UDIV, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);

  setOperationAction(ISD::FP_TO_UINT, MVT::i32, Expand);
  setOperationAction(ISD::UINT_TO_FP, MVT::i32, Expand);

  setOperationAction(ISD::BITCAST, MVT::f32, Expand);
  setOperationAction(ISD::BITCAST, MVT::i32, Expand);

  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Expand);

  setOperationAction(ISD::SETCC, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::f32, Expand);

  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_CC, MVT::f32, Custom);

  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Custom);

  setOperationAction(ISD::ATOMIC_FENCE, MVT::Other, Expand);

  setOperationAction(ISD::FSIN , MVT::f128, Expand);
  setOperationAction(ISD::FCOS , MVT::f128, Expand);
  setOperationAction(ISD::FSINCOS, MVT::f128, Expand);
  setOperationAction(ISD::FREM , MVT::f128, Expand);
  setOperationAction(ISD::FMA  , MVT::f128, Expand);
  setOperationAction(ISD::FSIN , MVT::f64, Expand);
  setOperationAction(ISD::FCOS , MVT::f64, Expand);
  setOperationAction(ISD::FSINCOS, MVT::f64, Expand);
  setOperationAction(ISD::FREM , MVT::f64, Expand);
  setOperationAction(ISD::FMA  , MVT::f64, Expand);
  setOperationAction(ISD::FSIN , MVT::f32, Expand);
  setOperationAction(ISD::FCOS , MVT::f32, Expand);
  setOperationAction(ISD::FSINCOS, MVT::f32, Expand);
  setOperationAction(ISD::FREM , MVT::f32, Expand);
  setOperationAction(ISD::FMA  , MVT::f32, Expand);
  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ , MVT::i32, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ , MVT::i32, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i32, Expand);
  //setOperationAction(ISD::SRA  , MVT::i32, Expand);
  setOperationAction(ISD::ROTL , MVT::i32, Expand);
  setOperationAction(ISD::ROTR , MVT::i32, Expand);
  setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  setOperationAction(ISD::FCOPYSIGN, MVT::f128, Expand);
  setOperationAction(ISD::FCOPYSIGN, MVT::f64, Expand);
  setOperationAction(ISD::FCOPYSIGN, MVT::f32, Expand);
  setOperationAction(ISD::FPOW , MVT::f128, Expand);
  setOperationAction(ISD::FPOW , MVT::f64, Expand);
  setOperationAction(ISD::FPOW , MVT::f32, Expand);

  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);

  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);

  // VASTART needs to be custom lowered to use the VarArgsFrameIndex.
  setOperationAction(ISD::VASTART           , MVT::Other, Custom);
  // VAARG needs to be lowered to not do unaligned accesses for doubles.
  setOperationAction(ISD::VAARG             , MVT::Other, Custom);

  // Use the default implementation.
  setOperationAction(ISD::VACOPY            , MVT::Other, Expand);
  setOperationAction(ISD::VAEND             , MVT::Other, Expand);
  setOperationAction(ISD::STACKSAVE         , MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE      , MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32  , Custom);

  setOperationAction(ISD::BUILD_VECTOR      , MVT::v2i32, Expand);
  setOperationAction(ISD::BUILD_VECTOR      , MVT::v4i32, Expand);
  setOperationAction(ISD::BUILD_VECTOR      , MVT::v2f32, Expand);
  setOperationAction(ISD::BUILD_VECTOR      , MVT::v4f32, Expand);
  setOperationAction(ISD::VECTOR_SHUFFLE    , MVT::v2i32, Expand);
  setOperationAction(ISD::VECTOR_SHUFFLE    , MVT::v4i32, Expand);
  setOperationAction(ISD::VECTOR_SHUFFLE    , MVT::v2f32, Expand);
  setOperationAction(ISD::VECTOR_SHUFFLE    , MVT::v4f32, Expand);
  setOperationAction(ISD::INSERT_VECTOR_ELT , MVT::v2i32, Expand);
  setOperationAction(ISD::INSERT_VECTOR_ELT , MVT::v4i32, Expand);
  setOperationAction(ISD::INSERT_VECTOR_ELT , MVT::v2f32, Expand);
  setOperationAction(ISD::INSERT_VECTOR_ELT , MVT::v4f32, Expand);

  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2i32, Custom);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4i32, Custom);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2f32, Custom);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v4f32, Custom);

  setMinFunctionAlignment(2);

  setIntDivIsCheap();
  setPow2DivIsCheap();

  computeRegisterProperties();
}

const char *MandarinTargetLowering::getTargetNodeName(unsigned Opcode) const {
	switch (Opcode) {
	default: return 0;
	case MDISD::RET_FLAG:
		return "MDISD::RET_FLAG";
	case MDISD::CMP:
		return "MDISD::CMP";
	case MDISD::ICMP:
		return "MDISD::ICMP";
	case MDISD::FCMP:
		return "MDISD::FCMP";
	case MDISD::BR_CC:
		return "MDISD::BR_CC";
	case MDISD::SELECT_CC:
		return "MDISD::SELECT_CC";
	case MDISD::HIGH:
		return "MDISD::HIGH";
	case MDISD::LOW:
		return "MDISD::LOW";
	case MDISD::CALL:
		return "MDISD::CALL";
	}
}

EVT MandarinTargetLowering::getSetCCResultType(LLVMContext &, EVT VT) const {
  if (!VT.isVector())
    return MVT::i32;
  return VT.changeVectorElementTypeToInteger();
}

/// isMaskedValueZeroForTargetNode - Return true if 'Op & Mask' is known to
/// be zero. Op is expected to be a target specific node. Used by DAG
/// combiner.
void MandarinTargetLowering::computeMaskedBitsForTargetNode
                                (const SDValue Op,
                                 APInt &KnownZero,
                                 APInt &KnownOne,
                                 const SelectionDAG &DAG,
                                 unsigned Depth) const {
  APInt KnownZero2, KnownOne2;
  KnownZero = KnownOne = APInt(KnownZero.getBitWidth(), 0);

  switch (Op.getOpcode()) {
  default: break;
  case MDISD::SELECT_CC:
    DAG.ComputeMaskedBits(Op.getOperand(1), KnownZero, KnownOne, Depth+1);
    DAG.ComputeMaskedBits(Op.getOperand(0), KnownZero2, KnownOne2, Depth+1);
    assert((KnownZero & KnownOne) == 0 && "Bits known to be one AND zero?");
    assert((KnownZero2 & KnownOne2) == 0 && "Bits known to be one AND zero?");

    // Only known if known in both the LHS and RHS.
    KnownOne &= KnownOne2;
    KnownZero &= KnownZero2;
    break;
  }
}

// Convert to a target node and set target flags.
SDValue MandarinTargetLowering::withTargetFlags(SDValue Op, unsigned TF,
                                             SelectionDAG &DAG) const
{
  if (const GlobalAddressSDNode *GA = dyn_cast<GlobalAddressSDNode>(Op))
    return DAG.getTargetGlobalAddress(GA->getGlobal(),
                                      SDLoc(GA),
                                      GA->getValueType(0),
                                      GA->getOffset(), TF);

  if (const ConstantPoolSDNode *CP = dyn_cast<ConstantPoolSDNode>(Op))
    return DAG.getTargetConstantPool(CP->getConstVal(),
                                     CP->getValueType(0),
                                     CP->getAlignment(),
                                     CP->getOffset(), TF);

  if (const BlockAddressSDNode *BA = dyn_cast<BlockAddressSDNode>(Op))
    return DAG.getTargetBlockAddress(BA->getBlockAddress(),
                                     Op.getValueType(),
                                     0,
                                     TF);

  if (const ExternalSymbolSDNode *ES = dyn_cast<ExternalSymbolSDNode>(Op))
    return DAG.getTargetExternalSymbol(ES->getSymbol(),
                                       ES->getValueType(0), TF);

  llvm_unreachable("Unhandled address SDNode");
}


// Build SDNodes for producing an address from a GlobalAddress, ConstantPool,
// or ExternalSymbol SDNode.
SDValue MandarinTargetLowering::LowerAddress(SDValue Op, SelectionDAG &DAG) const
{
	SDLoc DL(Op);
	EVT VT = getPointerTy();

    if (getTargetMachine().getRelocationModel() != Reloc::PIC_)
	{
		// %hi/%lo relocation
		// This is a generic model, but we will use a simplified one
		/*SDValue HiPart = DAG.getNode(MDISD::HIGH, DL, VT, withTargetFlags(Op, MDII::MO_HI16, DAG));
		SDValue LoPart = DAG.getNode(MDISD::LOW, DL, VT, withTargetFlags(Op, MDII::MO_LO16, DAG));
		return DAG.getNode(ISD::ADD, DL, VT, HiPart, LoPart);*/

		// 64kb
		return DAG.getNode(MDISD::LOW, DL, VT, withTargetFlags(Op, MDII::MO_LO16, DAG));
	}
  
	llvm_unreachable("Unsupported absolute code model");
}

static SDValue EmitCMP(SDValue &LHS, SDValue &RHS, SDValue &TargetCC,
                       ISD::CondCode CC,
                       SDLoc dl, SelectionDAG &DAG) {

  MDCC::CondCodes TCC = MDCC::COND_INVALID;

  // TODO: I have no idea how unsigned operations are performed

  switch (CC) {
  default:
	  llvm_unreachable("Invalid condition!");
  case ISD::SETEQ:
    TCC = MDCC::COND_EQ;
    // Minor optimization: if LHS is a constant, swap operands, then the constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant)
      std::swap(LHS, RHS);
    break;
  case ISD::SETGT:
    TCC = MDCC::COND_GR;
    break;
  case ISD::SETGE:
    TCC = MDCC::COND_GE;
    break;
  case ISD::SETLT:
    TCC = MDCC::COND_LS;
    break;
  case ISD::SETLE:
    TCC = MDCC::COND_LE;
    break;
  case ISD::SETNE:
    TCC = MDCC::COND_NE;
    // Minor optimization: if LHS is a constant, swap operands, then the constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant)
      std::swap(LHS, RHS);
    break;
  }

  TargetCC = DAG.getConstant(TCC, MVT::i32);

  if(LHS.getValueType().isFloatingPoint())
	  return DAG.getNode(MDISD::FCMP, dl, MVT::Glue, LHS, RHS);

  return DAG.getNode(MDISD::ICMP, dl, MVT::Glue, LHS, RHS);
}

SDValue MandarinTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const
{
	SDValue Chain = Op.getOperand(0);
	ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
	SDValue LHS   = Op.getOperand(2);
	SDValue RHS   = Op.getOperand(3);
	SDValue Dest  = Op.getOperand(4);
	SDLoc dl  (Op);

	SDValue TargetCC;
    SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

	return DAG.getNode(MDISD::BR_CC, dl, Op.getValueType(),
                     Chain, Dest, TargetCC, Flag);
}

SDValue MandarinTargetLowering::LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const
{
	SDValue LHS = Op.getOperand(0);
	SDValue RHS = Op.getOperand(1);
	ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
	SDValue TrueVal = Op.getOperand(2);
	SDValue FalseVal = Op.getOperand(3);
	SDLoc dl(Op);

	MDCC::CondCodes MDCC = MDCC::COND_INVALID;

	SDValue CompareFlag;
	if (LHS.getValueType().isInteger())
	{
		CompareFlag = DAG.getNode(MDISD::ICMP, dl, MVT::Glue, LHS, RHS);
		MDCC = DAGIntCCToMDCC(CC);
	} else {
		CompareFlag = DAG.getNode(MDISD::FCMP, dl, MVT::Glue, LHS, RHS);
		MDCC = DAGFloatCCToMDCC(CC);
	}
	return DAG.getNode(MDISD::SELECT_CC, dl, TrueVal.getValueType(), TrueVal, FalseVal,
						DAG.getConstant(MDCC, MVT::i32), CompareFlag);
}

SDValue MandarinTargetLowering::LowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const
{
	SDValue Value = Op.getOperand(0);
	SDValue Lane = Op.getOperand(1);
	if (!isa<ConstantSDNode>(Lane))
		return SDValue();

	Op.dump();

	Op.getOperand(0).dump();
	Op.getOperand(1).dump();

	//return DAG.getNode(MDISD::;

	return Op;
}

SDValue MandarinTargetLowering::
LowerOperation(SDValue Op, SelectionDAG &DAG) const {

  switch (Op.getOpcode()) {
  default:
	  Op.dump();
	  DAG.dump();
	  llvm_unreachable("Should not custom lower this!");
  case ISD::BR_CC:
	  return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
	  return LowerSELECT_CC(Op, DAG);
  case ISD::EXTRACT_VECTOR_ELT:
	  return LowerEXTRACT_VECTOR_ELT(Op, DAG);
  case ISD::GlobalAddress:
  case ISD::ConstantPool:
	  return LowerAddress(Op, DAG);

  /*case ISD::RETURNADDR:         return LowerRETURNADDR(Op, DAG, *this);
  case ISD::FRAMEADDR:          return LowerFRAMEADDR(Op, DAG);
  case ISD::GlobalTLSAddress:   return LowerGlobalTLSAddress(Op, DAG);
  case ISD::VASTART:            return LowerVASTART(Op, DAG, *this);
  case ISD::VAARG:              return LowerVAARG(Op, DAG);
  case ISD::DYNAMIC_STACKALLOC: return LowerDYNAMIC_STACKALLOC(Op, DAG,
                                                               Subtarget);*/
  }
}

MachineBasicBlock *
MandarinTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                 MachineBasicBlock *BB) const
{
	const TargetInstrInfo &TII = *getTargetMachine().getInstrInfo();
	DebugLoc dl = MI->getDebugLoc();
  
	MDCC::CondCodes CC = (MDCC::CondCodes)MI->getOperand(3).getImm();

	// To "insert" a SELECT_CC instruction, we actually have to insert the diamond
	// control-flow pattern.  The incoming instruction knows the destination vreg
	// to set, the condition code register to branch on, the true/false values to
	// select between

	const BasicBlock *LLVM_BB = BB->getBasicBlock();
	MachineFunction::iterator It = BB;
	++It;

	//  thisMBB:
	//  ...
	//   TrueVal = ...
	//   jCC copy1MBB
	//   fallthrough --> copy0MBB
	MachineBasicBlock *thisMBB = BB;
	MachineFunction *F = BB->getParent();
	MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
	MachineBasicBlock *sinkMBB = F->CreateMachineBasicBlock(LLVM_BB);
	F->insert(It, copy0MBB);
	F->insert(It, sinkMBB);

	// Transfer the remainder of BB and its successor edges to sinkMBB.
	sinkMBB->splice(sinkMBB->begin(), BB,
					llvm::next(MachineBasicBlock::iterator(MI)),
					BB->end());
	sinkMBB->transferSuccessorsAndUpdatePHIs(BB);

	// Add the true and fallthrough blocks as its successors.
	BB->addSuccessor(copy0MBB);
	BB->addSuccessor(sinkMBB);

	BuildMI(BB, dl, TII.get(MD::JCCi)).addMBB(sinkMBB).addImm(CC);

	//  copy0MBB:
	//   %FalseValue = ...
	//   # fallthrough to sinkMBB
	BB = copy0MBB;

	// Update machine-CFG edges
	BB->addSuccessor(sinkMBB);

	//  sinkMBB:
	//   %Result = phi [ %FalseValue, copy0MBB ], [ %TrueValue, thisMBB ]
	//  ...
	BB = sinkMBB;
	BuildMI(*BB, BB->begin(), dl, TII.get(MD::PHI), MI->getOperand(0).getReg())
		.addReg(MI->getOperand(2).getReg()).addMBB(copy0MBB)
		.addReg(MI->getOperand(1).getReg()).addMBB(thisMBB);

	MI->eraseFromParent();   // The pseudo instruction is gone now.

	return BB;
}

//===----------------------------------------------------------------------===//
//                         Mandarin Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
MandarinTargetLowering::ConstraintType
MandarinTargetLowering::getConstraintType(const std::string &Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:  break;
    case 'r': return C_RegisterClass;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass*>
MandarinTargetLowering::getRegForInlineAsmConstraint(const std::string &Constraint,
                                                  MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return std::make_pair(0U, &MD::GenericRegsRegClass);
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}

bool
MandarinTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  // The Mandarin target isn't yet aware of offsets.
  return false;
}

void MandarinTargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue>& Results,
                                             SelectionDAG &DAG) const {

  SDLoc dl(N);

  RTLIB::Libcall libCall = RTLIB::UNKNOWN_LIBCALL;

  switch (N->getOpcode()) {
  default:
    llvm_unreachable("Do not know how to custom type legalize this operation!");
    return;
  }
}
