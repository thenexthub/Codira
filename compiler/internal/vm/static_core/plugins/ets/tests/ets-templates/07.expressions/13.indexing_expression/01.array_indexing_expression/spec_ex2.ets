/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    It is essential that, if type T is a reference type, then the fields of array elements
    can be changed by changing the resultant variable fields.
    An illustration is given in the example below:
---*/

class RefType {
    field: number = 666;
}

function main() {
    const objects: RefType[] = [new RefType(), new RefType()];
    const obj = objects[1];
    obj.field = 777;            // change the field in the array element
    arktest.assertTrue( objects[0].field == 666 );
    arktest.assertTrue( objects[1].field == 777 );
}
