## Tests annotations

Test annotations are needed to collect statistics during project development, so it is important to write them correctly \
The test annotation is cpp comment, consisted of tags and values for test description in format \
! // Test: {tag1}={value1}, {tag2}={value2}, ...

### test-kind tag

    The kind of test:
        * api
        * mock
        * internal

### api tag

    api tag reflects the implemented api name which checks ypur test
    api tag must be in format *api_group*::*api_name*

    examples:
        api=GraphApiImpl::bbCreateEmpty - for bbCreateEmpty api from GraphApiImpl struct \
        api=DynamicIsa::CreateLoadString - for CreateLoadString method of DynamicIsa class

    api name should be correct (without typos and *api_name* is api from *api_group*)
    api tag is necessary if test-kind=api or test-kind=mock

### abc-kind tag

    The type of abc:
        * ArkTS1
        * ArkTS2
        * JS
        * TS
        * NoABC

### category tag

    the category of test:
        * positive
        * negative

### extension tag:

    Tested extension. Add this tag only to extension's tests, but not to c api tests
    * cpp

There are examples of correct annotations:
// Test: test-kind=api, api=GraphApiImpl::bbCreateEmpty, abc-kind=ArkTS1, category=positive
// Test: test-kind=api, api=File::GetModules, abc-kind=ArkTS1, category=positive, extension=cpp
