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

function test_dropframe() {
    function test_nested() {
        a = 2;
        var d = 0;
        function nested_inner1() {
            b = 3;
            d = 1;
            var e = 0;
            function nested_inner2() {
                c = 4;
                d = 2;
                e = 1;
            }
            nested_inner2();
        }
        nested_inner1();
    }

    function test_parallel() {
        a = 6;
        b = 4;
        c = 5;
        var d = 1;
        function parallel_inner1() {
            a = 4;
            b = 5;
            c = 6;
            d = d * 2;
        }
        function parallel_inner2() {
            d = d + 1;
            parallel_inner1();
        }
        parallel_inner2();
    }

    function test_multicall() {
        var d = 1;
        function multicall_inner() {
            a = a * 2;
            b = b + a;
            c = c * b;
            d = d * c + b * a;
        }
        multicall_inner();
        multicall_inner();
    }

    function test_forloop1() {
        var d = 1;
        function forloop1_inner() {
            a = a * 2;
            b = b + a;
            c = c + b * a;
            d = d * (b - a);
        }
        for (let i = 1; i <= 2; i++) {
            for (let j = 3; j <= 4; j++) {
                forloop1_inner();
            }
        }
    }

    function test_forloop2() {
        var d = 1;
        for (let i = 1; i <= 2; i++) {
            for (let j = 3; j <= 4; j++) {
                d = d + i * j;
                function forloop2_inner() {
                    a = a * i;
                    b = b + a * j;
                    c = c + a + b;
                    d = d * i * j + c;
                }
                forloop2_inner();
            }
        }
    }

    function test_assign() {
        var func = undefined
        function func1() {
            function func2() {
                var s = 2
                func = function func3() {
                    s++
                }
                func()
            }
            func2()
            func()
        }
        func1();
    }

    function test_recursion() {
        var x = 6;
        x = x + 1;
        a = a + 1;
        if (a < 10) {
            test_recursion();
        }
        x = x + 1;
        a = a + 1;
    }

    var a = 1;
    var b = 2;
    var c = 3;
    test_nested();
    test_parallel();
    test_multicall();
    test_forloop1();
    test_forloop2();
    test_assign();
    a = 1;
    test_recursion();

      function test_args() {
        var d = 1;
        function args_inner(...args) {
          var numArgs = args.length;
          for (var i = 0; i < numArgs; i++) {
            a += args[i];
            b += args[i];
            c += args[i];
            d += args[i];
          }
          d += 1;
        }
        args_inner();
        args_inner(1);
        args_inner(1, 2);
        args_inner(1, 2, 3);
        args_inner(1, 2, 3, 4);
        args_inner(1, 2, 3, 4, 5);
        args_inner(1, 2, 3, 4, 5, 6);
      }

      function test_newobj() {
        var d = 1;
        class TestClass {
          constructor() {
            var e = 1;
            function foo1() {
              e += a - b + c - d + e;
              a += 1;
              b += 2;
              c += 3;
              d += 4;
            }
            foo1();
          }
          foo2() {
            a += d;
            b += d;
            c += d;
            d += 1;
          }

          foo3() {
            var e = 1;
            function foo3_inner() {
              e += a - b + c - d + e;
              a += 1;
              b += 2;
              c += 3;
              d += 4;
            }
            foo3_inner();
          }
        }

        var obj = new TestClass();
        obj.foo2();
        obj.foo2();
        obj.foo3();
        obj.foo3();
      }

      function test_newobjsuper() {
        var d = 10;

        class TestClassSuper {
          constructor() {
            var e = 1;
            function fooSuper() {
              e = e + d;
              d = d + a;
              a = a + b + c;
              b = b + d;
              c = c + e;
            }
            fooSuper();
          }
        }

        class TestClass2 extends TestClassSuper {
          constructor() {
            super();
            a = a + 1;
            b = b + 1;
            c = c + 1;
          }

          foo() {
            a = a + 1;
            b = b + 2;
            c = c + 3;
            d = d + a;
          }
        }

        var obj2 = new TestClass2();
        obj2.foo();
        obj2.foo();
      }
    
      b = 2;
      c = 3;
      test_args();
      test_newobj();
      a = 1;
      b = 2;
      c = 3;
      test_newobjsuper();
    a = a + c;
    b = b + a;
}

test_dropframe();