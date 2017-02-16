//===-- MandarinAsmPrinter.cpp - Mandarin LLVM assembly writer ------------------===//
//
//                     Vyacheslav Egorov
//
// This file is distributed under the MIT License
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format Mandarin assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "Mandarin.h"
#include "MandarinInstrInfo.h"
#include "MandarinTargetMachine.h"
#include "MCTargetDesc/MandarinBaseInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/Mangler.h"
using namespace llvm;


namespace llvm
{
	// Just fuck off, llvm
	class HackMCAsmStreamer : public MCStreamer {
	public:
	  formatted_raw_ostream &OS;
	  const MCAsmInfo *MAI;
	};

	formatted_raw_ostream& HackStreamer(MCStreamer &OutStreamer)
	{
		HackMCAsmStreamer &hack = *(HackMCAsmStreamer*)&OutStreamer;

		return hack.OS;
	}
}

namespace {
  class MandarinAsmPrinter : public AsmPrinter {
  public:
    explicit MandarinAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

    virtual const char *getPassName() const {
      return "Mandarin Assembly Printer";
    }

    void printOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);
    void printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &OS,
                         const char *Modifier = 0);
	void printBasicBlock(const MachineInstr *MI, int opNum, raw_ostream &OS);
    void printCCOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);

	virtual void EmitConstantPool();
    virtual void EmitInstruction(const MachineInstr *MI) {
      SmallString<128> Str;
      raw_svector_ostream OS(Str);
      printInstruction(MI, OS);
      OutStreamer.EmitRawText(OS.str());
    }
    void printInstruction(const MachineInstr *MI, raw_ostream &OS);// autogen'd.
    static const char *getRegisterName(unsigned RegNo);

    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         unsigned AsmVariant, const char *ExtraCode,
                         raw_ostream &O);
    bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                               unsigned AsmVariant, const char *ExtraCode,
                               raw_ostream &O);

    virtual bool isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB)
                       const;

  };
} // end of anonymous namespace

#include "MandarinGenAsmWriter.inc"

void MandarinAsmPrinter::EmitConstantPool()
{
	const MachineConstantPool *MCP = MF->getConstantPool();
	const std::vector<MachineConstantPoolEntry> &CP = MCP->getConstants();
	if (CP.empty()) return;

	OutStreamer.EmitRawText(".DATA");

	unsigned Offset = 0;
    for (unsigned i = 0, e = CP.size(); i != e; ++i)
	{
		const MachineConstantPoolEntry &CPE = CP[i];
		unsigned Align = CPE.getAlignment();

		// Emit inter-object padding for alignment.
		unsigned AlignMask = CPE.getAlignment() - 1;
		unsigned NewOffset = (Offset + AlignMask) & ~AlignMask;

		SmallString<128> Str;
		raw_svector_ostream OS(Str);

		if(NewOffset - Offset)
			OS << "\tstring\tbyte[" << (NewOffset - Offset) << "] 0\n";

		Type *Ty = CPE.getType();
		Offset = NewOffset + TM.getDataLayout()->getTypeAllocSize(Ty);

		MCSymbol *Symbol = GetCPISymbol(i);

		OS << "\t" << *Symbol;

		HackStreamer(OutStreamer) << OS.str();

		if (CPE.isMachineConstantPoolEntry())
			EmitMachineConstantPoolValue(CPE.Val.MachineCPVal);
		else
			EmitGlobalConstant(CPE.Val.ConstVal);
    }
}

void MandarinAsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                   raw_ostream &O)
{
	const MachineOperand &MO = MI->getOperand (opNum);
	unsigned TF = MO.getTargetFlags();

	bool CloseParen = true;
	switch (TF) {
	default:
		llvm_unreachable("Unknown target flags on operand");
	case MDII::MO_NO_FLAG:
		CloseParen = false;
		break;
	case MDII::MO_HI32:
		O << "hi32(";
		break;
	case MDII::MO_HI24:
		O << "hi24(";
		break;
	case MDII::MO_HI16:
		O << "hi16(";
		break;
	case MDII::MO_HI8:
		O << "hi8(";
		break;
	case MDII::MO_LO32:
		O << "hi32(";
		break;
	case MDII::MO_LO24:
		O << "hi24(";
		break;
	case MDII::MO_LO16:
		O << "hi16(";
		break;
	case MDII::MO_LO8:
		O << "hi8(";
		break;
	}

	switch (MO.getType()) {
	case MachineOperand::MO_Register:
		O << StringRef(getRegisterName(MO.getReg())).lower();
		break;

	case MachineOperand::MO_Immediate:
		O << (int)MO.getImm();
		break;
	case MachineOperand::MO_MachineBasicBlock:
		O << *MO.getMBB()->getSymbol();
		return;
	case MachineOperand::MO_GlobalAddress:
		O << *getSymbol(MO.getGlobal());
		break;
	case MachineOperand::MO_BlockAddress:
		O <<  GetBlockAddressSymbol(MO.getBlockAddress())->getName();
		break;
	case MachineOperand::MO_ExternalSymbol:
		O << MO.getSymbolName();
		break;
	case MachineOperand::MO_ConstantPoolIndex:
		O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << "_"
			<< MO.getIndex();
		break;
	default:
		llvm_unreachable("<unknown operand type>");
	}

	if (CloseParen)
		O << ")";
}

void MandarinAsmPrinter::printMemOperand(const MachineInstr *MI, int opNum,
                                      raw_ostream &O, const char *Modifier) {
  printOperand(MI, opNum, O);
}

void MandarinAsmPrinter::printBasicBlock(const MachineInstr *MI, int opNum, raw_ostream &O)
{
  printOperand(MI, opNum, O);
}

void MandarinAsmPrinter::printCCOperand(const MachineInstr *MI, int opNum,
                                     raw_ostream &O)
{
	MDCC::CondCodes CC = (MDCC::CondCodes)MI->getOperand(opNum).getImm();
	switch(CC)
	{
	default:
		O << "cc_error";
		break;
	case MDCC::COND_EQ:
		O << "eq";
		break;
	case MDCC::COND_NE:
		O << "ne";
		break;
	case MDCC::COND_GR:
		O << "gr";
		break;
	case MDCC::COND_LS:
		O << "ls";
		break;
	case MDCC::COND_GE:
		O << "ge";
		break;
	case MDCC::COND_LE:
		O << "le";
		break;
	}
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool MandarinAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                      unsigned AsmVariant,
                                      const char *ExtraCode,
                                      raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1] != 0) return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default:
      // See if this is a generic print operand
      return AsmPrinter::PrintAsmOperand(MI, OpNo, AsmVariant, ExtraCode, O);
    case 'r':
     break;
    }
  }

  printOperand(MI, OpNo, O);

  return false;
}

bool MandarinAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                            unsigned OpNo, unsigned AsmVariant,
                                            const char *ExtraCode,
                                            raw_ostream &O) {
  if (ExtraCode && ExtraCode[0])
    return true;  // Unknown modifier

  O << '[';
  printMemOperand(MI, OpNo, O);
  O << ']';

  return false;
}

/// isBlockOnlyReachableByFallthough - Return true if the basic block has
/// exactly one predecessor and the control transfer mechanism between
/// the predecessor and this block is a fall-through.
///
/// This overrides AsmPrinter's implementation to handle delay slots.
bool MandarinAsmPrinter::
isBlockOnlyReachableByFallthrough(const MachineBasicBlock *MBB) const {
  // If this is a landing pad, it isn't a fall through.  If it has no preds,
  // then nothing falls through to it.
  if (MBB->isLandingPad() || MBB->pred_empty())
    return false;

  // If there isn't exactly one predecessor, it can't be a fall through.
  MachineBasicBlock::const_pred_iterator PI = MBB->pred_begin(), PI2 = PI;
  ++PI2;
  if (PI2 != MBB->pred_end())
    return false;

  // The predecessor has to be immediately before this block.
  const MachineBasicBlock *Pred = *PI;

  if (!Pred->isLayoutSuccessor(MBB))
    return false;

  // Check if the last terminator is an unconditional branch.
  MachineBasicBlock::const_iterator I = Pred->end();
  while (I != Pred->begin() && !(--I)->isTerminator())
    ; // Noop
  return I == Pred->end() || !I->isBarrier();
}

// Force static initialization.
extern "C" void LLVMInitializeMandarinAsmPrinter() {
  RegisterAsmPrinter<MandarinAsmPrinter> X(TheMandarinTarget);
}
