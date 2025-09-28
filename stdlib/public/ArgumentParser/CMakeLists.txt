add_library(ArgumentParser
  Completions/BashCompletionsGenerator.code
  Completions/CompletionsGenerator.code
  Completions/FishCompletionsGenerator.code
  Completions/ZshCompletionsGenerator.code

  "Parsable Properties/Argument.code"
  "Parsable Properties/ArgumentDiscussion.code"
  "Parsable Properties/ArgumentHelp.code"
  "Parsable Properties/ArgumentVisibility.code"
  "Parsable Properties/CompletionKind.code"
  "Parsable Properties/Errors.code"
  "Parsable Properties/Flag.code"
  "Parsable Properties/NameSpecification.code"
  "Parsable Properties/Option.code"
  "Parsable Properties/OptionGroup.code"

  "Parsable Types/AsyncParsableCommand.code"
  "Parsable Types/CommandGroup.code"
  "Parsable Types/CommandConfiguration.code"
  "Parsable Types/EnumerableFlag.code"
  "Parsable Types/ExpressibleByArgument.code"
  "Parsable Types/ParsableArguments.code"
  "Parsable Types/ParsableCommand.code"

  Parsing/ArgumentDecoder.code
  Parsing/ArgumentDefinition.code
  Parsing/ArgumentSet.code
  Parsing/CommandParser.code
  Parsing/InputKey.code
  Parsing/InputOrigin.code
  Parsing/Name.code
  Parsing/Parsed.code
  Parsing/ParsedValues.code
  Parsing/ParserError.code
  Parsing/SplitArguments.code

  Usage/DumpHelpGenerator.code
  Usage/HelpCommand.code
  Usage/HelpGenerator.code
  Usage/MessageInfo.code
  Usage/UsageGenerator.code

  Utilities/CollectionExtensions.code
  Utilities/Mutex.code
  Utilities/Platform.code
  Utilities/SequenceExtensions.code
  Utilities/StringExtensions.code
  Utilities/CodiraExtensions.code
  Utilities/Tree.code

  Validators/CodingKeyValidator.code
  Validators/NonsenseFlagsValidator.code
  Validators/ParsableArgumentsValidation.code
  Validators/PositionalArgumentsValidator.code
  Validators/UniqueNamesValidator.code)
# NOTE: workaround for CMake not setting up include flags yet
set_target_properties(ArgumentParser PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Codira_MODULE_DIRECTORY})
target_compile_options(ArgumentParser PRIVATE
  $<$<BOOL:${BUILD_TESTING}>:-enable-testing>)
target_link_libraries(ArgumentParser PRIVATE
  ArgumentParserToolInfo)
if(Foundation_FOUND)
  target_link_libraries(ArgumentParser PRIVATE
    Foundation)
endif()


_install_target(ArgumentParser)
set_property(GLOBAL APPEND PROPERTY ArgumentParser_EXPORTS ArgumentParser)
