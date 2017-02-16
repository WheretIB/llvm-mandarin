//===-- MandarinInstrInfo.cpp - Mandarin Instruction Information ----------------===//
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

#include "MandarinInstrInfo.h"
#include "Mandarin.h"
#include "MandarinMachineFunctionInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "MandarinGenInstrInfo.inc"

using namespace llvm;

// Pin the vtable to this file.
void MandarinInstrInfo::anchor() {}

MandarinInstrInfo::MandarinInstrInfo(MandarinSubtarget &ST)
	: MandarinGenInstrInfo(MD::ADJCALLSTACKDOWN, MD::ADJCALLSTACKUP),
    RI(ST), Subtarget(ST) {
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned MandarinInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                             int &FrameIndex) const
{
	MI->dump();

  /*if (MI->getOpcode() == MD::LOADri ||
      MI->getOpcode() == SP::LDXri ||
      MI->getOpcode() == SP::LDFri ||
      MI->getOpcode() == SP::LDDFri ||
      MI->getOpcode() == SP::LDQFri) {
    if (MI->getOperand(1).isFI() && MI->getOperand(2).isImm() &&
        MI->getOperand(2).getImm() == 0) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }*/

  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned MandarinInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
                                            int &FrameIndex) const
{
	MI->dump();

  /*if (MI->getOpcode() == SP::STri ||
      MI->getOpcode() == SP::STXri ||
      MI->getOpcode() == SP::STFri ||
      MI->getOpcode() == SP::STDFri ||
      MI->getOpcode() == SP::STQFri) {
    if (MI->getOperand(0).isFI() && MI->getOperand(1).isImm() &&
        MI->getOperand(1).getImm() == 0) {
      FrameIndex = MI->getOperand(0).getIndex();
      return MI->getOperand(2).getReg();
    }
  }*/

  return 0;
}

bool MandarinInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const
{
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugValue())
      continue;

    // When we see a non-terminator, we are done.
    if (!isUnpredicatedTerminator(I))
      break;

    // Terminator is not a branch.
    if (!I->isBranch())
      return true;

	// Cannot handle indirect branches.
	if (I->getOpcode() == MD::JMPr || I->getOpcode() == MD::JCCr)
      return true;

	// Handle unconditional branches.
    if (I->getOpcode() == MD::JMPi) {
      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a JMP, delete them.
      while (llvm::next(I) != MBB.end())
        llvm::next(I)->eraseFromParent();
      Cond.clear();
      FBB = 0;

      // Delete the JMP if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = 0;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

	MDCC::CondCodes BranchCode = static_cast<MDCC::CondCodes>(I->getOperand(1).getImm());
	if (BranchCode == MDCC::COND_INVALID)
      return true;  // Can't handle weird stuff.

	// Working from the bottom, handle the first conditional branch.
    if (Cond.empty()) {
      FBB = TBB;
      TBB = I->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(BranchCode));
      continue;
    }

	// Handle subsequent conditional branches. Only handle the case where all
    // conditional branches branch to the same destination.
    assert(Cond.size() == 1);
    assert(TBB);

    // Only handle the case where all conditional branches branch to
    // the same destination.
    if (TBB != I->getOperand(0).getMBB())
      return true;

	MDCC::CondCodes OldBranchCode = (MDCC::CondCodes)Cond[0].getImm();

    // If the conditions are the same, we can leave them alone.
    if (OldBranchCode == BranchCode)
      continue;

    return true;
  }
  return false;
}

