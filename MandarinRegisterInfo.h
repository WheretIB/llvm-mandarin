//===-- MandarinRegisterInfo.h - Mandarin Register Information Impl -------===//
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

#ifndef MANDARINREGISTERINFO_H
#define MANDARINREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "MandarinGenRegisterInfo.inc"

namespace llvm {

class MandarinSubtarget;
class TargetInstrInfo;
class Type;

struct MandarinRegisterInfo : public MandarinGenRegisterInfo {
  MandarinSubtarget &Subtarget;

  MandarinRegisterInfo(MandarinSubtarget &st);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;

  const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF,
                                                unsigned Kind) const;

  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                       RegScavenger *RS = NULL) const;

  bool requiresRegisterScavenging(const MachineFunction &MF) const {
    return true;
  }

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const {
    return true;
  }

  // Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const;
};

} // end namespace llvm

#endif
