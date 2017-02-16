//===-- MandarinSelectionDAGInfo.h - Mandarin SelectionDAG Info -------*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file defines the Mandarin subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MANDARINSELECTIONDAGINFO_H
#define MANDARINSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class MandarinTargetMachine;

class MandarinSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit MandarinSelectionDAGInfo(const MandarinTargetMachine &TM);
  ~MandarinSelectionDAGInfo();
};

}

#endif
