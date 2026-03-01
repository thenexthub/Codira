/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


import * as ns from '../mod1';

// definesendableclass
class A {
  constructor() {
    "use sendable";
  }
}

// ldsendableclass
class B {
  constructor() {
    "use sendable";
  }
  static b: number = 1;
  b: number = B.b;
}

// Class Inheritance
class C {
  constructor() {
    "use sendable";
  }
  func1(): void { }
  func2(): void { }
}

class D extends C {
  constructor() {
    "use sendable";
    super();
  }
  func2():void { }
}

// Test GetParClassExternalModuleName Method
class ExtendPhoneService extends ns.Phone.PhoneService {
  constructor() {
    "use sendable";
    super();
  }
}

// Test GetParClassGlobalVarName Method
class ExtendDataItem extends globalvar2.Data.DataItem {
  constructor() {
    "use sendable";
    super();
  }
}


