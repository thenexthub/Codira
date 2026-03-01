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
 desc: 04.05.04 @ObjectLink and @Observed Class Decorator Scenario Nested Objects
 name: 04.managing_state_owned_by_a_component/04.05_object_reference_with_@ObjectLink/04.05.04_@ObjectLink_and_@Observed_class_decorator_scenario_nested_objects/annotation_@ObjectLink
 tags: [compile-only]
 ---*/

@Component
struct ViewA {

  @ObjectLink a : ClassA;
  label : string = "ViewA1";

  build() {
     Row() {
        Button(`ViewA [${this.label}] this.a.c=${this.a.c} +1`)
        .onClick(() => {
            this.a.c += 1;
        })
     }
  }
}

@Entry
@Component 
struct ViewB {
  @State b : ClassB = new ClassB(new ClassA(0));

  build() {
     Column() {
       ViewA({ label: "ViewA #1", a: this.b.a })
       ViewA({ label: "ViewA #2", a: this.b.a })

       Button(`ViewB: this.b.a.c+= 1`)
        .onClick(() => {
            this.b.a.c +=1;
        })  
        Button(`ViewB: this.b.a = new ClassA(0)`)
        .onClick(() => {
            this.b.a = new ClassA(0);
        })  
        Button(`ViewB: this.b = new ClassB(ClassA(0))`)
        .onClick(() => {
            this.b = new ClassB(new ClassA(0));
        }) 
    }
  } 
}