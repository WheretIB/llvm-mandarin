//===-- MandarinSelectionDAGInfo.cpp - Mandarin SelectionDAG Info ---------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file implements the MandarinSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mandarin-selectiondag-info"
#include "MandarinTargetMachine.h"
using namespace llvm;

MandarinSelectionDAGInfo::MandarinSelectionDAGInfo(const MandarinTargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

MandarinSelectionDAGInfo::~MandarinSelectionDAGInfo() {
}
