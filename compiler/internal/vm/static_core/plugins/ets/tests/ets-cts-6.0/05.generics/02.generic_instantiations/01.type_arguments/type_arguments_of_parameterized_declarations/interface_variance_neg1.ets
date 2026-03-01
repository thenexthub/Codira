/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
/*---
desc: >-
    It is a compile-time error if a covariant wildcard is specified for a contravariant type parameter.
    It is a compile-time error if a contravariant wildcard is specified for a covariant type parameter.
params: interface arguments
tags: [compile-only, negative]
---*/

interface I<{{c.variance}}T> {}

class A<{{c.variance}}T> implements I<T> {
    private t: T

    public constructor(t: T) {
        this.t = t
    }
}

function main() {

    {{c.use|indent}}
    
}
{% endfor %}
