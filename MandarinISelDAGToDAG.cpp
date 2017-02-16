//===-- MandarinISelDAGToDAG.cpp - A dag to dag inst selector for Mandarin ------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the Mandarin target.
//
//===----------------------------------------------------------------------===//

#include "MandarinTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===--------------------------------------------------------------------===//
/// MandarinDAGToDAGISel - Mandarin specific code to select Mandarin machine
/// instructions for SelectionDAG operations.
///
namespace {
class MandarinDAGToDAGISel : public SelectionDAGISel {
  /// Subtarget - Keep a pointer to the Mandarin Subtarget around so that we can
  /// make the right decision when generating code for different targets.
  const MandarinSubtarget &Subtarget;
  MandarinTargetMachine &TM;
public:
  explicit MandarinDAGToDAGISel(MandarinTargetMachine &tm)
    : SelectionDAGISel(tm),
      Subtarget(tm.getSubtarget<MandarinSubtarget>()),
      TM(tm) {
  }

  SDNode *Select(SDNode *N);

  // Complex Pattern Selectors.
  bool SelectAddr(SDValue N, SDValue &R1);

  virtual const char *getPassName() const {
    return "Mandarin DAG->DAG Pattern Instruction Selection";
  }

  // Include the pieces autogenerated from the target description.
#include "MandarinGenDAGISel.inc"

private:
};
}  // end anonymous namespace

SDNode *MandarinDAGToDAGISel::Select(SDNode *N) {
  SDLoc dl(N);
  if (N->isMachineOpcode()) {
    N->setNodeId(-1);
    return NULL;   // Already selected.
  }

  return SelectCode(N);
}

bool MandarinDAGToDAGISel::SelectAddr(SDValue Addr, SDValue &R1)
{
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    R1 = CurDAG->getTargetFrameIndex(FIN->getIndex(),
                                       getTargetLowering()->getPointerTy());
    return true;
  }
  if (Addr.getOpcode() == ISD::TargetExternalSymbol ||
      Addr.getOpcode() == ISD::TargetGlobalAddress ||
      Addr.getOpcode() == ISD::TargetGlobalTLSAddress)
    return false;  // direct calls.

  R1 = Addr;
  return true;
}

/// createMandarinISelDag - This pass converts a legalized DAG into a
/// Mandarin-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createMandarinISelDag(MandarinTargetMachine &TM) {
  return new MandarinDAGToDAGISel(TM);
}