//===-- M68kMCInstLower.cpp - Convert M68k MI to MCInst ---------*- C++ -*-===//
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

#include "M68kMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/Mangler.h"
using namespace llvm;

void M68kMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (MachineInstr::const_mop_iterator Op = MI->operands_begin(),
       E = MI->operands_end(); Op != E; ++Op) {
    switch (Op->getType()) {
    default:
      MI->dump();
      printf("optype: %d\n",Op->getType());
      llvm_unreachable("unknown operand type");
      break;
    case MachineOperand::MO_Register:
      if (Op->isImplicit()) continue;
      OutMI.addOperand(MCOperand::CreateReg(Op->getReg()));
      break;
    case MachineOperand::MO_Immediate:
      OutMI.addOperand(MCOperand::CreateImm(Op->getImm()));
      break;
    case MachineOperand::MO_GlobalAddress:
      OutMI.addOperand(MCOperand::CreateExpr(MCSymbolRefExpr::Create(Printer.Mang->getSymbol(Op->getGlobal()), Ctx)));
      break;
    }
  }
}
