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

class Operations {
    static sum(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }
};

namespace na {
class A {
    static funcA(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }
    static funcA(a0: int, a1: int): int {
        return a1 - a0 as int;
    }
};
}

namespace nb {
namespace nc {
class A {
    static funcA(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }
};
}
}

namespace nd {
class A {
    static funcA(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }
};
class B extends A{
    static funcA(a0: short, a1: short): short {
        return (a1 - a0).toShort();
    }
};
}

class A {
    static funcA(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }

    static funcA(a0: int, a1: int): int {
        return a1 - a0 as int;
    }

    static funcB(a0: short, a1: short): short {
        return A.funcA(a0, a1);
    }
};
class B extends A{
    static funcB(a0: short, a1: short): short {
        return (a1 - a0).toShort();
    }
};
class C extends A{
};
class D extends A{
    static funcA(a0: short, a1: short): short {
        return (a1 - a0).toShort();
    }
};
class E extends A{
    static funcA(a0: short, a1: short): short {
        return A.funcA(a0, a1);
    }
};
class F {
    static increment(a0: short, a1: short): void {
        F.count += a0;
        F.count += a1;
    }

    static getCount(): short {
        return F.count;
    }

    static count: short = 0;
}
class G {
    public static publicMethod(a0: short, a1: short): short {
        return (a0 + a1).toShort();
    }

    private static privateMethod(a0: short, a1: short): short {
        return (a1 - a0).toShort();
    }

    static callPrivateMethod(a0: short, a1: short): short {
        return G.privateMethod(a0, a1);
    }
}

class CheckWrongSignature {
    static method(a: String): short {
        return 1;
    }
}
