/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 04.01 State owned by a Component @State
 name: 04.managing_state_owned_by_a_component/04.01_state_owned_by_a_component_@State/annotation_@State
 tags: [compile-only]
 ---*/

@Component
struct HelloComponent {
  @State say: string = 'Ooops!';
  build() {
    Row() {
      Text(this.say)
        .fontSize(20)
        .fontColor(Color.Red)
    }
    .backgroundColor(Color.Green)
  }
}

@Component
struct ParentComponent {
  build() {
    Column() {
      Text('ArkUI says:')
        .fontSize(20)
        HelloComponent({say: "Hello, World!"})
        Divider()
         .height(10)
        HelloComponent({say: "Goodbye, World!"})
        Divider()
          .height(10)
        HelloComponent()
    }
  }
}