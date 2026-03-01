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
 desc: 04.03 Bi-bidirectional syncing with a variable of parent component @Link
 name: 04.managing_state_owned_by_a_component/04.03_bi-bidirectional_syncing_with_a_variable_of_parent_component_@Link/04.03.03_example_for_@Link_with_array_type/annotation_@Link_2
 tags: [compile-only]
 ---*/

@Component
struct Child {
  @Link items: number[];
  build() {
    Column() {
      Button() {
        Text("Button1: push")
      }.onClick(() => {
        this.items.push(this.items.length+1);
      })
      Button() {
        Text("Button2: replace whole item")
      }.onClick(() => {
        this.items = [100, 200, 300];
      })
    }
  }
}

@Entry
@Component
struct Parent {
  @State arr: number[] = [1, 2, 3];
  build() {
    Column() {
      Child({ items: this.arr })
      ForEach(this.arr,
        item => {
          Text(`${item}`)
        },
        item => item.toString()
      )
    }
  }
}