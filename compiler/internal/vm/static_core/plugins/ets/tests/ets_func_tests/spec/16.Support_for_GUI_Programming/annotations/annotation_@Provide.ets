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
 desc: 04.07.02 Decorated @Component variables of type Set
 name: 04.managing_state_owned_by_a_component/04.07_more_examples/04.07.02_decorated_@Component_variables_of_type_set/annotation_@Provide
 tags: [compile-only]
 ---*/

@Component
struct LinkChild {
    @Link single: ClassA;
    @Link data: MySet<ClassA>;

    build() {
        Column() {
            Text(`@Link data.size ${this.data.size}`)
            ForEach(this.data.items, item => {
                Text(`#${item.id} -- ${item.msg}`)
            },
                item => item.id
            )

            Button("clear").width(300).height(70)
            .onClick(() => { 
                this.data.clear();
            })
            Button("add single object").width(300).height(70)
            .onClick(() => { 
                this.data.add(this.single);
            })
        }
        .border({ color: Color.Blue, width: 3})
    }
}

@Component
struct ConsumeChild {
    @Consume data: MySet<ClassA>;

    build() {
        Column({space: 10}) {
            Text(`@Consume child finds following ids`)
            Row({ space: 3}) {
                ForEach(this.data.items, item => {
                    Text(`#${item.id}`)
                },
                    item => item.id
                )
            }
        }
    }
}

@Component
struct PropChild {
    @Prop data: MySet<ClassA>;

    build() {
        Column() {
            Text(`@Prop data.size ${this.data.size}`)
            ForEach(this.data.items, item => {
                Text(`#${item.id} -- ${item.msg}`)
            },
                item => item.id
            )

            ConsumeChild()

            Button("clear").width(300).height(70)
            .onClick(() => { 
                this.data.clear(); 
            })
        }
        .border({ color: Color.Red, width: 3})
    }
}

let next = 4;

@Entry
@Component
struct Parent {
    @State single: ClassA;
    @Provide data: MySet<ClassA> = new MySet([this.single, new ClassA(3, "33")]);

    build() {
      List() {
        Column({ space: 10 }) {
            Text(`Parent data.size ${this.data.size}`)
            ForEach(this.data.items, item => {
                Text(`#${item.id} -- ${item.msg}`)
            },
                item => item.id
            )

            Button("add new").width(300).height(70)
            .onClick(() => { 
                this.data.add(new ClassA(next, `this is item ${next}`));
                next++;
            })
            Button("delete single object").width(300).height(70)
            .onClick(() => { 
                if (this.data.has(this.single)) {
                    this.data.delete(this.single);
                }
            })

            Divider()

            LinkChild({ single: this.$single, data: this.$data })
            PropChild({ data: this.data })
        }
      }
    }
}