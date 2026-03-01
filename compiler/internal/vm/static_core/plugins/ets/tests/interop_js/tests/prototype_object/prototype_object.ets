/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

package prototype_object.test;

class User_class {
    just_value: number = 0;
}

class Base_class {
    value: number = 1;
    
    bar(x: String): string {
        return "bar";
    }
}

class Super_class extends Base_class {
    super_value: number = 2;
    
    foo(): string {
        return "foo";
    }
}

function get_user_class(): User_class {
    const user_obj: User_class = new User_class();
    return user_obj;
}

function get_base_class(): Base_class {
    const base_obj: Base_class = new Base_class();
    return base_obj;
}

function get_super_class(): Super_class {
    const super_obj: Super_class = new Super_class();
    return super_obj;
}