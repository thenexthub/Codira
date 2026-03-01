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
 desc: 06.01.03 @Watch and custom component update example
 name: 06.other _state_management_functionality/06.01_get_notified_of_state_variable_changes_@Watch/06.01.03_@Watch_and_custom_component_update_example/06.other _state_management_functionality/06.01_get_notified_of_state_variable_changes_@Watch/annotation_@Watch_2
 tags: [compile-only]
 ---*/

@Component
struct TotalView {
  @Prop @Watch("onCountUpdated") count : number = 0;
  @State total : number = 0;
  // @Watch cb
  onCountUpdated(propName: string) : void {
    this.total += this.count;
  }
  build() {
    Text(`Total: ${this.total}`)
  }
}

@Entry
@Component
struct CountModifier {
  @State count : number = 0;
    build() {
      Column() {
        Button("add to basket")
          .onClick(() => { this.count++ })
        TotalView({ count: this.count })
      }
    }
}