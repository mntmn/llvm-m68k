//===-- M68kMCInstLower.h - Convert M68k MI to MCInst -----------*- C++ -*-===//
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

#ifndef M68KMCINSTLOWER_H
#define M68KMCINSTLOWER_H

#include "llvm/Support/Compiler.h"

namespace llvm {

  class AsmPrinter;
  class MCContext;
  class MachineInstr;
  class MCInst;

  class LLVM_LIBRARY_VISIBILITY M68kMCInstLower {
    MCContext &Ctx;
    AsmPrinter &Printer;
  public:
    M68kMCInstLower(MCContext &ctx, AsmPrinter &printer)
      : Ctx(ctx), Printer(printer) {}
  
    void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  };

} // end namespace llvm

#endif
