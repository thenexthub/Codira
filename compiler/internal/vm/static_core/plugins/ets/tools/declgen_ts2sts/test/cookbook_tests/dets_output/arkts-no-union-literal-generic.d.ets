'use static'
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
declare class SomeType1 {
}
declare class Base<T = number> {
}
declare class Base1<T = boolean> {
}
declare class Point1<T extends boolean> {
}
declare class Point2<T extends number | 'a'> {
}
declare class Point3<T extends boolean | 'a'> {
}
declare class Point4<T extends boolean | 'a'> {
}
declare class Point5<T extends boolean | boolean, D extends SomeType1> {
}
declare class Point6<T extends boolean | boolean, D = number> {
}
