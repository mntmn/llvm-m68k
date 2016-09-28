//===-- M68kISelLowering.cpp - M68k DAG Lowering ----------------*- C++ -*-===//
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

#include "M68kISelLowering.h"
#include "M68k.h"
#include "M68kTargetMachine.h"
#include "M68kRegisterInfo.h"
#include "M68kMacTargetObjectFile.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
using namespace llvm;

#include <stdio.h>

M68kTargetLowering::M68kTargetLowering(M68kTargetMachine &TM)
  : TargetLowering(TM, new M68kMacTargetLoweringObjectFile()) {
  // Setup register classes
  addRegisterClass(MVT::i8, &M68k::DR8RegClass);
  addRegisterClass(MVT::i16, &M68k::DR16RegClass);
  addRegisterClass(MVT::i32, &M68k::DR32RegClass);
  computeRegisterProperties();

  // We have no extending loads.
  for (unsigned ExtType = (unsigned)ISD::EXTLOAD;
       ExtType < (unsigned)ISD::LAST_LOADEXT_TYPE; ++ExtType) {
    // TODO(kwaters): What about i1 and float?
    setLoadExtAction(ExtType, MVT::i8, Expand);
    setLoadExtAction(ExtType, MVT::i16, Expand);
    setLoadExtAction(ExtType, MVT::i32, Expand);
  }

  // Easiest to match truncation stores with the truncating patterns.
  setTruncStoreAction(MVT::i16, MVT::i8, Expand);
  setTruncStoreAction(MVT::i32, MVT::i8, Expand);
  setTruncStoreAction(MVT::i32, MVT::i16, Expand);
}


//===----------------------------------------------------------------------===//
//  Calling Convention
//===----------------------------------------------------------------------===//

#include "M68kGenCallingConv.inc"

SDValue M68kTargetLowering::
LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                     bool isVarArg,
                     const SmallVectorImpl<ISD::InputArg> &Ins, DebugLoc dl,
                     SelectionDAG &DAG,
                     SmallVectorImpl<SDValue> &InVals) const {
  // Check for the right calling convention.
  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  // TODO
  assert(!isVarArg && "VarArg unsupported");

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_M68k);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    if (VA.isRegLoc()) {
      const TargetRegisterClass *RC;
      EVT ValVT = VA.getValVT();
      
      switch (ValVT.getSimpleVT().SimpleTy) {
        default:
          llvm_unreachable("ValVT not supported by formal arguments Lowering");
        case MVT::i32:
          RC = &M68k::DR32RegClass;
          break;
        case MVT::i16:
          RC = &M68k::DR16RegClass;
          break;
        case MVT::i8:
          RC = &M68k::DR8RegClass;
          break;
      }
      
      unsigned Reg = MF.addLiveIn(VA.getLocReg(), RC);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, ValVT);
      InVals.push_back(ArgValue);
    } else {
      int FI = MFI->CreateFixedObject(VA.getLocVT().getStoreSize(),
                                      VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, VA.getLocVT());
      InVals.push_back(DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(FI),
                                   false, false, false, 0));
    }
  }

  return Chain;
}

const char *M68kTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return 0;
  case M68kISD::JSR:          return "M68kISD::JSR";
  }
}

SDValue
M68kTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
          SmallVectorImpl<SDValue> &InVals) const {

  SelectionDAG &DAG                     = CLI.DAG;
  DebugLoc &dl                          = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;
  
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  
  CCInfo.AnalyzeCallOperands(Outs, CC_M68k);
  
  //Chain = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(0, true));
  
  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  
  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, j = 0, e = ArgLocs.size();
       i != e;
       ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;

    // Put argument in a physical register.
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      printf("ArgLocs walk error\n");
    }
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain
  // and flag operands which copy the outgoing args into the appropriate regs.
  SDValue InFlag;

  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }
  
  std::vector<SDValue> Ops;
  
  /*for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
    RegsToPass[i].second.getValueType()));*/
  
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    Callee = DAG.getTargetGlobalAddress(GV, dl, getPointerTy(), 0, 0);
  }
  
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  
  /*const TargetRegisterInfo *TRI = getTargetMachine().getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));*/
  
  if (InFlag.getNode())
    Ops.push_back(InFlag);
  
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  
  Chain = DAG.getNode(M68kISD::JSR, dl, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(0, true),
    DAG.getIntPtrConstant(0, true), InFlag);
  if (!Ins.empty())
  InFlag = Chain.getValue(1);
  
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg,
                         Ins, dl, DAG, InVals);
}

SDValue
M68kTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                   CallingConv::ID CallConv, bool isVarArg,
                                   const SmallVectorImpl<ISD::InputArg> &Ins,
                                   DebugLoc dl, SelectionDAG &DAG,
                                   SmallVectorImpl<SDValue> &InVals) const {

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCRetInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                    getTargetMachine(), RVLocs, *DAG.getContext());

  CCRetInfo.AnalyzeCallResult(Ins, RetCC_M68k);
  //printf("LowerCallResult: rvlocs %d invals %d\n",RVLocs.size(),InVals.size());

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue Val = DAG.getCopyFromReg(Chain, dl,
                                     VA.getLocReg(), VA.getLocVT(), InFlag);
    Chain = Val.getValue(1);
    InFlag = Val.getValue(2);

    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::AExt:
      Val = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), Val);
      break;
    case CCValAssign::ZExt:
      Val = DAG.getNode(ISD::AssertZext, dl, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), Val);
      break;
    case CCValAssign::SExt:
      Val = DAG.getNode(ISD::AssertSext, dl, VA.getLocVT(), Val,
                        DAG.getValueType(VA.getValVT()));
      Val = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), Val);
      break;
    }

    printf("inval\n");

    InVals.push_back(Val);
  }

  return Chain;
}

SDValue
M68kTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                DebugLoc dl, SelectionDAG &DAG) const {
  // Check for the right calling convention.
  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  // TODO
  assert(!isVarArg && "VarArg unsupported");

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_M68k);

  // Mark out registers as live outs.
  // TODO simplify
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }

  SDValue Flag;

  // TODO struct returns through stack voodoo.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers");
    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    // TODO: what does this mean?  We only know how to return one value does
    // this matter?
    Flag = Chain.getValue(1);
  }

  if (Flag.getNode())
    return DAG.getNode(M68kISD::RET_FLAG, dl, MVT::Other, Chain, Flag);
  return DAG.getNode(M68kISD::RET_FLAG, dl, MVT::Other, Chain);
}
