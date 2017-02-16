//===-- MandarinTargetInfo.cpp - Mandarin Target Implementation -----------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//

#include "Mandarin.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm
{
	Target TheMandarinTarget;
}

extern "C" void LLVMInitializeMandarinTargetInfo() {
  RegisterTarget<Triple::UnknownArch, /*HasJIT=*/ false>
    X(TheMandarinTarget, "mandarin", "Mandarin");
}
