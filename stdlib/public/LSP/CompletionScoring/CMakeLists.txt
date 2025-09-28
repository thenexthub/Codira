set(sources 
  Semantics/SemanticClassification.code
  Semantics/CompletionScore.code
  Semantics/Components
  Semantics/Components/TypeCompatibility.code
  Semantics/Components/StructuralProximity.code
  Semantics/Components/Flair.code
  Semantics/Components/CompletionKind.code
  Semantics/Components/Popularity.code
  Semantics/Components/ScopeProximity.code
  Semantics/Components/Availability.code
  Semantics/Components/SynchronicityCompatibility.code
  Semantics/Components/PopularityIndex.code
  Semantics/Components/ModuleProximity.code
  Semantics/Components/PopularityTable.code
  Utilities/UnsafeStackAllocator.code
  Utilities/Serialization
  Utilities/Serialization/BinaryCodable.code
  Utilities/Serialization/BinaryEncoder.code
  Utilities/Serialization/BinaryDecoder.code
  Utilities/Serialization/Conformances
  Utilities/Serialization/Conformances/Dictionary+BinaryCodable.code
  Utilities/Serialization/Conformances/OptionSet+BinaryCodable.code
  Utilities/Serialization/Conformances/Optional+BinaryCodable.code
  Utilities/Serialization/Conformances/Scalars+BinaryCodable.code
  Utilities/Serialization/Conformances/String+BinaryCodable.code
  Utilities/Serialization/Conformances/Array+BinaryCodable.code
  Utilities/CodiraExtensions.code
  Utilities/SelectTopK.code
  Utilities/UnsafeArray.code
  Text/CandidateBatch.code
  Text/MatchCollator.Match.code
  Text/UTF8Byte.code
  Text/MatchCollator.code
  Text/InfluencingIdentifiers.code
  Text/MatchCollator.Selection.code
  Text/ScoredMatchSelector.code
  Text/RejectionFilter.code
  Text/Pattern.code)

add_library(CompletionScoring STATIC ${sources})
set_target_properties(CompletionScoring PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(CompletionScoring PUBLIC
  CCompletionScoring)

add_library(CompletionScoringForPlugin STATIC ${sources})
set_target_properties(CompletionScoringForPlugin PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_link_libraries(CompletionScoringForPlugin PUBLIC
  CCompletionScoring)
