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
 name: 04.managing_state_owned_by_a_component/04.03.bi-bidirectional_syncing_with_a_variable_of_parent_component_@Link/annotation_@Link
 tags: [compile-only]
 ---*/

@Component
struct ChildA {
    
  @Prop counterVal : number;
    build() {
      Button(`ChildA: this.counterVal(${this.counterVal}) + 1`)
        .onClick(() => this.counterVal += 1)
        .width(400).height(100) 
    }
}

@Component
struct ChildB {
    
  @Link counterRef : number;
    build() {
      Button(`ChildB: this.counterRef(${this.counterRef}) + 1`)
        .onClick(() => this.counterRef += 1)
        .width(400).height(100) 
    }
}

@Entry
@Component
struct ParentView {

  initialCounterValue : number = 1;
  @State counter : number = this.initialCounterValue;
    build() {
      Column() {
        ChildA({ counterVal: this.counter })
          ChildB({ counterRef: this.counter })
          ChildB({ counterRef: this.counter })
          Text(`Parent: this.counter= ${this.counter}`)
           .width(400).height(100) 
      }
    }
}