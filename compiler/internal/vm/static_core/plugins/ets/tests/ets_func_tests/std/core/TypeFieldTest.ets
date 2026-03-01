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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

interface Serializable {
    serialize(): String;
}

class Point implements Serializable {
    protected x: double
    y: double

    override serialize(): String {
        return "{" + this.x + "," + this.y + "}"
    }

    add(oth: Point): Point {
        let res = new Point()
        res.x = this.x + oth.x
        res.y = this.y + oth.y
        return res
    }
}

class Point3D extends Point {
    z: double
    y: double = 3.0
    private state: int
    static ZERO: double = 0.0
    readonly r: int = 1
}

function testField(): int {
    let pt = Type.of(new Point3D()) as ClassType

    let xf = pt.getFieldByName("x")
    let yf = pt.getFieldByName("y")
    let sf = pt.getFieldByName("state")
    let zf = pt.getFieldByName("ZERO")
    let rf = pt.getFieldByName("r")

    let isInherited = (f: Field, cls: ClassType): boolean => {
        return !f.getOwnerType().equals(cls)
    };


    return test(xf.getName() == "x",                                        "FieldName: x") +
            test(xf.getOwnerType().equals(Type.of(new Point())),            "   OwnerType: Point") +
            test(xf.getType() == DoubleType.VAL,                            "   Type: double") +
            test(xf.getAttributes() == 0,                                   "   no attributes") +
            test(xf.getAccessModifier() == AccessModifier.PROTECTED,        "   protected access modififer") +
            test(!xf.isReadonly(),                                          "   not readonly") +
            test(isInherited(xf, pt),                                       "   inherited") +
            test(!xf.isStatic(),                                            "   not static") +
            test(yf.getName() == "y",                                       "FieldName: x") +
            test(yf.getOwnerType().equals(Type.of(new Point())),            "   OwnerType: Point") +
            test(yf.getType() == DoubleType.VAL,                            "   Type: double") +
            test(yf.getAttributes() == 0,                                   "   no attributes") +
            test(yf.getAccessModifier() == AccessModifier.PUBLIC,           "   public access modififer") +
            test(!yf.isReadonly(),                                          "   not readonly") +
            test(isInherited(yf, pt),                                       "   inherited") +
            test(!yf.isStatic(),                                            "   not static") +
            test(sf.getName() == "state",                                   "FieldName: state") +
            test(sf.getOwnerType().equals(pt),                              "   OwnerType: Point3D") +
            test(sf.getType() == IntType.VAL,                               "   Type: int") +
            test(sf.getAttributes() == 0,                                   "   no attributes") +
            test(sf.getAccessModifier() == AccessModifier.PRIVATE,          "   private access modifier") +
            test(!sf.isReadonly(),                                          "   not readonly") +
            test(!isInherited(sf, pt),                                      "   not inherited") +
            test(!sf.isStatic(),                                            "   not static") +
            test(zf.getName() == "ZERO",                                    "FieldName: ZERO") +
            test(zf.getOwnerType().equals(pt),                              "   OwnerType: Point3D") +
            test(zf.getType() == DoubleType.VAL,                            "   Type: double") +
            test(zf.getAttributes() == Attributes.STATIC,                   "   static attribute") +
            test(zf.getAccessModifier() == AccessModifier.PUBLIC,           "   public access modififer") +
            test((zf.getStaticValue() as Double) == Point3D.ZERO, "   static value") +
            test(!zf.isReadonly(),                                          "   not readonly") +
            test(!isInherited(zf, pt),                                      "   not inherited") +
            test(zf.isStatic(),                                             "   static") +
            test(rf.getName() == "r",                                       "FieldName: r") +
            test(rf.getOwnerType().equals(pt),                              "   OwnerType: Point3D") +
            test(rf.getType() == IntType.VAL,                               "   Type: int") +
            test(rf.getAttributes() == Attributes.READONLY,                 "   readonly attribute") +
            test(rf.getAccessModifier() == AccessModifier.PUBLIC,           "   public access modififer") +
            test(rf.isReadonly(),                                           "   readonly") +
            test(!isInherited(rf, pt),                                      "   not inherited") +
            test(!rf.isStatic(),                                            "   not static")
}

function main(): int {
    let failures = 0
    failures += testField()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
