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
 desc: 02.02.03 Default page entry component @Entry
 name: 02.general_ui_specifications/02.02_subdivision_into_custom_components/02.02.03_default_page_entry_component_@Entry/annotation_@Entry
 tags: [compile-only]
 ---*/

@Component
struct HideComponent {
  @State say: string = 'Goodbye';
  build() {
    Column() {
      Text(this.say)
        .fontColor(Color.Blue)
    }
  }
}

@Entry
@Component
struct MyComponent {
  build() {
    Column() {
      HideComponent({say: "Hello, World!"})
    }
  }
}