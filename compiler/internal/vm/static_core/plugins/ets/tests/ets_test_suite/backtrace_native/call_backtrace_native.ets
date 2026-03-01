/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

native function callStepArkByNativeFrame(useExtractor: boolean): int;

function __noinline__Test_StaticArk_Backtrace():int {
  return __noinline__Test_StaticArk_Backtrace1();
}

function __noinline__Test_StaticArk_Backtrace1(): int {
  return __noinline__Test_StaticArk_Backtrace2();
}

function __noinline__Test_StaticArk_Backtrace2(): int {
  return __noinline__Test_StaticArk_Backtrace3();
}

function __noinline__Test_StaticArk_Backtrace3(): int {
  let ret = 0;
  let ret1 = callStepArkByNativeFrame(false);
  let ret2 = callStepArkByNativeFrame(true);
  if (ret1 != ret2) {
      return ret;
  } else {
    ret = ret1;
  }
  return ret1;
}

function __noinline__Test_StaticArk_Backtrace_catch_error_ets(): int {
  let ret = 0;
  try {
    ret = __noinline__Test_StaticArk_Backtrace_catch_error_ets1();
  } catch(e) {
    let ret1 = callStepArkByNativeFrame(false);
    let ret2 = callStepArkByNativeFrame(true);
    if (ret1 != ret2) {
      return ret;
    } else {
      ret = ret1;
    }
  }
  return ret;
}

function __noinline__Test_StaticArk_Backtrace_catch_error_ets1(): int {
  return __noinline__Test_StaticArk_Backtrace_catch_error_ets2();
}

function __noinline__Test_StaticArk_Backtrace_catch_error_ets2(): int {
  return __noinline__Test_StaticArk_Backtrace_catch_error_ets3();
}

function __noinline__Test_StaticArk_Backtrace_catch_error_ets3(): int {
  throw new Error("Throw Error in ets");
}

native function throwErrorInNative(): int;

function __noinline__Test_StaticArk_Backtrace_catch_error_native(): int {
  let ret = 0;
  try {
    ret = __noinline__Test_StaticArk_Backtrace_catch_error_native1();
  } catch(e) {
    let ret1 = callStepArkByNativeFrame(false);
    let ret2 = callStepArkByNativeFrame(true);
    if (ret1 != ret2) {
      return ret;
    } else {
      ret = ret1;
    }
  }
  return ret;
}

function __noinline__Test_StaticArk_Backtrace_catch_error_native1(): int {
  return __noinline__Test_StaticArk_Backtrace_catch_error_native2();
}

function __noinline__Test_StaticArk_Backtrace_catch_error_native2(): int {
  return __noinline__Test_StaticArk_Backtrace_catch_error_native3();
}

function __noinline__Test_StaticArk_Backtrace_catch_error_native3(): int {
  return throwErrorInNative();
}

function main(): void {
  loadLibrary("call_backtrace_native");

  // Expected value of the depth of ETS function calls is 5 :
  // #00 __noinline__Test_StaticArk_Backtrace3()
  // #01 __noinline__Test_StaticArk_Backtrace2()
  // #02 __noinline__Test_StaticArk_Backtrace1()
  // #03 __noinline__Test_StaticArk_Backtrace()
  // #04 main()
  let res1 = __noinline__Test_StaticArk_Backtrace();
  if (res1 >= 0) {
    arktest.assertEQ(res1, 5);
  }

  // Expected value of the depth of ETS function calls is 2 :
  // #00 __noinline__Test_StaticArk_Backtrace_catch_error_ets()
  // #01 main()
  let res2 = __noinline__Test_StaticArk_Backtrace_catch_error_ets();
  if (res2 >= 0) {
    arktest.assertEQ(res2, 2);
  }
  
  // Expected value of the depth of ETS function calls is 2 :
  // #00 __noinline__Test_StaticArk_Backtrace_catch_error_native()
  // #01 main()
  let res3 = __noinline__Test_StaticArk_Backtrace_catch_error_native();
  if (res3 >= 0) {
    arktest.assertEQ(res3, 2);
  }
}