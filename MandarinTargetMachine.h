//===-- MandarinTargetMachine.h - Define TargetMachine for Mandarin -------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mandarin specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef MANDARINTARGETMACHINE_H
#define MANDARINTARGETMACHINE_H

#include "MandarinFrameLowering.h"
#include "MandarinISelLowering.h"
#include "MandarinInstrInfo.h"
#include "MandarinSelectionDAGInfo.h"
#include "MandarinSubtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class MandarinTargetLowering;

class MandarinTargetMachine : public LLVMTargetMachine {
  MandarinSubtarget Subtarget;
  const DataLayout DL;       // Calculates type size & alignment
  MandarinInstrInfo InstrInfo;
  MandarinTargetLowering TLInfo;
  MandarinSelectionDAGInfo TSInfo;
  MandarinFrameLowering FrameLowering;
public:
  MandarinTargetMachine(const Target &T, StringRef TT,
                     StringRef CPU, StringRef FS, const TargetOptions &Options,
                     Reloc::Model RM, CodeModel::Model CM,
                     CodeGenOpt::Level OL);

  virtual const MandarinInstrInfo *getInstrInfo() const {
    return &InstrInfo;
  }
  virtual const MandarinFrameLowering  *getFrameLowering() const {
    return &FrameLowering;
  }
  virtual const MandarinSubtarget   *getSubtargetImpl() const{
    return &Subtarget;
  }
  virtual const MandarinRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }
  virtual const MandarinTargetLowering* getTargetLowering() const {
    return &TLInfo;
  }
  virtual const MandarinSelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }
  virtual const DataLayout       *getDataLayout() const { return &DL; }

  // Pass Pipeline Configuration
  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
  virtual bool addCodeEmitter(PassManagerBase &PM, JITCodeEmitter &JCE);
};

} // end namespace llvm

#endif
