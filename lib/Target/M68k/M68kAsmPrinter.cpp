//===-- M68kAsmPrinter.cpp - M68k LLVM Assembly Printer ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "m68k-asm-printer"
#include "M68kAsmPrinter.h"
#include "M68k.h"
#include "M68kMCInstLower.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"

using namespace llvm;

M68kAsmPrinter::M68kAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
  : AsmPrinter(TM, Streamer) {
}

void M68kAsmPrinter::EmitFunctionEntryLabel() {
    OutStreamer.EmitRawText(CurrentFnSym->getName() + "\tPROC");
}

void M68kAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  M68kMCInstLower Lower(OutContext, *this);

  MCInst Inst;
  Lower.Lower(MI, Inst);
  OutStreamer.EmitInstruction(Inst);
}

void M68kAsmPrinter::EmitEndOfAsmFile(Module &) {
  OutStreamer.EmitRawText(StringRef("\tEND"));
}

extern "C" void LLVMInitializeM68kAsmPrinter() {
  RegisterAsmPrinter<M68kAsmPrinter> X(TheM68kTarget);
}
