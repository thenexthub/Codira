# A patch cannot be submitted without tests

## Severity
Error

## Applies To
/*.(ets|sts|cpp|h|ts)$

## Detect

A violation occurs when a patch is submitted without tests.

"Submitted without tests" means either of the following:

* Commit title or commit message mentions "bug", "fix", "bug fix", but there are no added or edited files which contain "tests" in their paths.
* Commit title or commit message mentions "feature", but there are no added files which contain "tests" in their paths.
* Patch contributes to other parts of the project, but does not contribute to the tests

## Example BAD code

N/A

## Example GOOD code

N/A

## Fix suggestion

Add tests to the patch.

## Message

The patch is submitted without tests. Please submit tests to the patch.
