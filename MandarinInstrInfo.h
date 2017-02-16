//===-- MandarinInstrInfo.h - Mandarin Instruction Information --------*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mandarin implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MANDARININSTRUCTIONINFO_H
#define MANDARININSTRUCTIONINFO_H

#include "MandarinRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "MandarinGenInstrInfo.inc"

namespace llvm {

/// MDII - This namespace holds all of the target specific flags that
/// instruction info tracks.
///
namespace MDII {
	enum {
		Pseudo = (1<<0),
		Load = (1<<1),
		Store = (1<<2),
		DelaySlot = (1<<3)
	};

	/// Target Operand Flags. Mandarin specific TargetFlags for MachineOperands and SDNodes.
	enum TOF {
		MO_NO_FLAG,

		MO_HI32,
		MO_HI24,
		MO_HI16,
		MO_HI8,
		MO_LO32,
		MO_LO24,
		MO_LO16,
		MO_LO8,
	};
}

class MandarinInstrInfo : public MandarinGenInstrInfo {
  const MandarinRegisterInfo RI;
  const MandarinSubtarget& Subtarget;
  virtual void anchor();
public:
  explicit MandarinInstrInfo(MandarinSubtarget &ST);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  virtual const MandarinRegisterInfo &getRegisterInfo() const { return RI; }

  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  virtual unsigned isLoadFromStackSlot(const MachineInstr *MI,
                                       int &FrameIndex) const;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  virtual unsigned isStoreToStackSlot(const MachineInstr *MI,
                                      int &FrameIndex) const;

  virtual bool AnalyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                             MachineBasicBlock *&FBB,
                             SmallVectorImpl<MachineOperand> &Cond,
                             bool AllowModify = false) const ;

  virtual unsigned RemoveBranch(MachineBasicBlock &MBB) const;

  virtual unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                MachineBasicBlock *FBB,
                                const SmallVectorImpl<MachineOperand> &Cond,
                                DebugLoc DL) const;

  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, DebugLoc DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const;

  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   unsigned SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    unsigned DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const;
};

}

#endif
