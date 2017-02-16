//===-- MandarinFrameLowering.h - Define frame lowering for Mandarin --*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef MANDARIN_FRAMEINFO_H
#define MANDARIN_FRAMEINFO_H

#include "Mandarin.h"
#include "MandarinSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class MandarinSubtarget;

class MandarinFrameLowering : public TargetFrameLowering {
  const MandarinSubtarget &SubTarget;
public:
  explicit MandarinFrameLowering(const MandarinSubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsUp, 4, 0),
      SubTarget(ST) {}

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  bool hasReservedCallFrame(const MachineFunction &MF) const;
  bool hasFP(const MachineFunction &MF) const;

private:
};

} // End llvm namespace

#endif
