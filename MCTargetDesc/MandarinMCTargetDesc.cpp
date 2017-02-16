//===-- MandarinMCTargetDesc.cpp - Mandarin Target Descriptions -----------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file provides Mandarin specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "MandarinMCTargetDesc.h"
#include "MandarinMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "MandarinGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MandarinGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MandarinGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createMandarinMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMandarinMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMandarinMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMandarinMCRegisterInfo(X, MD::R0);
  return X;
}

static MCSubtargetInfo *createMandarinMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                   StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMandarinMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.

static MCCodeGenInfo *createMandarinMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                               CodeModel::Model CM,
                                               CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();

  // The default 32-bit code model is abs32/pic32.
  if (CM == CodeModel::Default)
    CM = RM == Reloc::PIC_ ? CodeModel::Medium : CodeModel::Small;

  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

extern "C" void LLVMInitializeMandarinTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<MandarinELFMCAsmInfo> X(TheMandarinTarget);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheMandarinTarget,
                                       createMandarinMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheMandarinTarget, createMandarinMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheMandarinTarget, createMandarinMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheMandarinTarget,
                                          createMandarinMCSubtargetInfo);
}
