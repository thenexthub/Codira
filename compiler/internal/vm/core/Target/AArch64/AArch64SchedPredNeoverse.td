//===- AArch64SchedPredNeoverse.td - AArch64 Sched Preds -----*- tablegen -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file defines scheduling predicate definitions that are used by the
// AArch64 Neoverse processors.
//
//===----------------------------------------------------------------------===//

// Auxiliary predicates.

// Check for LSL shift == 0
def NeoverseNoLSL : MCSchedPredicate<
                      CheckAll<[CheckShiftLSL,
                                CheckShiftBy0]>>;

// Identify LDR/STR H/Q-form scaled (and potentially extended) FP instructions
def NeoverseHQForm : MCSchedPredicate<
                       CheckAll<[
                         CheckAny<[CheckHForm, CheckQForm]>,
                         CheckImmOperand<4, 1>]>>;

// Check if <Pd> == <Pg>
def NeoversePdIsPgFn : TIIPredicate<
                         "isNeoversePdSameAsPg",
                         MCOpcodeSwitchStatement<
                           [MCOpcodeSwitchCase<[BRKA_PPmP, BRKB_PPmP],
                             MCReturnStatement<CheckSameRegOperand<1, 2>>>],
                           MCReturnStatement<CheckSameRegOperand<0, 1>>>>;
def NeoversePdIsPg : MCSchedPredicate<NeoversePdIsPgFn>;

// Check if SVE INC/DEC (scalar), ALL, {1, 2, 4}
def NeoverseCheapIncDec : MCSchedPredicate<
                            CheckAll<[CheckOpcode<[
                                        INCB_XPiI, INCH_XPiI,
                                        INCW_XPiI, INCD_XPiI,
                                        DECB_XPiI, DECH_XPiI,
                                        DECW_XPiI, DECD_XPiI]>,
                                      CheckImmOperand<2, 31>,
                                      CheckAny<[
                                        CheckImmOperand<3, 1>,
                                        CheckImmOperand<3, 2>,
                                        CheckImmOperand<3, 4>]>]>>;

// Identify "[SU]?(MADD|MSUB)L?" as the alias for "[SU]?(MUL|MNEG)L?".
def NeoverseMULIdiomPred : MCSchedPredicate< // <op> Rd, Rs, Rv, ZR
                             CheckAll<[CheckOpcode<
                                         [MADDWrrr, MADDXrrr,
                                          MSUBWrrr, MSUBXrrr,
                                          SMADDLrrr, UMADDLrrr,
                                          SMSUBLrrr, UMSUBLrrr]>,
                                       CheckIsReg3Zero]>>;

def NeoverseZeroMove : MCSchedPredicate<
                         CheckAny<[
                           // MOV Wd, #0
                           // MOV Xd, #0
                           CheckAll<[CheckOpcode<[MOVZWi, MOVZXi]>,
                                     CheckIsImmOperand<1>,
                                     CheckImmOperand<1, 0>,
                                     CheckImmOperand<2, 0>]>,
                           // MOV Wd, WZR
                           // MOV Xd, XZR
                           // MOV Wd, Wn
                           // MOV Xd, Xn
                           CheckAll<[CheckOpcode<[ORRWrs, ORRXrs]>,
                                     CheckAll<[CheckIsReg1Zero,
                                               CheckImmOperand<3, 0>]>]>,
                           // FMOV Hd, WZR
                           // FMOV Hd, XZR
                           // FMOV Sd, WZR
                           // FMOV Dd, XZR
                           CheckAll<[CheckOpcode<[FMOVWHr, FMOVXHr,
                                                  FMOVWSr, FMOVXDr]>,
                                     CheckIsReg1Zero]>,
                           // MOVI Dd, #0
                           // MOVI Vd.2D, #0
                           CheckAll<[CheckOpcode<[MOVID, MOVIv2d_ns]>,
                                     CheckImmOperand<1, 0>]>,
                           // MOV Zd, Zn
                           CheckAll<[CheckOpcode<[ORR_ZZZ]>,
                                     CheckSameRegOperand<1, 2>]>,
                           // MOV Vd, Vn
                           CheckAll<[CheckOpcode<[ORRv16i8, ORRv8i8]>,
                                     CheckSameRegOperand<1, 2>]>,
                         ]>>;

def NeoverseAllActivePredicate : MCSchedPredicate<
                                   CheckAny<[
                                     // PTRUE Pd, ALL
                                     // PTRUES Pd, ALL
                                     CheckAll<[
                                       CheckOpcode<[
                                         PTRUE_B, PTRUE_H, PTRUE_S, PTRUE_D,
                                         PTRUES_B, PTRUES_H, PTRUES_S, PTRUES_D]>,
                                       CheckIsImmOperand<1>,
                                       CheckImmOperand<1, 31>]>,
                                   ]>>;
