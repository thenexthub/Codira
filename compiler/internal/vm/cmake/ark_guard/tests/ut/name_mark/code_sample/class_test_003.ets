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
@interface MyAnno1 {}

@MyAnno1()
class ClassA {
    public field1: int = 3;
    private field2: string = 'hello';
}

class ClassB {
    public field1: int = 3;
    private field2: string = 'hello';
}

class ClassC {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
}

class ClassD {
    public final field1: int = 1;
}

class ClassE {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
}

class ClassF {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
}

class ClassG {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
    @MyAnno1()
    private field3: int = 3;
    static field4: int = 3;
}

class ClassH {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
    @MyAnno1()
    private field3: int = 3;
    static field4: int = 3;
}

class ClassI {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
    @MyAnno1()
    private field3: int = 3;
    static field4: int = 3;
}

class ClassJ {
    @MyAnno1()
    public field1: int = 3;
    private field2: string = 'hello';
    @MyAnno1()
    private field3: int = 3;
    static field4: int = 3;
}