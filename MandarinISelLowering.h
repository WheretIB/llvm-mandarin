//===-- MandarinISelLowering.h - Mandarin DAG Lowering Interface ------*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Mandarin uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef MANDARIN_ISELLOWERING_H
#define MANDARIN_ISELLOWERING_H

#include "Mandarin.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
  class MandarinSubtarget;

  namespace MDISD {
    enum {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

	  /// Return with a flag operand. Operand 0 is the chain operand.
      RET_FLAG,

	  CMP,
	  ICMP,
	  FCMP,

	  BR_CC,
	  SELECT_CC,

	  HIGH,
	  LOW,

	  CALL,
    };
  }

  class MandarinTargetLowering : public TargetLowering {
    const MandarinSubtarget *Subtarget;
  public:
    MandarinTargetLowering(TargetMachine &TM);
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    /// computeMaskedBitsForTargetNode - Determine which of the bits specified
    /// in Mask are known to be either zero or one and return them in the
    /// KnownZero/KnownOne bitsets.
    virtual void computeMaskedBitsForTargetNode(const SDValue Op,
                                                APInt &KnownZero,
                                                APInt &KnownOne,
                                                const SelectionDAG &DAG,
                                                unsigned Depth = 0) const;

	/// Targets can use this to indicate that they only support *some*
	/// VECTOR_SHUFFLE operations, those with specific masks.  By default, if a
	/// target supports the VECTOR_SHUFFLE node, all mask values are assumed to be
	/// legal.
	virtual bool isShuffleMaskLegal(const SmallVectorImpl<int> &/*Mask*/,
									EVT /*VT*/) const
	{
		return false;
	}

    virtual MachineBasicBlock *
      EmitInstrWithCustomInserter(MachineInstr *MI,
                                  MachineBasicBlock *MBB) const;

    virtual const char *getTargetNodeName(unsigned Opcode) const;

    ConstraintType getConstraintType(const std::string &Constraint) const;
    std::pair<unsigned, const TargetRegisterClass*>
    getRegForInlineAsmConstraint(const std::string &Constraint, MVT VT) const;

    virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;
    virtual MVT getScalarShiftAmountTy(EVT LHSTy) const { return MVT::i32; }

    /// getSetCCResultType - Return the ISD::SETCC ValueType
    virtual EVT getSetCCResultType(LLVMContext &Context, EVT VT) const;

    virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv,
                           bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           SDLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerCall(TargetLowering::CallLoweringInfo &CLI,
                SmallVectorImpl<SDValue> &InVals) const;

	SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            SDLoc dl, SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

    virtual SDValue
      LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  SDLoc dl, SelectionDAG &DAG) const;

	SDValue withTargetFlags(SDValue Op, unsigned TF, SelectionDAG &DAG) const;
	SDValue LowerAddress(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
	SDValue LowerEXTRACT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;

    bool ShouldShrinkFPConstant(EVT VT) const {
      // Do not shrink FP constpool if VT == MVT::f128.
      // (ldd, call _Q_fdtoq) is more expensive than two ldds.
      return VT != MVT::f128;
    }

    virtual void ReplaceNodeResults(SDNode *N,
                                    SmallVectorImpl<SDValue>& Results,
                                    SelectionDAG &DAG) const;
  };
} // end namespace llvm

#endif    // MANDARIN_ISELLOWERING_H
