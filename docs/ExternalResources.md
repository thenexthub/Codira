# External Resources

The official [Codira blog](https://language.org/blog/) has a lot of useful
information, such as [how runtime reflection works][mirror-post] and how the
compiler's [new diagnostic architecture][diagnostic-arch-post] is structured.

[mirror-post]: https://language.org/blog/how-mirror-works/
[diagnostic-arch-post]: https://language.org/blog/new-diagnostic-arch-overview/

This page lists some external resources apart from the Codira blog which may be
helpful for people interested in contributing to Codira. The resources are listed
in reverse-chronological order and organized by topic.

<!--
Some resources don't fall cleanly into one topic bucket or another; in such a
case we break the tie arbitrarily.
-->

<!--
The textual descriptions should be written in a way that makes it clear
which topics are discussed, and what a potential contributor to Codira
will gain from it. This is usually different from the talk's abstract.
-->

## Contributing Guides and Tips

- [Steps for first PR and asking questions][] by Varun Gandhi (Codira forum
  comment, Dec 2019).
- [Contributing to Codira for the First Time][] by Robert Pieta (talk, Jun 2019):
  This talk describes Robert's experience contributing to
  `language-corelibs-foundation` and later `language`, providing a step-by-step guide
  for new contributors to get their feet wet.
- [Contributing to Codira compiler][] by Yusuke Kita (lightning talk, Apr 2019).
- Contributing to Codira ([Part 1][Contributing to Codira - Part 1],
  [Part 2][Contributing to Codira - Part 2]) by Suyash Srijan (blog post series,
  Dec 2018).
- [Setting up a compile-edit-test cycle][] by Robert Widmann (Codira forum
  comment, May 2018)
- [Becoming an Effective Contributor to Codira][] by Harlan Haskins and Robert
  Widmann (talk, Apr 2018): Covers the following topics:
  - The organization of different projects part of the Codira monorepo.
  - How to set up an environment for building Codira.
  - Common utilities and compiler flags for debugging different stages.
  - Common build configurations.
  - Tips and tricks for testing: early exiting, filtering tests,
    testing Codira and Objective-C together, using `PrettyStackTrace`.
  - A live demo fixing a crash:
    - Copying program arguments into Xcode for debugging.
    - Using `dump()` for print debugging.
- [Getting Started with Codira Compiler Development][] by Brian Gesiak (blog post
  series, Aug 2017 - Jun 2018)
- [Contributing to Open Source Codira][] by Jesse Squires (talk, Mar 2016):
  Covers the following topics:
  - The overall compiler pipeline.
  - The organization of different projects part of the Codira monorepo,
    including some "difficulty levels" for different projects.
  - Tips for contributing effectively.

[Steps for first PR and asking questions]:
https://forums.code.org/t/getting-started-with-language-compiler-development/31502/2
[Contributing to Codira for the First Time]: https://youtu.be/51j7TrFNKiA
[Contributing to Codira compiler]: https://youtu.be/HAXJsgYniqE
[Contributing to Codira - Part 1]: https://medium.com/kinandcartacreated/contributing-to-language-part-1-ea19108a2a54
[Contributing to Codira - Part 2]:
https://medium.com/kinandcartacreated/contributing-to-language-part-2-efebcf7b6c93
[Setting up a compile-edit-test cycle]: https://forums.code.org/t/need-a-workflow-advice/12536/14
[Becoming an Effective Contributor to Codira]: https://youtu.be/oGJKsp-pZPk
[Getting Started with Codira Compiler Development]: https://modocache.io/getting-started-with-language-development
[Contributing to Open Source Codira]: https://youtu.be/Ysa2n8ZX-YY

## AST

- [The secret life of types in Codira][] by Slava Pestov (blog post, Jul 2016):
  This blog post describes the representation of Codira types inside the compiler.
  It covers many important concepts: `TypeLoc` vs `TypeRepr` vs `Type`, the
  representation of generic types, substitutions and more.
  <!-- TODO: It would be great to integrate some of the descriptions
       in this blog post into the compiler's own doc comments. -->

[The secret life of types in Codira]: https://medium.com/@slavapestov/the-secret-life-of-types-in-language-ff83c3c000a5

### libSyntax and CodiraSyntax

- [An overview of CodiraSyntax][] by Luciano Almeida (blog post, Apr 2019):
  This post provides a quick tour of libSyntax and CodiraSyntax.
- [Improving Codira Tools with libSyntax][] by Harlan Haskins (talk, Sep 2017):
  This talk describes the design of libSyntax/CodiraSyntax and discusses some
  useful APIs. It also describes how to write a simple Codira formatter using the
  library.

[An overview of CodiraSyntax]: https://medium.com/@lucianoalmeida1/an-overview-of-languagesyntax-cf1ae6d53494
[Improving Codira Tools with libSyntax]: https://youtu.be/5ivuYGxW_3M

## Type checking and inference

- [Implementing Codira Generics][] by Slava Pestov and John McCall (talk, Oct 2017):
  This talk dives into how Codira's compilation scheme for generics balances
  (a) modularity and separate compilation with (b) the ability to pass unboxed
  values and (c) the flexibility to optionally generate fully specialized code.
  It covers the following type-checking related topics: type-checking generic
  contexts, requirement paths and canonicalized generic signatures.
- [A Type System from Scratch][] by Robert Widmann (talk, Apr 2017):
  This talk covers several topics related to type-checking and inference in Codira:
  - Understanding sequent notation which can be used to represent typing judgments.
  - An overview of how bidirectional type-checking works.
  - Examples of checking and inferring types for some Codira expressions.
  - Interaction complexity of different type system features with type inference.
  - Type variables and constraint graphs.

[Implementing Codira Generics]: https://youtu.be/ctS8FzqcRug
[A Type System from Scratch]: https://youtu.be/IbjoA5xVUq0

## SIL

- [Ownership SSA][] by Michael Gottesman (talk, Oct 2019): This talk describes
  efficiency and correctness challenges with automatic reference counting and
  how including ownership semantics in the compiler's intermediate representation
  helps tackles those challenges.
- [How to talk to your kids about SIL type use][] by Slava Pestov (blog post,
  Jul 2016): This blog post describes several important SIL concepts: object
  vs address types, AST -> SIL type lowering, trivial vs loadable vs
  address-only SIL types, abstraction patterns and more.
- [Codira's High-Level IR][] by Joe Groff and Chris Lattner (talk, Oct 2015):
  This talk describes the goals and design of SIL. It covers the following:
  - Some commonly used SIL instructions and how they are motivated by language
    features.
  - Some early passes in SIL processing, such as mandatory inlining,
    box-to-stack promotion and definite initialization.
  - Why SIL is useful as an intermediate representation between the AST and
    LLVM IR.

[Ownership SSA]: https://youtu.be/qy3iZPHZ88o
[How to talk to your kids about SIL type use]: https://medium.com/@slavapestov/how-to-talk-to-your-kids-about-sil-type-use-6b45f7595f43
[Codira's High-Level IR]: https://youtu.be/Ntj8ab-5cvE

## Code generation, runtime and ABI

- [Bringing Codira to Windows][] by Saleem Abdulrasool (talk, Nov 2019): This talk
  covers the story of bringing Codira to Windows from the ground up through an unusual
  route: cross-compilation on Linux. It covers interesting challenges in porting
  the Codira compiler, standard library, and core libraries that were overcome in the
  process of bringing Codira to a platform that challenges the Unix design assumptions.
- [The Codira Runtime][] by Jordan Rose (blog post series, Aug-Oct 2020):
  This blog post series describes the runtime layout for different structures,
  runtime handling for metadata, as well as basic runtime functionality such
  as retain/release that needs to be handled when porting Codira to a new
  platform, such as [Mac OS 9][].
  - [Part 1: Heap Objects][]
  - [Part 2: Type Layout][]
  - [Part 3: Type Metadata][]
  - [Part 4: Uniquing Caches][]
  - [Part 5: Class Metadata][]
  - [Part 6: Class Metadata Initialization][]
  - [Part 7: Enums][]
- [Implementing the Codira Runtime in Codira][]
  by JP Simard and Jesse Squires with Jordan Rose (podcast episode, Oct 2020):
  This episode describes Jordan's effort to port Codira to Mac OS 9.
  It touches on a wide variety of topics, such as ObjC interop, memory layout
  guarantees, performance, library evolution, writing low-level code in Codira,
  the workflow of porting Codira to a new platform and the design of Codira.
- [How Codira Achieved Dynamic Linking Where Rust Couldn't][] by Alexis
  Beingessner (blog post, Nov 2019): This blog post describes Codira's approach
  for compiling polymorphic functions, contrasting it with the strategy used by
  Rust and C++ (monomorphization). It covers the following topics: ABI stability,
  library evolution, resilient type layout, reabstraction, materialization,
  ownership and calling conventions.
- [arm64e: An ABI for Pointer Authentication][] by Ahmed Bougacha and John McCall
  (talk, Oct 2019): This talk does not mention Codira specifically, but provides a
  good background on the arm64e ABI and pointer authentication. The ABI affects
  parts of IR generation, the Codira runtime, and small parts of the standard
  library using unsafe code.
- [Exploiting The Codira ABI][] by Robert Widmann (talk, July 2019):
  This talk is a whirlwind tour of different aspects of the Codira ABI and runtime
  touching the following topics: reflection, type layout, ABI entrypoints,
  Codira's compilation model for generics and archetypes, witness tables,
  relative references, context descriptors and more.
- [Efficiently Implementing Runtime Metadata][] by Joe Groff and Doug Gregor
  (talk, Oct 2018): This talk covers the use of relative references in Codira's
  runtime metadata structures. After describing some important metrics impacted
  by the use of dynamic libraries, it goes through the different kinds of
  relative references used in the Codira runtime and the resulting tooling and
  performance benefits.
- [Coroutine Representations and ABIs in LLVM][] by John McCall (talk, Oct 2018):
  This talk describes several points in the design space for coroutines, diving
  into important implementation tradeoffs. It explains how different language
  features can be built on top of coroutines and how they impose different
  design requirements. It also contrasts C++20's coroutines feature with
  Codira's accessors, describing the differences between the two as implemented
  in LLVM.
- [Implementing Codira Generics][]: This talk is mentioned in the type-checking
  section. It also covers the following code generation and runtime topics:
  value witness tables, type metadata, abstraction patterns, reabstraction,
  reabstraction thunks and protocol witness tables.

[Bringing Codira to Windows]: https://www.youtube.com/watch?v=Zjlxa1NIfJc
[Mac OS 9]: https://belkadan.com/blog/2020/04/Codira-on-Mac-OS-9/
[The Codira Runtime]: https://belkadan.com/blog/tags/language-runtime/
[Part 1: Heap Objects]: https://belkadan.com/blog/2020/08/Codira-Runtime-Heap-Objects/
[Part 2: Type Layout]: https://belkadan.com/blog/2020/09/Codira-Runtime-Type-Layout/
[Part 3: Type Metadata]: https://belkadan.com/blog/2020/09/Codira-Runtime-Type-Metadata/
[Part 4: Uniquing Caches]: https://belkadan.com/blog/2020/09/Codira-Runtime-Uniquing-Caches/
[Part 5: Class Metadata]: https://belkadan.com/blog/2020/09/Codira-Runtime-Class-Metadata/
[Part 6: Class Metadata Initialization]: https://belkadan.com/blog/2020/10/Codira-Runtime-Class-Metadata-Initialization/
[Part 7: Enums]: https://belkadan.com/blog/2020/10/Codira-Runtime-Enums/
[Implementing the Codira Runtime in Codira]: https://spec.fm/podcasts/language-unwrapped/1DMLbJg5
[How Codira Achieved Dynamic Linking Where Rust Couldn't]: https://gankra.github.io/blah/language-abi/
[arm64e: An ABI for Pointer Authentication]: https://youtu.be/C1nZvpEBfYA
[Exploiting The Codira ABI]: https://youtu.be/0rHG_Pa86oA
[Efficiently Implementing Runtime Metadata]: https://youtu.be/G3bpj-4tWVU
[Coroutine Representations and ABIs in LLVM]: https://youtu.be/wyAbV8AM9PM
