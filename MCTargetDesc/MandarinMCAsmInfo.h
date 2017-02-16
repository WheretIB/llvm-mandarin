//===-- MandarinMCAsmInfo.h - Mandarin asm properties ---------------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MandarinMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MANDARINTARGETASMINFO_H
#define MANDARINTARGETASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class StringRef;

  class MandarinELFMCAsmInfo : public MCAsmInfoELF {
    virtual void anchor();
  public:
    explicit MandarinELFMCAsmInfo(StringRef TT);
  };

} // namespace llvm

#endif
