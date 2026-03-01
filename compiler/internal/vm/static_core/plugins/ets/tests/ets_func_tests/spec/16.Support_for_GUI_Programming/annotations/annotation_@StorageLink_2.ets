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
 desc: 05.07.01 Example of use
 name: 05.ui_state_storages/05.07_synchronization_between_appstorage_and_ui_components_using_@StorageLink_and_@StorageProp_decorators/05.07.01_example_of_use/annotation_@StorageLink_2
 tags: [negative, compile-only]
 ---*/

 // in non-UI logic
AppStorage.setOrCreate("PropA", 47); 
let linkToPropA = AppStorage.link("PropA"); 

@Entry
@Component
struct CompA {
   
    // @StorageLink variable decorator creates two-way link with property "PropA" in AppStorage
    // initial value will be 47, because set in above code
    @StorageLink("PropA") storLink : number = 1;

    build() {
        Column() {
          // bad practise and will not always work to use global vars in UI
          Text(`@StorageLink: ${this.storLink} - linkToPropA: ${linkToPropA.get()}`)
        }
    }
}