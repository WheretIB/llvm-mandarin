set(LLVM_TARGET_DEFINITIONS Mandarin.td)

tablegen(LLVM MandarinGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MandarinGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MandarinGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM MandarinGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MandarinGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM MandarinGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM MandarinGenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(MandarinCommonTableGen)

add_llvm_target(MandarinCodeGen

  MandarinAsmPrinter.cpp
  MandarinInstrInfo.cpp
  MandarinISelDAGToDAG.cpp
  MandarinISelLowering.cpp
  MandarinFrameLowering.cpp
  MandarinMachineFunctionInfo.cpp
  MandarinRegisterInfo.cpp
  MandarinSubtarget.cpp
  MandarinTargetMachine.cpp
  MandarinSelectionDAGInfo.cpp
  )

add_dependencies(LLVMMandarinCodeGen MandarinCommonTableGen intrinsics_gen)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
