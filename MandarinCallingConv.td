//===-- MandarinCallingConv.td - Mandarin Instruction Formats ------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//

def RetCC_Mandarin : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[f32], CCAssignToReg<[R0, R1, R2, R3]>>
]>;

def CC_Mandarin_AssignStack : CallingConv<[
  // Pass by value if the byval attribute is given
  CCIfByVal<CCPassByVal<4, 4>>,

  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[f32], CCAssignToStack<4, 4>>
]>;
