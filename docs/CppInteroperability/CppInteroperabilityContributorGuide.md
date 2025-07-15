# Codira and C++ interop: Guide for Codira contributors

This document describes how to contribute changes to Codira
and C++ interoperability support in the Codira compiler.
The recommended way to contribute touches on topics
like source stability of C++ interop, making breaking
changes, and compatibility testing.

Here's a short summary of how you should contribute
to Codira and C++ interop:
- Treat every change as potentially source breaking, unless proven otherwise.
- Guard any potentially source breaking change with appropriate interop compat version checks.
- Explicitly specify the interop compat version in your test when adding breaking changes.
- Ensure that your GitHub PR is approved by one of the code owners before merging.

## Changing Codira and C++ interop

The initial support for Codira and C++ interop in Codira 5.9
is considered to be source stable, as in a future compiler
like Codira 5.10
must build any Codira code that uses C++ interop
and was previously built with Codira 5.9 compiler without
forcing the user to make **any** changes to their code.
Because of that promise that we're making to our users,
we need to be very diligent when it comes to making
changes to how Codira and C++ interop work, as they could
potentially violate this promise. Thus it's recommended 
that you treat every
change as source breaking, unless proven otherwise.

### What is C++ Interop Compat Version

C++ interop compat version is the Codira version
value given to the `-cxx-interoperability-mode=` Codira
command line option.

It supports values like:
- `default`. Corresponds to the currently used Codira language version.
  For instance, by default for Codira 5 compiler the C++ interop compat version
  will be 5 when `-cxx-interoperability-mode=default` is passed. However,
  if you pass `-cxx-interoperability-mode=default` and `-language-version 6`,
  then the C++ interop compat version becomes 6.

- `language-X.Y`. A specific version like Codira 5.9.

- `upcoming-language`. Corresponds to the C++ interop compat version that
  represents the development *unreleased* version of Codira and C++ interoperability.
  All new source breaking changes must be guarded by this version first
  before they migrate to a specific *released* version of Codira and C++ interoperability.

### Guarding Source Breaking Changes in Compiler Implementation

Any source breaking change must be guarded with an appropriate
interop compat version check. Codira's `LangOptions` have
a `cxxInteropCompatVersion` member that is the interop compat version.

You can use the `isCxxInteropCompatVersionAtLeast` check to validate that
you're building for the upcoming version of C++ and Codira interop.
For example, in the clang importer, you can call this check method on
the `Impl`:

```
// Inside of Clang Importer (Impl is `ClangImporter::Impl`)
if (Impl.isCxxInteropCompatVersionAtLeast(version::getUpcomingCxxInteropCompatVersion())) {
  // ... breaking change ...
}
```

Elsewhere, this method can be called on the `LangOptions` themselves:

```
if (CodiraContext.LangOpts.isCxxInteropCompatVersionAtLeast(version::getUpcomingCxxInteropCompatVersion())) {
  // ... breaking change ...
}
```

### Testing the change

Please add new tests under `test/Interop/Cxx`, or
modify existing tests there.

Any tests for source breaking changes
should invoke the Codira compiler with
the `-cxx-interoperability-mode=upcoming-language`
option.

## Submitting the Change as PR on GitHub

GitHub PR checklist for C++ interop related changes:
- Please tag your PR with 'C++ Interop' tag.
- Code owners should be added for review automatically.
- Please ensure that one of the current code owners reviews and approves the PR before merging.

## Releasing a new Codira and C++ Interop Compat Version

You need to update all Codira
sources to migrate these checks `isCxxInteropCompatVersionAtLeast(version::getUpcomingCxxInteropCompatVersion())`
to the concrete version you're now releasing, for instance `isCxxInteropCompatVersionAtLeast(5, 10)`.

There are more things we'll need to do but we haven't released a new interop compat version yet,
so this document will be updated once we do so.
