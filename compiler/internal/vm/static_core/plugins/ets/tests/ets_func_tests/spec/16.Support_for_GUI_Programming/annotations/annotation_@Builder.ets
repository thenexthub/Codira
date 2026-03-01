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
 desc: 02.04.06 Custom component variable holding a @Builder function reference @BuilderParam and BuilderType<C>
 name: 02.general_ui_specifications/02.04_custom_build_functions_@Builder/02.04.06_custom_component_variable_holding_a_@Builder_function_reference_@BuilderParam_and_BuilderType<C>/annotation_@Builder
 tags: [compile-only]
 ---*/

@Builder function GlobalBuilder0() {  // short for @Builder function GlobalBuilder0($$ : {}) 
  Text("0 - global builder")
        .width(400)
        .height(50)
        .backgroundColor(Color.Yellow)
}

@Builder function GlobalBuilder1($$ : {label: string }) {
  Text($$.label)
        .width(400)
        .height(50)
        .backgroundColor(Color.Blue)
}

@Component
struct CC {

  @Prop size_ : number;

  @Builder doNothingBuilder() { 
    // does nothing
  };
  
  @BuilderParam aBuilder0 : BuilderType<{}> = this.doNothingBuilder;
  @BuilderParam aBuilder1 : BuilderType<{ label : string }>;
  
  build() {
    Column() {
      this.aBuilder0()       // short for this.aBuilder0({})
      this.aBuilder1({label: "1 - global Builder label" } )
    }
    .height(3*50)
  }
}

@Entry
@Component
struct CCUser {

  @State size_ : number = 1;

  @Builder componentBuilder() {   // short for @Builder componentBuilder( $$ : {})
      Text(`2 + 3 - component builder`)
        .width(400)
        .height(50)
        .backgroundColor(Color.Green)
    }

  build() {
    Column() {
      Text("Start")

      CC({size_: 1, aBuilder0: GlobalBuilder0, aBuilder1: GlobalBuilder1})

      Text("End")
    }
  }
}