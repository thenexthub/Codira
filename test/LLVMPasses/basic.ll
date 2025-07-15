; RUN: %language-toolchain-opt -passes=language-toolchain-arc-optimize %s | %FileCheck %s

; Use this testfile to check if the `language-frontend -language-dependency-tool` option works.
; RUN: %language_frontend_plain -language-toolchain-opt -passes=language-toolchain-arc-optimize %s | %FileCheck %s

target datalayout = "e-p:64:64:64-S128-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f16:16:16-f32:32:32-f64:64:64-f128:128:128-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-apple-macosx10.9"

%language.refcounted = type { ptr, i64 }
%language.heapmetadata = type { ptr, ptr }
%objc_object = type opaque
%language.bridge = type opaque

declare ptr @language_unknownObjectRetain(ptr returned)
declare void @language_unknownObjectRelease(ptr)
declare ptr @toolchain.objc.retain(ptr)
declare void @toolchain.objc.release(ptr)
declare ptr @language_allocObject(ptr , i64, i64) nounwind
declare void @language_release(ptr nocapture)
declare ptr @language_retain(ptr returned) nounwind
declare ptr @language_bridgeObjectRetain(ptr)
declare void @language_bridgeObjectRelease(ptr)
declare ptr @language_retainUnowned(ptr returned)

declare void @user(ptr) nounwind
declare void @user_objc(ptr) nounwind
declare void @unknown_func()

define private void @__language_fixLifetime(ptr) noinline nounwind {
entry:
  ret void
}

