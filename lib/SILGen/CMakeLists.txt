
add_language_host_library(languageSILGen STATIC
  ArgumentSource.cpp
  Cleanup.cpp
  Condition.cpp
  FormalEvaluation.cpp
  ManagedValue.cpp
  ResultPlan.cpp
  RValue.cpp
  Scope.cpp
  SwitchEnumBuilder.cpp
  SILGen.cpp
  SILGenApply.cpp
  SILGenAvailability.cpp
  SILGenBridging.cpp
  SILGenBuilder.cpp
  SILGenBuiltin.cpp
  SILGenConcurrency.cpp
  SILGenConstructor.cpp
  SILGenConvert.cpp
  SILGenDecl.cpp
  SILGenDestructor.cpp
  SILGenDistributed.cpp
  SILGenDynamicCast.cpp
  SILGenEpilog.cpp
  SILGenExpr.cpp
  SILGenForeignError.cpp
  SILGenFunction.cpp
  SILGenGlobalVariable.cpp
  SILGenLazyConformance.cpp
  SILGenLocalArchetype.cpp
  SILGenLValue.cpp
  SILGenPack.cpp
  SILGenPattern.cpp
  SILGenPoly.cpp
  SILGenProlog.cpp
  SILGenRequests.cpp
  SILGenStmt.cpp
  SILGenThunk.cpp
  SILGenTopLevel.cpp
  SILGenType.cpp)
target_link_libraries(languageSILGen PRIVATE
  languageSerialization
  languageSIL)

set_language_toolchain_is_available(languageSILGen)
