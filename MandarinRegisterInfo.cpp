//===-- MandarinRegisterInfo.cpp - Mandarin Register Information ----------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mandarin implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "MandarinRegisterInfo.h"
#include "Mandarin.h"
#include "MandarinMachineFunctionInfo.h"
#include "MandarinTargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "MandarinGenRegisterInfo.inc"

using namespace llvm;

MandarinRegisterInfo::MandarinRegisterInfo(MandarinSubtarget &st)
  : MandarinGenRegisterInfo(MD::R0), Subtarget(st) {
}

const uint16_t* MandarinRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF)
                                                                         const {
  static const uint16_t CalleeSavedRegs[] = {
    0
  };
  return CalleeSavedRegs;
}

BitVector MandarinRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  Reserved.set(MD::R30);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF))
    Reserved.set(MD::R31);

  return Reserved;
}

const TargetRegisterClass*
MandarinRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                      unsigned Kind) const {
  return &MD::GenericRegsRegClass;
}

void
MandarinRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? MD::R31 : MD::R30);
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo()->getStackSize();

  MachineRegisterInfo &RegInfo = MF.getRegInfo();
  const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();

  unsigned VReg = BasePtr;
  
  if(Offset)
  {
	  VReg = RegInfo.createVirtualRegister(&MD::GenericRegsRegClass);

	  if(Offset > 0)
		  BuildMI(MBB, II, dl, TII.get(MD::ADDri), VReg).addReg(BasePtr).addImm(Offset);
	  else
		  BuildMI(MBB, II, dl, TII.get(MD::SUBri), VReg).addReg(BasePtr).addImm(Offset);
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(VReg, false);
}

unsigned MandarinRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? MD::R31 : MD::R30;
}

