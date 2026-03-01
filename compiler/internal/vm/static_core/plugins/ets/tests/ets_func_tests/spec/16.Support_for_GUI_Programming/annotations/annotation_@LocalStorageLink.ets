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
 desc: 05.02.02 Example for using AppStorage and LocalStorage from inside UI
 name: 05.ui_state_storages/05.02_storage_api_for_application-wide_ui_state_appstorage/05.02.02_example_for_using_appstorage_and_localstorage_from_inside_ui/annotation_@LocalStorageLink
 tags: [compile-only]
 ---*/

// set PropA in AppStorage, if the singleton does not exist yet, it will be created on the fly
AppStorage.setOrCreate("PropA", 47); 

// create new instance and init with given object
let storage = new LocalStorage({"PropA": 48}); 

@Entry(storage)
@Component
struct CompA {
   
    // @StorageLink variable decorator creates two-way link with property "PropA" in AppStorage
    // initial value will be 47, because set in above code
    @StorageLink("PropA") storLink : number = 1;

    // @LocalStorageLink variable decorator creates two-way link with property "PropA" in LocalStorage
    // initial value will be 48, because set in above code
    @LocalStorageLink("PropA") localStorLink : number = 1;

    build() {
        Column({ space: 20 }) {
        Text(`From AppStorage ${this.storLink}`)
            // changes will sync with "PropA" in AppStorage, not in LocalStorage
            .onClick(() => this.storLink+=1 ) 

        Text(`From LocalStorage ${this.localStorLink}`)
            // changes will sync with "PropA" in LocalStorage, not in AppStorage
            .onClick(() => this.localStorLink+=1 ) 
       }
   }
}