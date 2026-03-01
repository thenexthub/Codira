/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Super method call. 
---*/

class Shape {
  public constructor () {}

  public draw(): boolean {
    return true;
  }
}

class Rectangle extends Shape {
  protected height: int = 0;
  protected width: int = 0;

  public constructor (h: int, w: int) {
    this.height = h;
    this.width = w;
  }
}

final class FilledRectangle extends Rectangle {
  color: String = "";

  constructor (h: int, w: int, c: String) {
    super(h, w);
    this.color = c;
  }

  override draw(): boolean {
    return super.draw();
  }
}

function main(): void {
  let instance: FilledRectangle = new FilledRectangle(5, 5, "color");

  arktest.assertEQ( instance.draw(), true );
}
