This directory tests fixed compiler crashers related to differentiable
programming in Codira.

`lit.local.cfg` checks for `asserts` as a `lit` available feature, so there is
no need to add `REQUIRES: asserts` to individual test files.
