//===-- MandarinTargetMachine.cpp - Define TargetMachine for Mandarin -----===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "MandarinTargetMachine.h"
#include "Mandarin.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm {
class Target;

extern Target TheMandarinTarget;

} // End llvm namespace

extern "C" void LLVMInitializeMandarinTarget() {
  // Register the target.
  RegisterTargetMachine<MandarinTargetMachine> X(TheMandarinTarget);
}

/// MandarinTargetMachine ctor
///
MandarinTargetMachine::MandarinTargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    DL(Subtarget.getDataLayout()),
    InstrInfo(Subtarget),
    TLInfo(*this),
	TSInfo(*this),
    FrameLowering(Subtarget)
{
  initAsmInfo();
}

namespace {
/// Mandarin Code Generator Pass Configuration Options.
class MandarinPassConfig : public TargetPassConfig {
public:
  MandarinPassConfig(MandarinTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  MandarinTargetMachine &getMandarinTargetMachine() const {
    return getTM<MandarinTargetMachine>();
  }

  virtual bool addInstSelector();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *MandarinTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MandarinPassConfig(this, PM);
}

bool MandarinPassConfig::addInstSelector() {
  addPass(createMandarinISelDag(getMandarinTargetMachine()));
  return false;
}

bool MandarinTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                        JITCodeEmitter &JCE) {
  // Machine code emitter pass for Mandarin.
  //PM.add(createMandarinJITCodeEmitterPass(*this, JCE));
  return false;
}

/// addPreEmitPass - This pass may be implemented by targets that want to run
/// passes immediately before machine code is emitted.  This should return
/// true if -print-machineinstrs should print out the code after the passes.
bool MandarinPassConfig::addPreEmitPass(){
  return true;
}
