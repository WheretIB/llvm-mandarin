//===-- MandarinFrameLowering.cpp - Mandarin Frame Information ------------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mandarin implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MandarinFrameLowering.h"
#include "MandarinInstrInfo.h"
#include "MandarinMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

void MandarinFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MandarinMachineFunctionInfo *FuncInfo = MF.getInfo<MandarinMachineFunctionInfo>();
  const MandarinInstrInfo &TII =
    *static_cast<const MandarinInstrInfo*>(MF.getTarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo
  uint64_t StackSize = MFI->getStackSize();

  if (hasFP(MF)) {
	  MFI->setOffsetAdjustment(-StackSize);

	  // Save current frame pointer
	  BuildMI(MBB, MBBI, DL, TII.get(MD::STORELri), MD::R31).addImm(0);
	  
	  // Update frame pointer
	  BuildMI(MBB, MBBI, DL, TII.get(MD::MOVrr), MD::R31).addReg(MD::R30);

	  // Mark the FramePtr as live-in in every block except the entry.
    for (MachineFunction::iterator I = llvm::next(MF.begin()), E = MF.end();
         I != E; ++I)
      I->addLiveIn(MD::R31);
  }

  if (StackSize) {
	  MachineInstr *MI = BuildMI(MBB, MBBI, DL, TII.get(MD::ADDri), MD::R30).addReg(MD::R30).addImm(StackSize);
  }
}

void MandarinFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const
{
	const MandarinInstrInfo &TII =
		*static_cast<const MandarinInstrInfo*>(MF.getTarget().getInstrInfo());

  if (!hasReservedCallFrame(MF))
  {
    MachineInstr &MI = *I;
    int Size = MI.getOperand(0).getImm();

	if (Size)
	{
		if (MI.getOpcode() == MD::ADJCALLSTACKDOWN)
		{
			BuildMI(MF, MI.getDebugLoc(), TII.get(MD::SUBri), MD::R30).addReg(MD::R30).addImm(Size);
		}else{
			BuildMI(MF, MI.getDebugLoc(), TII.get(MD::ADDri), MD::R30).addReg(MD::R30).addImm(Size);
		}
	}
  }

  MBB.erase(I);
}

void MandarinFrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  MandarinMachineFunctionInfo *FuncInfo = MF.getInfo<MandarinMachineFunctionInfo>();
  const MandarinInstrInfo &TII =
    *static_cast<const MandarinInstrInfo*>(MF.getTarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  unsigned RetOpcode = MBBI->getOpcode();
  DebugLoc DL = MBBI->getDebugLoc();

  assert(RetOpcode == MD::RET &&
         "Can only put epilog before 'ret' instruction!");

  uint64_t StackSize = MFI->getStackSize();

  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(MD::LOADLri), MD::R31).addImm(0);
  }

  if (MFI->hasVarSizedObjects()) {
    BuildMI(MBB, MBBI, DL, TII.get(MD::MOVrr), MD::R30).addReg(MD::R31);
  } else {
    if (StackSize) {
      MachineInstr *MI =
        BuildMI(MBB, MBBI, DL, TII.get(MD::SUBri), MD::R30)
        .addReg(MD::R30).addImm(StackSize);
    }
  }
}

bool MandarinFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  // Reserve call frame if there are no variable sized objects on the stack.
  return !MF.getFrameInfo()->hasVarSizedObjects();
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool MandarinFrameLowering::hasFP(const MachineFunction &MF) const {
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
    MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken();
}
