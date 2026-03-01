//===- AArch64SchedPredExynos.td - AArch64 Sched Preds -----*- tablegen -*-===//
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
// AArch64 Exynos processors.
//
//===----------------------------------------------------------------------===//

// Auxiliary predicates.

// Check the shift in arithmetic and logic instructions.
def ExynosCheckShift : CheckAny<[CheckShiftBy0,
                                 CheckAll<
                                   [CheckShiftLSL,
                                    CheckAny<
                                      [CheckShiftBy1,
                                       CheckShiftBy2,
                                       CheckShiftBy3]>]>]>;

// Exynos predicates.

// Identify BLR specifying the LR register as the indirect target register.
def ExynosBranchLinkLRPred : MCSchedPredicate<
                               CheckAll<[CheckOpcode<[BLR]>,
                                         CheckRegOperand<0, LR>]>>;

// Identify arithmetic instructions without or with limited extension or shift.
def ExynosArithFn   : TIIPredicate<
                        "isExynosArithFast",
                        MCOpcodeSwitchStatement<
                          [MCOpcodeSwitchCase<
                             IsArithExtOp.ValidOpcodes,
                             MCReturnStatement<
                               CheckAny<[CheckExtBy0,
                                         CheckAll<
                                           [CheckAny<
                                             [CheckExtUXTW,
                                              CheckExtUXTX]>,
                                            CheckAny<
                                              [CheckExtBy1,
                                               CheckExtBy2,
                                               CheckExtBy3]>]>]>>>,
                           MCOpcodeSwitchCase<
                             IsArithShiftOp.ValidOpcodes,
                             MCReturnStatement<ExynosCheckShift>>,
                           MCOpcodeSwitchCase<
                             IsArithUnshiftOp.ValidOpcodes,
                             MCReturnStatement<TruePred>>,
                           MCOpcodeSwitchCase<
                             IsArithImmOp.ValidOpcodes,
                             MCReturnStatement<TruePred>>],
                          MCReturnStatement<FalsePred>>>;
def ExynosArithPred : MCSchedPredicate<ExynosArithFn>;

// Identify logic instructions with limited shift.
def ExynosLogicFn   : TIIPredicate<
                        "isExynosLogicFast",
                        MCOpcodeSwitchStatement<
                          [MCOpcodeSwitchCase<
                             IsLogicShiftOp.ValidOpcodes,
                             MCReturnStatement<ExynosCheckShift>>,
                           MCOpcodeSwitchCase<
                             IsLogicUnshiftOp.ValidOpcodes,
                             MCReturnStatement<TruePred>>,
                           MCOpcodeSwitchCase<
                             IsLogicImmOp.ValidOpcodes,
                             MCReturnStatement<TruePred>>],
                          MCReturnStatement<FalsePred>>>;
def ExynosLogicPred : MCSchedPredicate<ExynosLogicFn>;

// Identify more logic instructions with limited shift.
def ExynosLogicExFn   : TIIPredicate<
                          "isExynosLogicExFast",
                          MCOpcodeSwitchStatement<
                            [MCOpcodeSwitchCase<
                               IsLogicShiftOp.ValidOpcodes,
                               MCReturnStatement<
                                 CheckAny<
                                   [ExynosCheckShift,
                                    CheckAll<
                                     [CheckShiftLSL,
                                      CheckShiftBy8]>]>>>,
                             MCOpcodeSwitchCase<
                               IsLogicUnshiftOp.ValidOpcodes,
                               MCReturnStatement<TruePred>>,
                             MCOpcodeSwitchCase<
                               IsLogicImmOp.ValidOpcodes,
                               MCReturnStatement<TruePred>>],
                            MCReturnStatement<FalsePred>>>;
def ExynosLogicExPred : MCSchedPredicate<ExynosLogicExFn>;

// Identify a load or store using the register offset addressing mode
// with a scaled non-extended register.
def ExynosScaledIdxFn   : TIIPredicate<"isExynosScaledAddr",
                                       MCOpcodeSwitchStatement<
                                         [MCOpcodeSwitchCase<
                                            IsLoadStoreRegOffsetOp.ValidOpcodes,
                                            MCReturnStatement<
                                              CheckAny<
                                                [CheckMemExtSXTW,
                                                 CheckMemExtUXTW,
                                                 CheckMemScaled]>>>],
                                         MCReturnStatement<FalsePred>>>;
def ExynosScaledIdxPred : MCSchedPredicate<ExynosScaledIdxFn>;

// Identify FP instructions.
def ExynosFPPred : MCSchedPredicate<CheckFpOrNEON>;

// Identify 128-bit NEON instructions.
def ExynosQFormPred : MCSchedPredicate<CheckQForm>;

// Identify instructions that reset a register efficiently.
def ExynosResetFn   : TIIPredicate<
                        "isExynosResetFast",
                        MCOpcodeSwitchStatement<
                          [MCOpcodeSwitchCase<
                             [ADR, ADRP,
                              MOVNWi, MOVNXi,
                              MOVZWi, MOVZXi],
                             MCReturnStatement<TruePred>>,
                           MCOpcodeSwitchCase<
                             [ORRWri, ORRXri],
                             MCReturnStatement<CheckIsReg1Zero>>],
                          MCReturnStatement<
                            CheckAny<
                              [IsCopyIdiomFn,
                               IsZeroFPIdiomFn]>>>>;
def ExynosResetPred : MCSchedPredicate<ExynosResetFn>;

// Identify cheap arithmetic and logic immediate instructions.
def ExynosCheapFn : TIIPredicate<
                      "isExynosCheapAsMove",
                      MCOpcodeSwitchStatement<
                        [MCOpcodeSwitchCase<
                           IsArithLogicImmOp.ValidOpcodes,
                           MCReturnStatement<TruePred>>],
                        MCReturnStatement<
                          CheckAny<
                            [ExynosArithFn, ExynosResetFn, ExynosLogicFn]>>>>;
