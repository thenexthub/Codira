/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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

{% for type in types %}

/*---
desc: >-
    Execution of some primitive type operations can throw OutOfMemoryError
    if allocation of new object is required but the available memory is not
    sufficient to perform it.
ark_options: ['--heap-size-limit=16777216']
timeout: 1800
---*/

let array: Array<{{type}}> = new Array<{{type}}>()

let count: {{type}} = 0

function append () {
    let n = count++
    array.push(n) // just store the variable in dynamic array
}

function main() {
    try {
        for (;;) { // infinite loop
            append()
        }
    }
    catch (error: Error) {
        if (error instanceof OutOfMemoryError) // OK
            return

        throw error // Unknown error
    }
}

{% endfor %}
