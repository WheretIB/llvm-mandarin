//===-- Mandarin.h - Top-level interface for Mandarin representation --*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Mandarin back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_MANDARIN_H
#define TARGET_MANDARIN_H

#include "MCTargetDesc/MandarinMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace MDCC {
  // Mandarin specific condition code.
  enum CondCodes {
    COND_EQ  = 0,
    COND_NE = 1,
    COND_GR = 2,
    COND_LS = 3,
    COND_GE = 4,
    COND_LE = 5,

    COND_INVALID = -1
  };
}

namespace llvm {
  class FunctionPass;
  class MandarinTargetMachine;
  class formatted_raw_ostream;

  FunctionPass *createMandarinISelDag(MandarinTargetMachine &TM);

} // end namespace llvm;

#endif
