//===-- Mandarin.td - Describe the Mandarin Target Machine ----------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File
//===----------------------------------------------------------------------===//

include "MandarinRegisterInfo.td"
include "MandarinCallingConv.td"
include "MandarinInstrInfo.td"

def MandarinInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// Mandarin processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic",         []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def Mandarin : Target {
  // Pull in Instruction Info:
  let InstructionSet = MandarinInstrInfo;
}
