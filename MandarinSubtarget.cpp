//===-- MandarinSubtarget.cpp - Mandarin Subtarget Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Mandarin specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "MandarinSubtarget.h"
#include "Mandarin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MandarinGenSubtargetInfo.inc"

using namespace llvm;

void MandarinSubtarget::anchor() { }

MandarinSubtarget::MandarinSubtarget(const std::string &TT, const std::string &CPU,
                               const std::string &FS) :
  MandarinGenSubtargetInfo(TT, CPU, FS)
{

  // Parse features string.
  ParseSubtargetFeatures(CPU, FS);
}
