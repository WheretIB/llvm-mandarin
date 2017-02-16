//===- MandarinMachineFunctionInfo.h - Mandarin Machine Function Info -*- C++ -*-===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file declares  Mandarin specific per-machine-function information.
//
//===----------------------------------------------------------------------===//
#ifndef MANDARINMACHINEFUNCTIONINFO_H
#define MANDARINMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

  class MandarinMachineFunctionInfo : public MachineFunctionInfo {
    virtual void anchor();
  private:
    unsigned GlobalBaseReg;

    /// VarArgsFrameOffset - Frame offset to start of varargs area.
    int VarArgsFrameOffset;

    /// SRetReturnReg - Holds the virtual register into which the sret
    /// argument is passed.
    unsigned SRetReturnReg;

    /// IsLeafProc - True if the function is a leaf procedure.
    bool IsLeafProc;
  public:
    MandarinMachineFunctionInfo()
      : GlobalBaseReg(0), VarArgsFrameOffset(0), SRetReturnReg(0) {}
    explicit MandarinMachineFunctionInfo(MachineFunction &MF)
      : GlobalBaseReg(0), VarArgsFrameOffset(0), SRetReturnReg(0) {}

    int getVarArgsFrameOffset() const { return VarArgsFrameOffset; }
    void setVarArgsFrameOffset(int Offset) { VarArgsFrameOffset = Offset; }

    unsigned getSRetReturnReg() const { return SRetReturnReg; }
    void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }
  };
}

#endif
