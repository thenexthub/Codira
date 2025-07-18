#ifndef TEST_INTEROP_CXX_CLASS_INPUTS_DESTRUCTORS_H
#define TEST_INTEROP_CXX_CLASS_INPUTS_DESTRUCTORS_H

struct
    __attribute__((language_attr("conforms_to:CodiraTest.Testable")))
    HasTest {
  void test() const;
};

struct
    __attribute__((language_attr("conforms_to:CodiraTest.Playable")))
    __attribute__((language_attr("import_reference")))
    __attribute__((language_attr("retain:immortal")))
    __attribute__((language_attr("release:immortal")))
    HasPlay {
  void play() const;
};

struct __attribute__((language_attr("conforms_to:CodiraTest.Testable")))
__attribute__((language_attr(
    "conforms_to:CodiraTest.Playable"))) MultipleConformanceHasTestAndPlay {
  void test() const;
  void play() const;
};

struct
    __attribute__((language_attr("conforms_to:ImportedModule.ProtocolFromImportedModule")))
    HasImportedConf {
  void testImported() const;
};

struct DerivedFromHasTest : HasTest {};
struct DerivedFromDerivedFromHasTest : HasTest {};
struct DerivedFromMultipleConformanceHasTestAndPlay
    : MultipleConformanceHasTestAndPlay {};

struct __attribute__((language_attr("conforms_to:CodiraTest.Testable")))
DerivedFromDerivedFromHasTestWithDuplicateArg : HasTest {};

struct DerivedFromHasPlay : HasPlay {};
struct DerivedFromDerivedFromHasPlay : HasPlay {};

struct HasTestAndPlay : HasPlay, HasTest {};
struct DerivedFromHasTestAndPlay : HasPlay, HasTest {};

struct DerivedFromHasImportedConf : HasImportedConf {};
struct DerivedFromDerivedFromHasImportedConf : HasImportedConf {};

#endif // TEST_INTEROP_CXX_CLASS_INPUTS_DESTRUCTORS_H