; CHECK-LABEL: @trivial_objc_canonicalization(
; CHECK-NEXT: entry:
; CHECK-NEXT: [[RET1:%.+]] = tail call ptr @toolchain.objc.retain(ptr [[RET0:%.+]])
; CHECK-NEXT: call void @user_objc(ptr [[RET0:%.+]])
; CHECK-NEXT: ret void

define void @trivial_objc_canonicalization(ptr %O) {
entry:
  %0 = tail call ptr @toolchain.objc.retain(ptr %O)
  call void @user_objc(ptr %0) nounwind
  ret void
}

; CHECK-LABEL: @trivial_retain_release(
; CHECK-NEXT: entry:
; CHECK-NEXT: call void @user
; CHECK-NEXT: ret void

define void @trivial_retain_release(ptr %P, ptr %O, ptr %B) {
entry:
  tail call ptr @language_retain(ptr %P)
  tail call void @language_release(ptr %P) nounwind
  tail call ptr @language_unknownObjectRetain(ptr %P)
  tail call void @language_unknownObjectRelease(ptr %P)
  tail call ptr @toolchain.objc.retain(ptr %O)
  tail call void @toolchain.objc.release(ptr %O)
  %v = tail call ptr @language_bridgeObjectRetain(ptr %B)
  tail call void @language_bridgeObjectRelease(ptr %v)
  call void @user(ptr %P) nounwind
  ret void
}

; CHECK-LABEL: @trivial_retain_release_with_rcidentity(
; CHECK-NEXT: entry:
; CHECK-NEXT: call void @user
; CHECK-NEXT: ret void

define void @trivial_retain_release_with_rcidentity(ptr %P, ptr %O, ptr %B) {
entry:
  tail call ptr @language_retain(ptr %P)
  tail call void @language_release(ptr %P) nounwind
  tail call ptr @language_unknownObjectRetain(ptr %P)
  tail call void @language_unknownObjectRelease(ptr %P)
  tail call ptr @toolchain.objc.retain(ptr %O)
  tail call void @toolchain.objc.release(ptr %O)
  call void @user(ptr %P) nounwind
  ret void
}

; retain_motion1 - This shows motion of a retain across operations that can't
; release an object.  Release motion can't zap this.

; CHECK-LABEL: @retain_motion1(
; CHECK-NEXT: store i32
; CHECK-NEXT: ret void


define void @retain_motion1(ptr %A) {
  tail call ptr @language_retain(ptr %A)
  store i32 42, ptr %A
  tail call void @language_release(ptr %A) nounwind
  ret void
}

; rdar://11583269 - Optimize out objc_retain/release(null)

; CHECK-LABEL: @objc_retain_release_null(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret void

define void @objc_retain_release_null() {
entry:
  tail call void @toolchain.objc.release(ptr null) nounwind
  tail call ptr @toolchain.objc.retain(ptr null)
  ret void
}

; CHECK-LABEL: @languageunknown_retain_release_null(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret void

define void @languageunknown_retain_release_null() {
entry:
  tail call void @language_unknownObjectRelease(ptr null)
  tail call ptr @language_unknownObjectRetain(ptr null) nounwind
  ret void
}

; rdar://11583269 - Useless objc_retain/release optimization.

; CHECK-LABEL: @objc_retain_release_opt(
; CHECK-NEXT: store i32 42
; CHECK-NEXT: ret void

define void @objc_retain_release_opt(ptr %P, ptr %IP) {
  tail call ptr @toolchain.objc.retain(ptr %P) nounwind
  store i32 42, ptr %IP
  tail call void @toolchain.objc.release(ptr %P) nounwind
  ret void
}

; CHECK-LABEL: define{{( protected)?}} void @language_fixLifetimeTest
; CHECK: language_retain
; CHECK: language_fixLifetime
; CHECK: language_release
define void @language_fixLifetimeTest(ptr %A) {
  tail call ptr @language_retain(ptr %A)
  call void @user(ptr %A) nounwind
  call void @__language_fixLifetime(ptr %A)
  tail call void @language_release(ptr %A) nounwind
  ret void
}

; CHECK-LABEL: @move_retain_across_unknown_retain
; CHECK-NOT: language_retain
; CHECK: language_unknownObjectRetain
; CHECK-NOT: language_release
; CHECK: ret
define void @move_retain_across_unknown_retain(ptr %A, ptr %B) {
  tail call ptr @language_retain(ptr %A)
  tail call ptr @language_unknownObjectRetain(ptr %B)
  tail call void @language_release(ptr %A) nounwind
  ret void
}

; CHECK-LABEL: @move_retain_across_objc_retain
; CHECK-NOT: language_retain
; CHECK: toolchain.objc.retain
; CHECK-NOT: language_release
; CHECK: ret
define void @move_retain_across_objc_retain(ptr %A, ptr %B) {
  tail call ptr @language_retain(ptr %A)
  tail call ptr @toolchain.objc.retain(ptr %B)
  tail call void @language_release(ptr %A) nounwind
  ret void
}

; CHECK-LABEL: @move_retain_across_load
; CHECK-NOT: language_retain
; CHECK-NOT: language_release
; CHECK: ret
define i32 @move_retain_across_load(ptr %A, ptr %ptr) {
  tail call ptr @language_retain(ptr %A)
  %val = load i32, ptr %ptr
  tail call void @language_release(ptr %A) nounwind
  ret i32 %val
}

; CHECK-LABEL: @move_retain_but_not_release_across_objc_fix_lifetime
; CHECK: call void @__language_fixLifetime
; CHECK-NEXT: tail call ptr @language_retain
; CHECK-NEXT: call void @user
; CHECK-NEXT: call void @__language_fixLifetime
; CHECK-NEXT: call void @language_release
; CHECK-NEXT: ret
define void @move_retain_but_not_release_across_objc_fix_lifetime(ptr %A) {
  tail call ptr @language_retain(ptr %A)
  call void @__language_fixLifetime(ptr %A) nounwind
  call void @user(ptr %A) nounwind
  call void @__language_fixLifetime(ptr %A) nounwind
  tail call void @language_release(ptr %A) nounwind
  ret void
}

; CHECK-LABEL: @optimize_retain_unowned
; CHECK-NEXT: load
; CHECK-NEXT: add
; CHECK-NEXT: load
; CHECK-NEXT: call void @language_checkUnowned
; CHECK-NEXT: ret
define void @optimize_retain_unowned(ptr %A) {
  tail call ptr @language_retainUnowned(ptr %A)

  ; loads from the %A and speculatively executable instructions
  %L1 = load i64, ptr %A, align 8
  %R1 = add i64 %L1, 1
  %L2 = load i64, ptr %A, align 8

  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @dont_optimize_retain_unowned
; CHECK-NEXT: call ptr @language_retainUnowned
; CHECK-NEXT: load
; CHECK-NEXT: load
; CHECK-NEXT: call void @language_release
; CHECK-NEXT: ret
define void @dont_optimize_retain_unowned(ptr %A) {
  tail call ptr @language_retainUnowned(ptr %A)

  %L1 = load ptr, ptr %A, align 8
  ; Use of a potential garbage address from a load of %A.
  %L2 = load i64, ptr %L1, align 8

  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @dont_optimize_retain_unowned2
; CHECK-NEXT: call ptr @language_retainUnowned
; CHECK-NEXT: store
; CHECK-NEXT: call void @language_release
; CHECK-NEXT: ret
define void @dont_optimize_retain_unowned2(ptr %A, ptr %B) {
  tail call ptr @language_retainUnowned(ptr %A)

  ; store to an unknown address
  store i32 42, ptr %B

  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @dont_optimize_retain_unowned3
; CHECK-NEXT: call ptr @language_retainUnowned
; CHECK-NEXT: call void @unknown_func
; CHECK-NEXT: call void @language_release
; CHECK-NEXT: ret
define void @dont_optimize_retain_unowned3(ptr %A) {
  tail call ptr @language_retainUnowned(ptr %A)

  ; call of an unknown function
  call void @unknown_func()

  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @dont_optimize_retain_unowned4
; CHECK-NEXT: call ptr @language_retainUnowned
; CHECK-NEXT: call ptr @language_retain
; CHECK-NEXT: call void @language_release
; CHECK-NEXT: ret
define void @dont_optimize_retain_unowned4(ptr %A, ptr %B) {
  tail call ptr @language_retainUnowned(ptr %A)

  ; retain of an unknown reference (%B could be equal to %A)
  tail call ptr @language_retain(ptr %B)

  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @remove_redundant_check_unowned
; CHECK-NEXT: load
; CHECK-NEXT: call void @language_checkUnowned
; CHECK-NEXT: load
; CHECK-NEXT: store
; CHECK-NEXT: load
; CHECK-NEXT: ret
define void @remove_redundant_check_unowned(ptr %A, ptr %B, ptr %C) {
  tail call ptr @language_retainUnowned(ptr %A)
  %L1 = load i64, ptr %A, align 8
  tail call void @language_release(ptr %A)

  ; Instructions which cannot do a release.
  %L2 = load i64, ptr %C, align 8
  store i64 42, ptr %C

  tail call ptr @language_retainUnowned(ptr %A)
  %L3 = load i64, ptr %A, align 8
  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @dont_remove_redundant_check_unowned
; CHECK-NEXT: load
; CHECK-NEXT: call void @language_checkUnowned
; CHECK-NEXT: call void @unknown_func
; CHECK-NEXT: load
; CHECK-NEXT: call void @language_checkUnowned
; CHECK-NEXT: ret
define void @dont_remove_redundant_check_unowned(ptr %A, ptr %B, ptr %C) {
  tail call ptr @language_retainUnowned(ptr %A)
  %L1 = load i64, ptr %A, align 8
  tail call void @language_release(ptr %A)

  ; Could do a release of %A
  call void @unknown_func()

  tail call ptr @language_retainUnowned(ptr %A)
  %L3 = load i64, ptr %A, align 8
  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @unknown_retain_promotion
; CHECK-NEXT: language_retain
; CHECK-NEXT: language_retain
; CHECK-NEXT: language_retain
; CHECK-NEXT: ret
define void @unknown_retain_promotion(ptr %A) {
  tail call ptr @language_unknownObjectRetain(ptr %A)
  tail call ptr @language_unknownObjectRetain(ptr %A)
  tail call ptr @language_retain(ptr %A)
  ret void
}

; CHECK-LABEL: @unknown_release_promotion
; CHECK-NEXT: language_release
; CHECK-NEXT: language_release
; CHECK-NEXT: language_release
; CHECK-NEXT: ret
define void @unknown_release_promotion(ptr %A) {
  tail call void @language_unknownObjectRelease(ptr %A)
  tail call void @language_unknownObjectRelease(ptr %A)
  tail call void @language_release(ptr %A)
  ret void
}

; CHECK-LABEL: @unknown_retain_nopromotion
; CHECK: bb1
; CHECK-NOT: language_retain
; CHECK: ret
define void @unknown_retain_nopromotion(ptr %A) {
  tail call ptr @language_retain(ptr %A)
  br label %bb1
bb1:
  tail call ptr @language_unknownObjectRetain(ptr %A)
  ret void
}

; CHECK-LABEL: @releasemotion_forwarding
; CHECK-NOT: language_retain
; CHECK-NOT: language_release
; CHECK: call void @user(ptr %P)
; CHECK: ret
define void @releasemotion_forwarding(ptr %P, ptr %O, ptr %B) {
entry:
  %res = tail call ptr @language_retain(ptr %P)
  tail call void @language_release(ptr %res) nounwind
  call void @user(ptr %res) nounwind
  ret void
}

; CHECK-LABEL: @retainmotion_forwarding
; CHECK: store ptr %P, ptr %R, align 4
; CHECK-NOT: language_retain
; CHECK-NOT: language_release
; CHECK: ret
define void @retainmotion_forwarding(ptr %P, ptr %R, ptr %B) {
entry:
  %res = tail call ptr @language_retain(ptr %P)
  store ptr %res, ptr %R, align 4
  call void @language_bridgeObjectRelease(ptr %B)
  tail call void @language_release(ptr %res) nounwind
  ret void
}

; CHECK-LABEL: @unknownreleasemotion_forwarding
; CHECK-NOT: language_unknownObjectRetain
; CHECK-NOT: language_unknownObjectRelease
; CHECK: call void @user(ptr %P)
; CHECK: ret
define void @unknownreleasemotion_forwarding(ptr %P, ptr %O, ptr %B) {
entry:
  %res = tail call ptr @language_unknownObjectRetain(ptr %P)
  tail call void @language_unknownObjectRelease(ptr %res) nounwind
  call void @user(ptr %res) nounwind
  ret void
}

; CHECK-LABEL: @unknownretainmotion_forwarding
; CHECK: store ptr %P, ptr %R, align 4
; CHECK-NOT: language_unknownObjectRetain
; CHECK-NOT: language_unknownObjectRelease
; CHECK: ret
define void @unknownretainmotion_forwarding(ptr %P, ptr %R, ptr %B) {
entry:
  %res = tail call ptr @language_unknownObjectRetain(ptr %P)
  store ptr %res, ptr %R, align 4
  call void @language_bridgeObjectRelease(ptr %B)
  tail call void @language_unknownObjectRelease(ptr %res) nounwind
  ret void
}

!toolchain.dbg.cu = !{!1}
!toolchain.module.flags = !{!4}

!0 = !DILocation(line: 0, scope: !3)
!1 = distinct !DICompileUnit(language: DW_LANG_Codira, file: !2)
!2 = !DIFile(filename: "basic.code", directory: "")
!3 = distinct !DISubprogram(name: "_", scope: !1, file: !2, type: !DISubroutineType(types: !{}))
!4 = !{i32 1, !"Debug Info Version", i32 3}
