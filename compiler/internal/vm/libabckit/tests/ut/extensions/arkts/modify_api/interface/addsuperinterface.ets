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

interface Style {
  color: string;
  showColor():void;
}

interface ExtendedStyle {
  width: number;
  calculateAreaSize(): number;
}

class RectangleSize implements ExtendedStyle {
  public width: number;
  public height: number;
  constructor(w: number, h:number) {
    this.width = w;
    this.height = h;
  }
  calculateAreaSize(): number {
    return this.width * this.height;
  }
}

function main() {
    let c = new RectangleSize(20, 30);
    console.log(c.calculateAreaSize());
}

