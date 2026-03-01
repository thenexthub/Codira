# Added AST nodes must match language grammar

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when a patch introduces a new AST node which does not match the spec, which means following:

* A patch contains additions to the file `ir/astNodeMapping.h`
* Added lines contain node names which are not found in the "Grammar Summary" chapter of the ArkTS language specification.

## Example BAD code

N/A

## Example GOOD code

N/A

## Fix suggestion

Add tests to the patch.

## Message

The patch is submitted without tests. Please submit tests to the patch.
