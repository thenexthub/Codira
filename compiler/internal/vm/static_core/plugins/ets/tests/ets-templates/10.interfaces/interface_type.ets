/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Declaring an interface type.
assert: An interface declaration declares an interface type.
---*/

interface I {}

class C implements I {}

interface J {}

class D implements I, J {}

class E extends C {}

class F extends C implements J {}

class G extends D {}

function main(): void {
  let c: C = new C();
  arktest.assertTrue(
    c instanceof I, 
    "Type C expected to be subtype I as interface implementation"
  );
  arktest.assertFalse(
    c instanceof J, 
    "Type C expected not to be subtype J because it doesn't implement interface"
  );
  
  let d: D = new D();
  arktest.assertTrue(
    d instanceof I && d instanceof J, 
    "Type D expected to be subtype I and J as both interface implementation"
  );
  
  let e: E = new E();
  arktest.assertTrue(
    e instanceof I, 
    "Type E expected to be subtype I as implementing class extension"
  );
  
  let f: F = new F();
  arktest.assertTrue(
    f instanceof I && f instanceof J, 
    "Type F expected to be subtype I and J as implementing class extension and interface implementation"
  );
  
  let g: G = new G();
  arktest.assertTrue(
    g instanceof I && g instanceof J,
    "Type G expected to be subtype I and J as both interfaces implementing class extension"
  );
}