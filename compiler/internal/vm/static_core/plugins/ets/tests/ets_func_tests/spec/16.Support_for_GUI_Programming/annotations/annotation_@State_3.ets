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
 desc: 04.04 Bi-directionally syncing state with descendent components @Provide and @Consume
 name: 04.managing_state_owned_by_a_component/04.04_bi-directionally_syncing_state_with_descendent_components_@Provide_and_@Consume/annotation_@State_3
 tags: [negative, compile-only]
 ---*/

@Component
struct CompD {

  @State reviewVotes : number;
    build() {
        Column() {
          Text(`reviewVotes(${this.reviewVotes})`)
          Button(`reviewVotes(${this.reviewVotes}), give +1`)
            .onClick(() => this.reviewVotes += 1 )
        }
        .border({ width: 3, color: Color.Black })
        .margin(5)
    }
}

@Component
struct CompC {

  build() {
    Row({ space: 5 }) {
      CompD()
      CompD()
    }
  }
}

@Component
struct CompB {

  build() {
    CompC()
  }
}
