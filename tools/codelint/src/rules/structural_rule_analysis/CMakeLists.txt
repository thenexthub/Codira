# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

include_directories(${CMAKE_SOURCE_DIR}/src)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
aux_source_directory(./ ALL_SRCS)
aux_source_directory(./StructuralRuleGCHK02 G_CHK_02)
aux_source_directory(./StructuralRuleGCHK04 G_CHK_04)
aux_source_directory(./StructuralRuleGCLS01 G_CLS_01)
aux_source_directory(./StructuralRuleGCON01 G_CON_01)
aux_source_directory(./StructuralRuleGCON02 G_CON_02)
aux_source_directory(./StructuralRuleGCON03 G_CON_03)
aux_source_directory(./StructuralRuleGENU01 G_ENU_01)
aux_source_directory(./StructuralRuleGENU02 G_ENU_02)
aux_source_directory(./StructuralRuleGERR01 G_ERR_01)
aux_source_directory(./StructuralRuleGERR02 G_ERR_02)
aux_source_directory(./StructuralRuleGERR03 G_ERR_03)
aux_source_directory(./StructuralRuleGERR04 G_ERR_04)
aux_source_directory(./StructuralRuleGEXP01 G_EXP_01)
aux_source_directory(./StructuralRuleGEXP02 G_EXP_02)
aux_source_directory(./StructuralRuleGEXP03 G_EXP_03)
aux_source_directory(./StructuralRuleGEXP04 G_EXP_04)
aux_source_directory(./StructuralRuleGEXP05 G_EXP_05)
aux_source_directory(./StructuralRuleGEXP06 G_EXP_06)
aux_source_directory(./StructuralRuleGEXP07 G_EXP_07)
aux_source_directory(./StructuralRuleGFMT01 G_FMT_01)
aux_source_directory(./StructuralRuleGFMT13 G_FMT_13)
aux_source_directory(./StructuralRuleGFMT15 G_FMT_15)
aux_source_directory(./StructuralRuleGFUN01 G_FUN_01)
aux_source_directory(./StructuralRuleGFUN02 G_FUN_02)
aux_source_directory(./StructuralRuleGFUNC03 G_FUN_03)
aux_source_directory(./StructuralRuleGNAM01 G_NAM_01)
aux_source_directory(./StructuralRuleGNAM02 G_NAM_02)
aux_source_directory(./StructuralRuleGNAM03 G_NAM_03)
aux_source_directory(./StructuralRuleGNAM04 G_NAM_04)
aux_source_directory(./StructuralRuleGNAM05 G_NAM_05)
aux_source_directory(./StructuralRuleGNAM06 G_NAM_06)
aux_source_directory(./StructuralRuleGDCL01 G_DCL_01)
aux_source_directory(./StructuralRuleGDCL02 G_DCL_02)
aux_source_directory(./StructuralRuleGOPR01 G_OPR_01)
aux_source_directory(./StructuralRuleGOPR02 G_OPR_02)
aux_source_directory(./StructuralRuleGOTH02 G_OTH_02)
aux_source_directory(./StructuralRuleGOTH03 G_OTH_03)
aux_source_directory(./StructuralRuleGOTH04 G_OTH_04)
aux_source_directory(./StructuralRuleGPKG01 G_PKG_01)
aux_source_directory(./StructuralRuleGSEC01 G_SEC_01)
aux_source_directory(./StructuralRuleGSER01 G_SER_01)
aux_source_directory(./StructuralRuleGSER02 G_SER_02)
aux_source_directory(./StructuralRuleGVAR02 G_VAR_02)
aux_source_directory(./StructuralRuleGVAR03 G_VAR_03)
aux_source_directory(./StructuralRuleGITF01 G_ITF_01)
aux_source_directory(./StructuralRuleGITF02 G_ITF_02)
aux_source_directory(./StructuralRuleGITF04 G_ITF_04)
aux_source_directory(./StructuralRuleP01 P_01)
aux_source_directory(./StructuralRuleP03 P_03)
aux_source_directory(./StructuralRuleGITF03 G_ITF_03)
aux_source_directory(./StructuralRuleGSYN01 G_SYN_01)
aux_source_directory(./StructuralRuleGTYP03 G_TYP_03)
aux_source_directory(./StructuralRuleFFIC7 FFI_C_7)

add_library(structuralrule STATIC
        ${ALL_SRCS}
        ${G_CHK_02}
        ${G_CHK_04}
        ${G_CLS_01}
        ${G_CON_01}
        ${G_CON_02}
        ${G_CON_03}
        ${G_ENU_01}
        ${G_ENU_02}
        ${G_ERR_01}
        ${G_ERR_02}
        ${G_ERR_03}
        ${G_ERR_04}
        ${G_EXP_01}
        ${G_EXP_02}
        ${G_EXP_03}
        ${G_EXP_04}
        ${G_EXP_05}
        ${G_EXP_06}
        ${G_EXP_07}
        ${G_FMT_01}
        ${G_FMT_13}
        ${G_FMT_15}
        ${G_FUN_01}
        ${G_FUN_02}
        ${G_FUN_03}
        ${G_NAM_01}
        ${G_NAM_02}
        ${G_NAM_03}
        ${G_NAM_04}
        ${G_NAM_05}
        ${G_NAM_06}
        ${G_DCL_01}
        ${G_DCL_02}
        ${G_OPR_01}
        ${G_OPR_02}
        ${G_OTH_02}
        ${G_OTH_03}
        ${G_OTH_04}
        ${G_PKG_01}
        ${G_SEC_01}
        ${G_SER_01}
        ${G_SER_02}
        ${G_VAR_02}
        ${G_VAR_03}
        ${G_ITF_01}
        ${G_ITF_02}
        ${G_ITF_04}
        ${P_01}
        ${P_03}
        ${G_ITF_03}
        ${G_SYN_01}
        ${G_TYP_03}
        ${FFI_C_7}
)
if(COVERAGE_FLAG)
    target_compile_options(structuralrule PUBLIC "-fno-exceptions")
endif()