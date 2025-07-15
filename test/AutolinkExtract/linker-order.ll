; RUN: llc -mtriple armv7--linux-gnueabihf -filetype obj -o - %s | %target-language-autolink-extract -o - - | %FileCheck %s

; REQUIRES: autolink-extract
; REQUIRES: CODEGENERATOR=ARM

; Ensure that the options in the object file preserve ordering.  The linker
; options are order dependent, and we would accidentally reorder them because we
; used a std::set rather than an toolchain::SmallSetVector.

@_language1_autolink_entries_1 = private constant [6 x i8] c"First\00", section ".code1_autolink_entries", align 8
@_language1_autolink_entries_0 = private constant [7 x i8] c"Second\00", section ".code1_autolink_entries", align 8

@_language1_autolink_entries_2 = private constant [7 x i8] c"-rpath\00", section ".code1_autolink_entries", align 8
@_language1_autolink_entries_3 = private constant [7 x i8] c"Second\00", section ".code1_autolink_entries", align 8
@_language1_autolink_entries_4 = private constant [7 x i8] c"-rpath\00", section ".code1_autolink_entries", align 8
@_language1_autolink_entries_5 = private constant [6 x i8] c"First\00", section ".code1_autolink_entries", align 8

; CHECK: First
; CHECK: Second

; CHECK: -rpath
; CHECK: Second

; CHECK: -rpath
; CHECK: First

