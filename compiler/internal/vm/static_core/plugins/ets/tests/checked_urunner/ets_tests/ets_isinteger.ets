/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! CHECKER     Codegen inlining ETS Float.isInteger
//! RUN         force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_isinteger.ETSGLOBAL::main"
//! ASM_METHOD  "ets_isinteger.ETSGLOBAL::main"
//! ASM_INST    "Intrinsic.StdFloatIsInteger"
//! ASM      x64: /xor.*/,                 arm64: /frintz.*/,             arm32: /vrintz\.f32.*/
//! ASM_NEXT x64: /roundss.*/,             arm64: /fsub.*/,               arm32: /vsub\.f32.*/
//! ASM_NEXT x64: /movss.*/,               arm64: /fabs.*/,               arm32: /vabs\.f32.*/
//! ASM_NEXT x64: /subss.*/,               arm64: /fcmp.*/,               arm32: /mov\s+r[0-9]+,.*/
//! ASM_NEXT x64: /movss.*/,               arm64: /b\.vs.*/,              arm32: /vmov.*/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFF,.*/, arm64: /cset\s+w[0-9]+,\s+le/, arm32: /vcmp\.f32.*/
//! ASM_NEXT x64: /movd.*/,                arm64: /b\s+.*/,               arm32: /vmrs APSR_nzcv, FPSCR/
//! ASM_NEXT x64: /andps.*/,               arm64: /mov\s+w[0-9]+, #0x0/,  arm32: /bvs.*/
//! ASM_NEXT x64: /mov\s+\$0x00,.*/,                                      arm32: /movle\s+r[0-9]+,\s+#1/
//! ASM_NEXT x64: /movd.*/,                                               arm32: /movgt\s+r[0-9]+,\s+#0/
//! ASM_NEXT x64: /ucomiss.*/,                                            arm32: /b.*/
//! ASM_NEXT x64: /jp.*/,                                                 arm32: /mov\s+r[0-9]+,\s+#0/
//! ASM_NEXT x64: /setbe.*/
//! ASM_INST "Intrinsic.StdFloatIsSafeInteger"
//! ASM      x64: /xor.*/,                 arm64: /frintz.*/,             arm32: /vrintz\.f32.*/
//! ASM_NEXT x64: /roundss.*/,             arm64: /fsub.*/,               arm32: /vsub\.f32.*/
//! ASM_NEXT x64: /movss.*/,               arm64: /fabs.*/,               arm32: /vabs\.f32.*/
//! ASM_NEXT x64: /subss.*/,               arm64: /fcmp.*/,               arm32: /mov\s+r[0-9]+,.*/
//! ASM_NEXT x64: /movss.*/,               arm64: /b\.vs.*/,              arm32: /vmov.*/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFF,.*/, arm64: /b\.gt.*/,              arm32: /vcmp\.f32.*/
//! ASM_NEXT x64: /movd.*/,                arm64: /fabs.*/,               arm32: /vmrs APSR_nzcv, FPSCR/
//! ASM_NEXT x64: /andps.*/,               arm64: /ldr.*/,                arm32: /bvs.*/
//! ASM_NEXT x64: /mov\s+\$0x00,.*/,       arm64: /fcmp.*/,               arm32: /bgt.*/
//! ASM_NEXT x64: /movd.*/,                arm64: /cset\s+w[0-9]+,\s+le/, arm32: /vabs\.f32/
//! ASM_NEXT x64: /ucomiss.*/,             arm64: /b\s+.*/,               arm32: /mov\s+r[0-9]+,\s+#ffff/
//! ASM_NEXT x64: /jp.*/,                  arm64: /mov\s+w[0-9]+, #0x0/,  arm32: /movt\s+r[0-9]+,\s+#4b7f/
//! ASM_NEXT x64: /jnbe.*/,                                               arm32: /vmov.*/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFF,.*/,                                arm32: /vcmp\.f32.*/
//! ASM_NEXT x64: /movd.*/,                                               arm32: /vmrs APSR_nzcv, FPSCR/
//! ASM_NEXT x64: /movss.*/,                                              arm32: /movle\s+r[0-9]+,\s+#1/
//! ASM_NEXT x64: /andps.*/,                                              arm32: /movgt\s+r[0-9]+,\s+#0/
//! ASM_NEXT x64: /mov\s+\$0x4B7FFFFF,.*/,                                arm32: /b.*/
//! ASM_NEXT x64: /movd.*/,                                               arm32: /mov\s+r[0-9]+,\s+#0/
//! ASM_NEXT x64: /ucomiss.*/
//! ASM_NEXT x64: /setbe.*/
//! ASM_INST "Intrinsic.StdDoubleIsInteger"
//! ASM      x64: /xor.*/,                         arm64: /frintz.*/,             arm32: /vrintz\.f64.*/
//! ASM_NEXT x64: /roundsd.*/,                     arm64: /fsub.*/,               arm32: /vsub\.f64.*/
//! ASM_NEXT x64: /movsd.*/,                       arm64: /fabs.*/,               arm32: /vabs\.f64.*/
//! ASM_NEXT x64: /subsd.*/,                       arm64: /fcmp.*/,               arm32: /vmov.*/
//! ASM_NEXT x64: /movsd.*/,                       arm64: /b\.vs.*/,              arm32: /vcmp.*/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFFFFFFFFFF,.*/, arm64: /cset\s+w[0-9]+,\s+le/, arm32: /vmrs.*/
//! ASM_NEXT x64: /movq.*/,                        arm64: /b\s+.*/,               arm32: /bvs.*/
//! ASM_NEXT x64: /andps.*/,                       arm64: /mov\s+w[0-9]+, #0x0/,  arm32: /movle.*/
//! ASM_NEXT x64: /mov\s+\$0x00,.*/,                                              arm32: /movgt.*/
//! ASM_NEXT x64: /movq.*/,                                                       arm32: /b.*/
//! ASM_NEXT x64: /ucomisd.*/,                                                    arm32: /mov.*/
//! ASM_NEXT x64: /jp.*/
//! ASM_NEXT x64: /setbe.*/
//! ASM_INST "Intrinsic.StdDoubleIsSafeInteger"
//! ASM      x64: /xor.*/,                         arm64: /frintz.*/,             arm32: /vrintz\.f64.*/
//! ASM_NEXT x64: /roundsd.*/,                     arm64: /fsub.*/,               arm32: /vsub\.f64.*/
//! ASM_NEXT x64: /movsd.*/,                       arm64: /fabs.*/,               arm32: /vabs\.f64.*/
//! ASM_NEXT x64: /subsd.*/,                       arm64: /fcmp.*/,               arm32: /vmov.*/
//! ASM_NEXT x64: /movsd.*/,                       arm64: /b\.vs.*/,              arm32: /vcmp.*/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFFFFFFFFFF,.*/, arm64: /b\.gt.*/,              arm32: /vmrs.*/
//! ASM_NEXT x64: /movq.*/,                        arm64: /fabs.*/,               arm32: /bvs.*/
//! ASM_NEXT x64: /andps.*/,                       arm64: /ldr.*/,                arm32: /bgt.*/
//! ASM_NEXT x64: /mov\s+\$0x00,.*/,               arm64: /fcmp.*/,               arm32: /vabs\.f64.*/
//! ASM_NEXT x64: /movq.*/,                        arm64: /cset\s+w[0-9]+,\s+le/, arm32: /mvn.*/
//! ASM_NEXT x64: /ucomisd.*/,                     arm64: /b\s+.*/,               arm32: /vdup\.32.*/
//! ASM_NEXT x64: /jp.*/,                          arm64: /mov\s+w[0-9]+, #0x0/,  arm32: /mov\s+r[0-9]+,\s+#ffff/
//! ASM_NEXT x64: /jnbe.*/,                                                       arm32: /movt\s+r[0-9]+,\s+#433f/
//! ASM_NEXT x64: /mov\s+\$0x7FFFFFFFFFFFFFFF,.*/,                                arm32: /vmov\.32.*/
//! ASM_NEXT x64: /movq.*/,                                                       arm32: /vcmp\.f64.*/
//! ASM_NEXT x64: /movsd.*/,                                                      arm32: /vmrs APSR_nzcv, FPSCR/
//! ASM_NEXT x64: /andps.*/,                                                      arm32: /movle\s+r[0-9]+,\s+#1/
//! ASM_NEXT x64: /mov\s+\$0x433FFFFFFFFFFFFF,.*/,                                arm32: /movgt.*/
//! ASM_NEXT x64: /movq.*/,                                                       arm32: /b.*/
//! ASM_NEXT x64: /ucomisd.*/,                                                    arm32: /mov\s+r[0-9]+,\s+#0/
//! ASM_NEXT x64: /setbe.*/

function main(): int {
    let trueInt: float = 1.0f;
    if (Float.isInteger(trueInt) != true)
        return 1;

    let maxSafeInt: float = Float.MAX_SAFE_INTEGER;
    if (Float.isSafeInteger(maxSafeInt) != true)
        return 2;

    let trueIntDbl: double = 1.0;
    if (Double.isInteger(trueIntDbl) != true)
        return 3;

    let maxSafeIntDbl: double = Double.MAX_SAFE_INTEGER;
    if (Double.isSafeInteger(maxSafeIntDbl) != true)
        return 4;

    return 0;
}
