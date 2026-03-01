/* Copyright (c) 2025 Huawei Device Co., Ltd.
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
class TestChar {
    static char_value: char = c'c';
    static string_value: String = "c";
    static special1: string = "";
    static special2: null = null;
    static special3: undefined  = undefined ;
    static special4: char = c'\n';
    static special5: char = c'\r';
    static special6: char = c'\t';
    static special7: char = c'\b';
    static special8: char = c'\0';
    static charMin: char = Int.toChar(0);
    static charMax: char = Int.toChar(65535);
}

class TestCharA extends TestChar {
    static char_value: char = c'c';
}

final class TestCharFinal extends TestChar {
    static char_value: char = c'c';
}
