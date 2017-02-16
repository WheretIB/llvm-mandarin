//===-- MandarinMCAsmInfo.cpp - Mandarin asm properties -------------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the MandarinMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MandarinMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void MandarinELFMCAsmInfo::anchor() { }

MandarinELFMCAsmInfo::MandarinELFMCAsmInfo(StringRef TT) {
  IsLittleEndian = false;
  Triple TheTriple(TT);
 
  StackGrowsUp = true;
  SeparatorString = "\r\n\t";
  CommentString = "// ";
  InlineAsmStart = "// inline start\n";
  InlineAsmEnd = "// inline end\n";

  PrivateGlobalPrefix = "pg_";

  AlignDirective = "\t// align\t";
  GlobalDirective = "\t// global\t";
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;

  Data8bitsDirective = "\tbyte\t";
  Data16bitsDirective = "\tword\t";
  Data32bitsDirective = "\tdword\t";

  HasIdentDirective = false;
}
