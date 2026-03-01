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
 desc: 04.05.02 @ObjectLink variable and its source variable of parent component both refer to the same object
 name: 04.managing_state_owned_by_a_component/04.05_object_reference_with_@ObjectLink/04.05.02_@ObjectLink_variable_and_its_source_variable_of_parent_component_both_refer_to_the_same_object/annotation_@Observed
 tags: [compile-only]
 ---*/

@Observed
class ClassA {
    public a: number;
    constructor(a?: number) {
        this.a = a ? a : 0;
    }
}

@Component
struct Child {
    @Link link: ClassA;
    @Prop prop: ClassA;
    @ObjectLink objLink: ClassA;

    build() {
        Column({ space: 10 }) {
            Row({ space: 10 }) {
                Text(`link: ${JSON.stringify(this.link)}`)
                Button(`a++`)
                .onClick(() => { this.link.a++; })
                Button(`assign`)
                .onClick(() => { this.link = new ClassA(10*this.link.a); })
            }
            Row({ space: 10 }) {
                Text(`prop: ${JSON.stringify(this.prop)}`)
                Button(`a++`)
                .onClick(() => { this.prop.a++; })
                Button(`assign`)
                .onClick(() => { this.prop = new ClassA(10*this.prop.a); })
            }
            Row({ space: 10 }) {
                Text(`objLink: ${JSON.stringify(this.objLink)}`)
                Button(`a++`)
                .onClick(() => { this.objLink.a++; })
                Text(`assign not possible`)
            }
        }
        .border({ width: 3, color: Color.Red })
    }
}

@Entry
@Component
struct Parent {
    @State parent: ClassA = new ClassA(123);

    build() {
        Column({ space: 20 }) {
            Row({ space: 10 }) {
                Text(`parent: ${JSON.stringify(this.parent)}`)
                Button(`a++`)
                .onClick(() => { this.parent.a++; })
                Button(`assign`)
                .onClick(() => { this.parent = new ClassA(10*this.parent.a); })
            }
            Child({ link: this.$parent, prop: this.parent, objLink: this.parent })
        }
        .width('100%')
    }
}