unsigned
MandarinInstrInfo::InsertBranch(MachineBasicBlock &MBB,MachineBasicBlock *TBB,
                             MachineBasicBlock *FBB,
                             const SmallVectorImpl<MachineOperand> &Cond,
                             DebugLoc DL) const {
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 1 || Cond.size() == 0) &&
         "Mandarin branch conditions should have one component!");

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch with multiple successors!");
	BuildMI(&MBB, DL, get(MD::JMPi)).addMBB(TBB);
    return 1;
  }

  // Conditional branch
  unsigned Count = 0;
  BuildMI(&MBB, DL, get(MD::JCCi)).addMBB(TBB).addImm(Cond[0].getImm());
  ++Count;

  if (FBB)
  {
	// Two-way Conditional branch. Insert the second branch.
	BuildMI(&MBB, DL, get(MD::JMPi)).addMBB(FBB);
    ++Count;
  }

  return Count;
}

unsigned MandarinInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;

    if (I->isDebugValue())
      continue;

	if (I->getOpcode() != MD::JMPi &&
        I->getOpcode() != MD::JCCi &&
        I->getOpcode() != MD::JMPr &&
		I->getOpcode() != MD::JCCr)
      break; // Not a branch

	// Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

void MandarinInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I, DebugLoc DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const
{
	if(MD::GenericRegsRegClass.contains(DestReg, SrcReg))
	{
		BuildMI(MBB, I, DL, get(MD::MOVrr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
	}
	else if(MD::DoubleRegsRegClass.contains(DestReg, SrcReg) )
	{
		BuildMI(MBB, I, DL, get(MD::MOV2rr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
	}
	else if(MD::DoubleRegsRegClass.contains(DestReg) && (SrcReg == MD::R0 || SrcReg == MD::R2))
	{
		printf("Registers are already there\n");
	}
	else if(MD::QuadRegsRegClass.contains(DestReg, SrcReg))
	{
		BuildMI(MBB, I, DL, get(MD::MOV4rr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
	}
	else if(MD::QuadRegsRegClass.contains(DestReg) && SrcReg == MD::R0)
	{
		printf("Registers are already there\n");
	}
	else
	{
		llvm_unreachable("Impossible reg-to-reg copy");
	}
}

/// storeRegToStackSlot - Store the specified register of the given register
/// class to the specified stack frame index. The store instruction is to be
/// added to the given machine basic block before the specified machine
/// instruction. If isKill is true, the register operand is the last use and
/// must be marked kill.
void MandarinInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                    unsigned SrcReg, bool isKill, int FrameIndex,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
	DebugLoc DL;
	if (MI != MBB.end())
		DL = MI->getDebugLoc();

	MachineFunction *MF = MBB.getParent();
	const MachineFrameInfo &MFI = *MF->getFrameInfo();
	MachineMemOperand *MMO =
		MF->getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIndex),
								MachineMemOperand::MOStore,
								MFI.getObjectSize(FrameIndex),
								MFI.getObjectAlignment(FrameIndex));

	if (RC == &MD::GenericRegsRegClass)
	{
		BuildMI(MBB, MI, DL, get(MD::STORErr))
		  .addFrameIndex(FrameIndex).addImm(0)
		  .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
	}else{
		llvm_unreachable("Cannot store this register to stack slot!");
	}
}

/// loadRegFromStackSlot - Load the specified register of the given register
/// class from the specified stack frame index. The load instruction is to be
/// added to the given machine basic block before the specified machine
/// instruction.
void MandarinInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                     unsigned DestReg, int FrameIndex,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
	DebugLoc DL;
	if (MI != MBB.end())
		DL = MI->getDebugLoc();

	MachineFunction &MF = *MBB.getParent();
	MachineFrameInfo &MFI = *MF.getFrameInfo();

	MachineMemOperand *MMO =
		MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIndex),
								MachineMemOperand::MOLoad,
								MFI.getObjectSize(FrameIndex),
								MFI.getObjectAlignment(FrameIndex));

	if (RC == &MD::GenericRegsRegClass)
	{
		BuildMI(MBB, MI, DL, get(MD::LOADrr), DestReg)
		  .addFrameIndex(FrameIndex).addImm(0).addMemOperand(MMO);
	}else{
		llvm_unreachable("Cannot store this register to stack slot!");
	}
}
