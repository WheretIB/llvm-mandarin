//===-- MandarinCallingConv.td - Mandarin Instruction Formats ------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//

def RetCC_Mandarin : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[f32], CCAssignToReg<[R0, R1, R2, R3]>>,
  CCIfType<[v2i32], CCAssignToReg<[RD0, RD1]>>,
  CCIfType<[v2f32], CCAssignToReg<[RD0, RD1]>>,
  CCIfType<[v4i32], CCAssignToReg<[RQ0]>>,
  CCIfType<[v4f32], CCAssignToReg<[RQ0]>>
]>;

def CC_Mandarin_AssignStack : CallingConv<[
  // Pass by value if the byval attribute is given
  CCIfByVal<CCPassByVal<4, 4>>,

  CCIfType<[i32], CCAssignToStack<4, 4>>,
  CCIfType<[f32], CCAssignToStack<4, 4>>,
  CCIfType<[v2i32], CCAssignToStack<8, 4>>,
  CCIfType<[v2f32], CCAssignToStack<8, 4>>,
  CCIfType<[v4i32], CCAssignToStack<16, 4>>,
  CCIfType<[v4f32], CCAssignToStack<16, 4>>
]>;
