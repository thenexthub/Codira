# ArkTest framework
ArkTest is unit testing framework for ArkTS code.
_NOTE(ipetrov): add more documentation_

### Example of usage
```ts
function main(): int {
    // Create a testsuite instance
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    // Add a test to the testsuite
    myTessuite.addTest("TestWithEqualityAndNonEquality", () => {
        let one = 1;
        arktest.assertEQ(one, 1);
        arktest.assertNE(one, 2);
        arktest.assertLT(one, 3);
        arktest.assertLE(one, 1, "1 should be <= 1");
        arktest.assertLE(one, 4);
    });
    // Add one more test to the testsuite
    myTessuite.addTest("TestWithExceptions", () => {
        arktest.expectError(() => { throw new Error() }, new Error());
        let expectedException = new Exception("Expected message");
        arktest.expectException(() => { throw new Exception("Expected message") }, expectedException);
        // Expect any exception
        arktest.expectException(() => { throw new Exception("Expected message") });
        arktest.expectNoThrow(() => {});
    });
    // Run all tests for the testsuite
    // Must have `return` that returns amount of failed tests
    return myTestsuite.run();
}
```


### Example of forbidden usage example 1
```ts
function main() {
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    myTessuite.addTest("TestWithEquality", () => {
        let one = 1;
        arktest.assertEQ(one, 1);
    });
    myTestsuite.run(); // Forbidden not use `return`
}
```


### Example of forbidden usage example 2
```ts
function main() {
    let myTestsuite = new arktest.ArkTestsuite("myTestsuite");
    myTessuite.addTest("TestWithEquality1", () => {
        let one = 1;
        arktest.assertEQ(one, 1);
    });
    myTessuite.addTest("TestWithEquality2", () => {
        let two = 2;
        arktest.assertEQ(two, 1); // Here is bug
    });
    myTestsuite.run();
    return true; // Forbidden always return constant value like `true`, 0 or any others constant value
}

TEST_F(...) {
    ASSERT_EQ(true, main());  // Test will always pass, but test has bug
}
```
