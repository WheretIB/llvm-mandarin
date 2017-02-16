//===-- MandarinMCTargetDesc.h - Mandarin Target Descriptions -------------===//
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

#ifndef MANDARINMCTARGETDESC_H
#define MANDARINMCTARGETDESC_H

namespace llvm {
class Target;

extern Target TheMandarinTarget;

} // End llvm namespace

// Defines symbolic names for Mandarin registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "MandarinGenRegisterInfo.inc"

// Defines symbolic names for the Mandarin instructions.
//
#define GET_INSTRINFO_ENUM
#include "MandarinGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "MandarinGenSubtargetInfo.inc"

#endif
