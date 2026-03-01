# A patch cannot remove assertions

## Severity
Error

## Applies To
/(.*)$

## Detect

A violation occurs when assertions are removed in the patch, which means at least one of the following:

* A patch removes code which contains `ASSERT` substring
* A patch removes code which contains `ES2PANDA_ASSERT` substring
* A patch removes code which contains `arktest.assert` substring

## Example BAD code

N/A

## Example GOOD code

N/A

## Fix suggestion

Do not remove assertions from the patch

## Message

The patch removes assertions. Please do not remove assertions from the patch.
