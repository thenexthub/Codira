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

/*---
desc: >-
    After the type to use is known, the method to call must be determined.
    If a method name in the call refers an overload declaration (see Overload Declarations),
    then Overload Resolution for Overload Declarations is used to select the method to call.
    A compile-time error occurs if no method is available to call.
---*/

interface I {
    meth(s: String): String;
}

class A implements I {
  public override meth(s: String): String {
      return s + "A";
  }
}

class B extends A {
  public override meth(s: String): String {
      return super.meth(s) + "B";
  }
}

function main() {

	let a: A = new A();
	arktest.assertEQ(a.meth("X"), "XA")

	a = new B();
	arktest.assertEQ(a.meth("Y"), "YAB")

	let i: I = new A();
	arktest.assertEQ(i.meth("Z"), "ZA")

	i = new B();
	arktest.assertEQ(i.meth("W"), "WAB")
    
}
