/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "Format/FormatCodeProcessor.h"
#include <regex>
#include "Format/CommentHandler.h"
#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/DocProcessor/BreakParentTypeProcessor.h"
#include "Format/DocProcessor/ConcatTypeProcessor.h"
#include "Format/DocProcessor/DotProcessor.h"
#include "Format/DocProcessor/FuncArgProcessor.h"
#include "Format/DocProcessor/GroupProcessor.h"
#include "Format/DocProcessor/LambdaBodyProcessor.h"
#include "Format/DocProcessor/LambdaProcessor.h"
#include "Format/DocProcessor/LineDotProcessor.h"
#include "Format/DocProcessor/LineProcessor.h"
#include "Format/DocProcessor/MemberAccessProcessor.h"
#include "Format/DocProcessor/SeparateProcessor.h"
#include "Format/DocProcessor/SoftLineTypeProcessor.h"
#include "Format/DocProcessor/SoftLineWithSpaceTypeProcessor.h"
#include "Format/DocProcessor/StringTypeProcessor.h"
#include "Format/NodeFormatter/Decl/ClassDeclFormatter.h"
#include "Format/NodeFormatter/Decl/EnumDeclFormatter.h"
#include "Format/NodeFormatter/Decl/ExtendDeclFormatter.h"
#include "Format/NodeFormatter/Decl/FuncDeclFormatter.h"
#include "Format/NodeFormatter/Decl/GenericParamDeclFormatter.h"
#include "Format/NodeFormatter/Decl/InterfaceDeclFormatter.h"
#include "Format/NodeFormatter/Decl/MacroDeclFormatter.h"
#include "Format/NodeFormatter/Decl/MacroExpandDeclFormatter.h"
#include "Format/NodeFormatter/Decl/MacroExpandParamFormatter.h"
#include "Format/NodeFormatter/Decl/MainDeclFormatter.h"
#include "Format/NodeFormatter/Decl/PrimaryCtorDeclFormatter.h"
#include "Format/NodeFormatter/Decl/PropDeclFormatter.h"
#include "Format/NodeFormatter/Decl/StructDeclFormatter.h"
#include "Format/NodeFormatter/Decl/TypeAliasDeclFormatter.h"
#include "Format/NodeFormatter/Decl/VarDeclFormatter.h"
#include "Format/NodeFormatter/Decl/VarWithPatternDeclFormatter.h"
#include "Format/NodeFormatter/Expr/ArrayExprFormatter.h"
#include "Format/NodeFormatter/Expr/ArrayLitFormatter.h"
#include "Format/NodeFormatter/Expr/AsExprFormatter.h"
#include "Format/NodeFormatter/Expr/AssignExprFormatter.h"
#include "Format/NodeFormatter/Expr/BinaryExprFormatter.h"
#include "Format/NodeFormatter/Expr/BlockFormatter.h"
#include "Format/NodeFormatter/Expr/CallExprFormatter.h"
#include "Format/NodeFormatter/Expr/DoWhileExprFormatter.h"
#include "Format/NodeFormatter/Expr/ForInExprFormatter.h"
#include "Format/NodeFormatter/Expr/IfExprFormatter.h"
#include "Format/NodeFormatter/Expr/IncOrDecExprFormatter.h"
#include "Format/NodeFormatter/Expr/IsExprFormatter.h"
#include "Format/NodeFormatter/Expr/JumpExprFormatter.h"
#include "Format/NodeFormatter/Expr/LambdaExprFormatter.h"
#include "Format/NodeFormatter/Expr/LetPatternDestructorFormatter.h"
#include "Format/NodeFormatter/Expr/LitConstExprFormatter.h"
#include "Format/NodeFormatter/Expr/MacroExpandExprFormatter.h"
#include "Format/NodeFormatter/Expr/MatchExprFormatter.h"
#include "Format/NodeFormatter/Expr/MemberAccessFormatter.h"
#include "Format/NodeFormatter/Expr/OptionalChainExprFormatter.h"
#include "Format/NodeFormatter/Expr/OptionalExprFormatter.h"
#include "Format/NodeFormatter/Expr/ParenExprFormatter.h"
#include "Format/NodeFormatter/Expr/PrimitiveTypeExprFormatter.h"
#include "Format/NodeFormatter/Expr/QuoteExprFormatter.h"
#include "Format/NodeFormatter/Expr/RangeExprFormatter.h"
#include "Format/NodeFormatter/Expr/RefExprFormatter.h"
#include "Format/NodeFormatter/Expr/ReturnExprFormatter.h"
#include "Format/NodeFormatter/Expr/SpawnExprFormatter.h"
#include "Format/NodeFormatter/Expr/SubscriptExprFormatter.h"
#include "Format/NodeFormatter/Expr/SynchronizedExprFormatter.h"
#include "Format/NodeFormatter/Expr/ThrowExprFormatter.h"
#include "Format/NodeFormatter/Expr/TrailingClosureExprFormatter.h"
#include "Format/NodeFormatter/Expr/TryExprFormatter.h"
#include "Format/NodeFormatter/Expr/TupleLitFormatter.h"
#include "Format/NodeFormatter/Expr/TypeConvExprFormatter.h"
#include "Format/NodeFormatter/Expr/UnaryExprFormatter.h"
#include "Format/NodeFormatter/Expr/WhileExprFormatter.h"
#include "Format/NodeFormatter/Expr/WildcardExprFormatter.h"
#include "Format/NodeFormatter/Node/AnnotationFormatter.h"
#include "Format/NodeFormatter/Node/ClassBodyFormatter.h"
#include "Format/NodeFormatter/Node/FileFormatter.h"
#include "Format/NodeFormatter/Node/FuncArgFormatter.h"
#include "Format/NodeFormatter/Node/FuncBodyFormatter.h"
#include "Format/NodeFormatter/Node/FuncParamFormatter.h"
#include "Format/NodeFormatter/Node/FuncParamListFormatter.h"
#include "Format/NodeFormatter/Node/GenericConstraintFormatter.h"
#include "Format/NodeFormatter/Node/ImportSpecFormatter.h"
#include "Format/NodeFormatter/Node/InterfaceBodyFormatter.h"
#include "Format/NodeFormatter/Node/MatchCaseFormatter.h"
#include "Format/NodeFormatter/Node/MatchCaseOtherFormatter.h"
#include "Format/NodeFormatter/Node/ModifierFormatter.h"
#include "Format/NodeFormatter/Node/PackageSpecFormatter.h"
#include "Format/NodeFormatter/Node/StructBodyFormatter.h"
#include "Format/NodeFormatter/Pattern/ConstPatternFormatter.h"
#include "Format/NodeFormatter/Pattern/EnumPatternFormatter.h"
#include "Format/NodeFormatter/Pattern/ExceptTypePatternFormatter.h"
#include "Format/NodeFormatter/Pattern/TuplePatternFormatter.h"
#include "Format/NodeFormatter/Pattern/TypePatternFormatter.h"
#include "Format/NodeFormatter/Pattern/VarOrEnumPatternFormatter.h"
#include "Format/NodeFormatter/Pattern/VarPatternFormatter.h"
#include "Format/NodeFormatter/Pattern/WildcardPatternFormatter.h"
#include "Format/NodeFormatter/Type/ConstantTypeFormatter.h"
#include "Format/NodeFormatter/Type/FuncTypeFormatter.h"
#include "Format/NodeFormatter/Type/OptionTypeFormatter.h"
#include "Format/NodeFormatter/Type/ParenTypeFormatter.h"
#include "Format/NodeFormatter/Type/PrimitiveTypeFormatter.h"
#include "Format/NodeFormatter/Type/QualifiedTypeFormatter.h"
#include "Format/NodeFormatter/Type/RefTypeFormatter.h"
#include "Format/NodeFormatter/Type/ThisTypeFormatter.h"
#include "Format/NodeFormatter/Type/TupleTypeFormatter.h"
#include "Format/NodeFormatter/Type/VArrayTypeFormatter.h"
#include "Format/OptionContext.h"
#include "Format/SimultaneousIterator.h"
#include "Format/TomlParser.h"

namespace Codira::Format {
using namespace Codira;
using namespace Codira::AST;

#ifdef _WIN32
static const std::string DELIMITER = "\\";
#else
static const std::string DELIMITER = "/";
#endif
static const int MAX_LINEBREAKS = 2;

std::string PathJoin(const std::string& baseDir, const std::string& baseName)
{
    std::string lastChar(1, baseDir.at(baseDir.length() - 1));

    if (lastChar == DELIMITER) {
        return baseDir + baseName;
    } else {
        return baseDir + DELIMITER + baseName;
    }
}

bool HasEnding(std::string const& fullString, std::string const& ending)
{
    if (fullString.length() >= ending.length()) {
        return (fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0);
    }
    return false;
}

void TraveDepthLimitedDirs(
    const std::string& path, std::map<std::string, std::string>& fileMap, int depth, const int maxDepth)
{
    if (depth >= maxDepth && maxDepth != MAX_RECURSION_DEPTH) {
        return;
    }
    DIR* pd = opendir(path.c_str());
    if (!pd) {
        std::cout << "cant open dirs(" << path << "), opendir fail" << std::endl;
        return;
    }
    dirent* dir = nullptr;
    struct stat statBuf {};
    while ((dir = readdir(pd)) != nullptr) {
        std::string dName = std::string(dir->d_name);
        std::string subFileOrDir = PathJoin(path, dName);
        (void)stat(subFileOrDir.c_str(), &statBuf); // use lstat if want to ignore symbolic-link-dir
        if (S_ISDIR(statBuf.st_mode)) {
            // continue if present dir or  parent dir
            if (dName == "." || dName == "..") {
                continue;
            }
            // traverse sub-dir
            TraveDepthLimitedDirs(subFileOrDir, fileMap, depth + 1, maxDepth);
        } else {
            std::string name = dir->d_name;
            if (HasEnding(name, ".code")) {
                (void)fileMap.emplace(subFileOrDir, name);
            }
            continue;
        }
    }
    (void)closedir(pd);
}

void RegisterNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<PackageSpecFormatter>(ASTKind::PACKAGE_SPEC, options);
    toSourceManager.RegisterNode<FileFormatter>(ASTKind::FILE, options);
    toSourceManager.RegisterNode<ImportSpecFormatter>(ASTKind::IMPORT_SPEC, options);
    toSourceManager.RegisterNode<ClassBodyFormatter>(ASTKind::CLASS_BODY, options);
    toSourceManager.RegisterNode<StructBodyFormatter>(ASTKind::STRUCT_BODY, options);
    toSourceManager.RegisterNode<InterfaceBodyFormatter>(ASTKind::INTERFACE_BODY, options);
    toSourceManager.RegisterNode<FuncBodyFormatter>(ASTKind::FUNC_BODY, options);
    toSourceManager.RegisterNode<FuncParamFormatter>(ASTKind::FUNC_PARAM, options);
    toSourceManager.RegisterNode<FuncParamListFormatter>(ASTKind::FUNC_PARAM_LIST, options);
    toSourceManager.RegisterNode<FuncArgFormatter>(ASTKind::FUNC_ARG, options);
    toSourceManager.RegisterNode<AnnotationFormatter>(ASTKind::ANNOTATION, options);
    toSourceManager.RegisterNode<GenericConstraintFormatter>(ASTKind::GENERIC_CONSTRAINT, options);
    toSourceManager.RegisterNode<MatchCaseFormatter>(ASTKind::MATCH_CASE, options);
    toSourceManager.RegisterNode<MatchCaseOtherFormatter>(ASTKind::MATCH_CASE_OTHER, options);
    toSourceManager.RegisterNode<ModifierFormatter>(ASTKind::MODIFIER, options);
}

void RegisterDeclNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<ClassDeclFormatter>(ASTKind::CLASS_DECL, options);
    toSourceManager.RegisterNode<InterfaceDeclFormatter>(ASTKind::INTERFACE_DECL, options);
    toSourceManager.RegisterNode<EnumDeclFormatter>(ASTKind::ENUM_DECL, options);
    toSourceManager.RegisterNode<MacroDeclFormatter>(ASTKind::MACRO_DECL, options);
    toSourceManager.RegisterNode<ExtendDeclFormatter>(ASTKind::EXTEND_DECL, options);
    toSourceManager.RegisterNode<MainDeclFormatter>(ASTKind::MAIN_DECL, options);
    toSourceManager.RegisterNode<FuncDeclFormatter>(ASTKind::FUNC_DECL, options);
    toSourceManager.RegisterNode<TypeAliasDeclFormatter>(ASTKind::TYPE_ALIAS_DECL, options);
    toSourceManager.RegisterNode<GenericParamDeclFormatter>(ASTKind::GENERIC_PARAM_DECL, options);
    toSourceManager.RegisterNode<MacroExpandDeclFormatter>(ASTKind::MACRO_EXPAND_DECL, options);
    toSourceManager.RegisterNode<MacroExpandParamFormatter>(ASTKind::MACRO_EXPAND_PARAM, options);
    toSourceManager.RegisterNode<VarDeclFormatter>(ASTKind::VAR_DECL, options);
    toSourceManager.RegisterNode<VarWithPatternDeclFormatter>(ASTKind::VAR_WITH_PATTERN_DECL, options);
    toSourceManager.RegisterNode<PropDeclFormatter>(ASTKind::PROP_DECL, options);
    toSourceManager.RegisterNode<StructDeclFormatter>(ASTKind::STRUCT_DECL, options);
    toSourceManager.RegisterNode<PrimaryCtorDeclFormatter>(ASTKind::PRIMARY_CTOR_DECL, options);
}

void RegisterExprNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<BlockFormatter>(ASTKind::BLOCK, options);
    toSourceManager.RegisterNode<LetPatternDestructorFormatter>(ASTKind::LET_PATTERN_DESTRUCTOR, options);
    toSourceManager.RegisterNode<LitConstExprFormatter>(ASTKind::LIT_CONST_EXPR, options);
    toSourceManager.RegisterNode<CallExprFormatter>(ASTKind::CALL_EXPR, options);
    toSourceManager.RegisterNode<MemberAccessFormatter>(ASTKind::MEMBER_ACCESS, options);
    toSourceManager.RegisterNode<RefExprFormatter>(ASTKind::REF_EXPR, options);
    toSourceManager.RegisterNode<PrimitiveTypeExprFormatter>(ASTKind::PRIMITIVE_TYPE_EXPR, options);
    toSourceManager.RegisterNode<ArrayLitFormatter>(ASTKind::ARRAY_LIT, options);
    toSourceManager.RegisterNode<TypeConvExprFormatter>(ASTKind::TYPE_CONV_EXPR, options);
    toSourceManager.RegisterNode<SpawnExprFormatter>(ASTKind::SPAWN_EXPR, options);
    toSourceManager.RegisterNode<SynchronizedExprFormatter>(ASTKind::SYNCHRONIZED_EXPR, options);
    toSourceManager.RegisterNode<TryExprFormatter>(ASTKind::TRY_EXPR, options);
    toSourceManager.RegisterNode<ThrowExprFormatter>(ASTKind::THROW_EXPR, options);
    toSourceManager.RegisterNode<LambdaExprFormatter>(ASTKind::LAMBDA_EXPR, options);
    toSourceManager.RegisterNode<ReturnExprFormatter>(ASTKind::RETURN_EXPR, options);
    toSourceManager.RegisterNode<UnaryExprFormatter>(ASTKind::UNARY_EXPR, options);
    toSourceManager.RegisterNode<BinaryExprFormatter>(ASTKind::BINARY_EXPR, options);
    toSourceManager.RegisterNode<AssignExprFormatter>(ASTKind::ASSIGN_EXPR, options);
    toSourceManager.RegisterNode<IfExprFormatter>(ASTKind::IF_EXPR, options);
    toSourceManager.RegisterNode<IsExprFormatter>(ASTKind::IS_EXPR, options);
    toSourceManager.RegisterNode<AsExprFormatter>(ASTKind::AS_EXPR, options);
    toSourceManager.RegisterNode<JumpExprFormatter>(ASTKind::JUMP_EXPR, options);
    toSourceManager.RegisterNode<WildcardExprFormatter>(ASTKind::WILDCARD_EXPR, options);
    toSourceManager.RegisterNode<IncOrDecExprFormatter>(ASTKind::INC_OR_DEC_EXPR, options);
    toSourceManager.RegisterNode<ForInExprFormatter>(ASTKind::FOR_IN_EXPR, options);
    toSourceManager.RegisterNode<QuoteExprFormatter>(ASTKind::QUOTE_EXPR, options);
    toSourceManager.RegisterNode<MacroExpandExprFormatter>(ASTKind::MACRO_EXPAND_EXPR, options);
    toSourceManager.RegisterNode<WhileExprFormatter>(ASTKind::WHILE_EXPR, options);
    toSourceManager.RegisterNode<DoWhileExprFormatter>(ASTKind::DO_WHILE_EXPR, options);
    toSourceManager.RegisterNode<ParenExprFormatter>(ASTKind::PAREN_EXPR, options);
    toSourceManager.RegisterNode<RangeExprFormatter>(ASTKind::RANGE_EXPR, options);
    toSourceManager.RegisterNode<MatchExprFormatter>(ASTKind::MATCH_EXPR, options);
    toSourceManager.RegisterNode<SubscriptExprFormatter>(ASTKind::SUBSCRIPT_EXPR, options);
    toSourceManager.RegisterNode<TrailingClosureExprFormatter>(ASTKind::TRAIL_CLOSURE_EXPR, options);
    toSourceManager.RegisterNode<ArrayExprFormatter>(ASTKind::ARRAY_EXPR, options);
    toSourceManager.RegisterNode<OptionalChainExprFormatter>(ASTKind::OPTIONAL_CHAIN_EXPR, options);
    toSourceManager.RegisterNode<OptionalExprFormatter>(ASTKind::OPTIONAL_EXPR, options);
    toSourceManager.RegisterNode<TupleLitFormatter>(ASTKind::TUPLE_LIT, options);
}

void RegisterPatternNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<EnumPatternFormatter>(ASTKind::ENUM_PATTERN, options);
    toSourceManager.RegisterNode<VarOrEnumPatternFormatter>(ASTKind::VAR_OR_ENUM_PATTERN, options);
    toSourceManager.RegisterNode<TuplePatternFormatter>(ASTKind::TUPLE_PATTERN, options);
    toSourceManager.RegisterNode<VarPatternFormatter>(ASTKind::VAR_PATTERN, options);
    toSourceManager.RegisterNode<ConstPatternFormatter>(ASTKind::CONST_PATTERN, options);
    toSourceManager.RegisterNode<TypePatternFormatter>(ASTKind::TYPE_PATTERN, options);
    toSourceManager.RegisterNode<ExceptTypePatternFormatter>(ASTKind::EXCEPT_TYPE_PATTERN, options);
    toSourceManager.RegisterNode<WildcardPatternFormatter>(ASTKind::WILDCARD_PATTERN, options);
}

void RegisterTypeNode(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterNode<PrimitiveTypeFormatter>(ASTKind::PRIMITIVE_TYPE, options);
    toSourceManager.RegisterNode<RefTypeFormatter>(ASTKind::REF_TYPE, options);
    toSourceManager.RegisterNode<ThisTypeFormatter>(ASTKind::THIS_TYPE, options);
    toSourceManager.RegisterNode<ParenTypeFormatter>(ASTKind::PAREN_TYPE, options);
    toSourceManager.RegisterNode<TupleTypeFormatter>(ASTKind::TUPLE_TYPE, options);
    toSourceManager.RegisterNode<QualifiedTypeFormatter>(ASTKind::QUALIFIED_TYPE, options);
    toSourceManager.RegisterNode<FuncTypeFormatter>(ASTKind::FUNC_TYPE, options);
    toSourceManager.RegisterNode<OptionTypeFormatter>(ASTKind::OPTION_TYPE, options);
    toSourceManager.RegisterNode<VArrayTypeFormatter>(ASTKind::VARRAY_TYPE, options);
    toSourceManager.RegisterNode<ConstantTypeFormatter>(ASTKind::CONSTANT_TYPE, options);
}

void RegisterDocProcessors(ASTToFormatSource& toSourceManager, FormattingOptions& options)
{
    toSourceManager.RegisterDocProcessors<StringTypeProcessor>(DocType::STRING, options);
    toSourceManager.RegisterDocProcessors<ConcatTypeProcessor>(DocType::CONCAT, options);
    toSourceManager.RegisterDocProcessors<LineProcessor>(DocType::LINE, options);
    toSourceManager.RegisterDocProcessors<ArgsProcessor>(DocType::ARGS, options);
    toSourceManager.RegisterDocProcessors<FuncArgProcessor>(DocType::FUNC_ARG, options);
    toSourceManager.RegisterDocProcessors<LambdaBodyProcessor>(DocType::LAMBDA_BODY, options);
    toSourceManager.RegisterDocProcessors<MemberAccessProcessor>(DocType::MEMBER_ACCESS, options);
    toSourceManager.RegisterDocProcessors<LambdaProcessor>(DocType::LAMBDA, options);
    toSourceManager.RegisterDocProcessors<LineDotProcessor>(DocType::LINE_DOT, options);
    toSourceManager.RegisterDocProcessors<DotProcessor>(DocType::DOT, options);
    toSourceManager.RegisterDocProcessors<SeparateProcessor>(DocType::SEPARATE, options);
    toSourceManager.RegisterDocProcessors<GroupProcessor>(DocType::GROUP, options);
    toSourceManager.RegisterDocProcessors<SoftLineWithSpaceTypeProcessor>(DocType::SOFTLINE_WITH_SPACE, options);
    toSourceManager.RegisterDocProcessors<SoftLineTypeProcessor>(DocType::SOFTLINE, options);
    toSourceManager.RegisterDocProcessors<BreakParentTypeProcessor>(DocType::BREAK_PARENT, options);
}

namespace {
class FormatCodeProcessor {
public:
    FormatCodeProcessor(const std::string& rawCode, const std::string& filepath, const Region region)
        : rawCode(rawCode), filepath(filepath), sm(), diag(), regionToFormat(region), tracker(region)
    {
        diag.SetSourceManager(&sm);
        OptionContext& optionContext = OptionContext::GetInstance();
        this->options = optionContext.GetConfigOptions();
    }

    std::optional<std::string> Run()
    {
        auto inputTokens = RunLexer(rawCode, filepath);
        if (inputTokens.empty()) {
            return "";
        }

        Parser parser(inputTokens, diag, sm);
        GlobalOptions globalOptions;
        globalOptions.outputMode = GlobalOptions::OutputMode::CHIR;
        globalOptions.commonPartCodeo = "hasValue";
        parser.SetCompileOptions(globalOptions);
        auto file = parser.ParseTopLevel();
        diag.EmitCategoryGroup();
        if (parser.GetDiagnosticEngine().GetErrorCount() > 0) {
            return std::nullopt;
        }

        std::string result;
        auto formatted = TransformAstToText(file);
        if (tracker.actuallyFormattedStart.has_value() && tracker.actuallyFormattedEnd.has_value()) {
            result = ConstructResultingText(file, inputTokens, formatted);
        } else {
            // couldn't find any ast nodes to format, should report this to user?
            result = rawCode;
        }

        if (result.back() != '\n') {
            result.push_back('\n');
        }
        return result;
    }

private:
    const std::string& rawCode;
    const std::string& filepath;
    SourceManager sm;
    DiagnosticEngine diag;
    Region regionToFormat;
    RegionFormattingTracker tracker;
    FormattingOptions options;

    std::string TransformAstToText(const OwnedPtr<File>& file)
    {
        ASTToFormatSource astTransformer(tracker, sm);
        SetupTranformer(astTransformer);
        Doc doc = astTransformer.ASTToDoc(file.get());
        return astTransformer.DocToString(doc);
    }

    std::vector<Token> GetTokensBetweenPositions(
        const std::vector<Token>& allTokens, const Codira::Position& start, const Codira::Position& end)
    {
        std::vector<Token> result;
        auto iterator = allTokens.begin();

        while (iterator != allTokens.end() && iterator->Begin() < start) {
            ++iterator;
        }
        while (iterator != allTokens.end() && iterator->End() <= end) {
            result.push_back(*iterator);
            ++iterator;
        }
        return result;
    }

    int CalculateIndentForCodeFragment(
        const std::vector<Token>& originalFragmentTokens, const std::vector<Token>& formattedFragmentTokens)
    {
        auto formattedFront = formattedFragmentTokens.front().Begin();
        auto originalFront = originalFragmentTokens.front().Begin();
        auto formattedPrefix =
            sm.GetContentBetween(Position(formattedFront.fileID, formattedFront.line, 1), formattedFront);
        auto isOnNewLineInFormattedCode = formattedPrefix.find_first_not_of(" \t") == std::string::npos;
        auto originalPrefix =
            sm.GetContentBetween(Position(originalFront.fileID, originalFront.line, 1), originalFront);
        auto isOnNewLineInOriginalCode = originalPrefix.find_first_not_of(" \t") == std::string::npos;
        int indentation;
        if (isOnNewLineInFormattedCode && isOnNewLineInOriginalCode) {
            indentation = formattedFront.column - 1;
        } else if (!isOnNewLineInFormattedCode && !isOnNewLineInOriginalCode) {
            auto lastSymbolInOriginalCodeColumn = formattedPrefix.find_last_not_of(" \t") + 1;
            auto indentationInFormattedText =
                (formattedFront.column - static_cast<int>(lastSymbolInOriginalCodeColumn)) - 1;
            indentation = indentationInFormattedText;
        } else if (isOnNewLineInOriginalCode && !isOnNewLineInFormattedCode) {
            indentation = originalFront.column - 1;
        } else { // !isOnNewLineInOriginal && isOnNewLineInFormattedCode
            indentation = 1;
        }
        return indentation;
    }

    std::string ConstructTextForCodeFragment(
        const std::vector<Token>& originalFileAllTokens, const std::vector<Token>& formattedTextTokens)
    {
        auto formattedBegin = tracker.actuallyFormattedStart.value();
        auto formattedEnd = tracker.actuallyFormattedEnd.value();
        auto originalFragmentTokens = GetTokensBetweenPositions(originalFileAllTokens, formattedBegin, formattedEnd);
        auto formattedFragmentTokens = formattedTextTokens;
        auto preciseFragmentBounds = tracker.GetPreciseFragmentBounds(sm);
        if (preciseFragmentBounds.has_value()) {
            auto smallerFragments = ExtractTokensBetweenPositions(originalFragmentTokens, formattedFragmentTokens,
                preciseFragmentBounds->first, preciseFragmentBounds->second);
            if (smallerFragments.has_value()) {
                std::tie(originalFragmentTokens, formattedFragmentTokens) = smallerFragments.value();
            }
        }

        auto fragmentText = InsertComments(originalFragmentTokens, formattedFragmentTokens, sm);
        auto contentBefore =
            sm.GetContentBetween(originalFileAllTokens.front().Begin(), originalFragmentTokens.front().Begin());
        auto contentAfter =
            sm.GetContentBetween(originalFragmentTokens.back().End(), originalFileAllTokens.back().End());

        (void)contentBefore.erase(contentBefore.find_last_not_of(" \t") + 1);

        auto blankLinesBeforeSnippet = originalFragmentTokens.front().Begin().line - tracker.shouldFormatBegin.line;
        if (blankLinesBeforeSnippet > 1) {
            TrimTrailingEmptyLines(contentBefore, static_cast<size_t>(blankLinesBeforeSnippet));
        }
        (void)contentBefore.erase(contentBefore.find_last_not_of(" ") + 1);

        auto blankLinesAfterSnippet = tracker.shouldFormatEnd.line - originalFragmentTokens.back().Begin().line;
        if (blankLinesAfterSnippet > 1) {
            TrimLeadingEmptyLines(contentAfter, static_cast<size_t>(blankLinesAfterSnippet - 1));
        }

        auto indentation = CalculateIndentForCodeFragment(originalFragmentTokens, formattedFragmentTokens);
        for (int i = 0; i < indentation; ++i) {
            contentBefore.push_back(' ');
        }

        return contentBefore + fragmentText + contentAfter;
    }

    std::string ConstructResultingText(OwnedPtr<File>&, const std::vector<Token>& inputTokens, std::string& formatted)
    {
        (void)formatted.erase(0, formatted.find_first_not_of(" \t\n"));
        (void)formatted.erase(formatted.find_last_not_of(" \t\n") + 1);
        auto formattedTokens = RunLexer(formatted, "___formattedSnippet.code");
        if (!regionToFormat.isWholeFile) {
            return ConstructTextForCodeFragment(inputTokens, formattedTokens);
        } else {
            return InsertComments(inputTokens, formattedTokens, sm);
        }
    }

    std::vector<Token> RunLexer(const std::string& code, const std::string& path)
    {
        auto fileID = sm.AddSource(path, code);
        auto lexer = Lexer(fileID, code, diag, sm);
        return lexer.GetTokens();
    }

    void SetupTranformer(ASTToFormatSource& transformer)
    {
        RegisterNode(transformer, options);
        RegisterDeclNode(transformer, options);
        RegisterExprNode(transformer, options);
        RegisterPatternNode(transformer, options);
        RegisterTypeNode(transformer, options);
        RegisterDocProcessors(transformer, options);
    }

    void TrimTrailingEmptyLines(std::string& str, size_t maxLinesToTrim)
    {
        size_t currentPos = str.length();
        size_t lineBreakCount = 0;
        for (size_t i = str.length() - 1; i > 0; i--) {
            auto ch = str[i];
            if (!std::isspace(ch)) {
                break;
            }
            if (ch == '\n') {
                lineBreakCount++;
                currentPos = i + 1;
                if (lineBreakCount > maxLinesToTrim) {
                    break;
                }
            }
        }
        str.erase(lineBreakCount >= MAX_LINEBREAKS ? currentPos + 1 : currentPos);
    }

    void TrimLeadingEmptyLines(std::string& str, size_t maxLinesToTrim)
    {
        size_t currentPos = 0;
        size_t lineBreakCount = 0;
        for (size_t i = 0; i < str.length() - 1; i++) {
            auto ch = str[i];
            if (!std::isspace(ch)) {
                break;
            }
            if (ch == '\n') {
                lineBreakCount++;
                currentPos = i;
                if (lineBreakCount > maxLinesToTrim) {
                    break;
                }
            }
        }
        str.erase(0, currentPos);
    }
};
} // namespace

std::optional<std::string> FormatText(const std::string& rawCode, const std::string& filepath, const Region region)
{
    auto formatCode = FormatCodeProcessor(rawCode, filepath, region);
    return formatCode.Run();
}

bool FormatFile(std::string& rawCode, const std::string& filepath, std::string& sourceFormat, Region regionToFormat)
{
    std::ifstream instream(filepath);
    std::string buffer((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    instream.close();
    rawCode = buffer;
    if (regionToFormat.startLine == -1 || regionToFormat.endLine == -1) {
        regionToFormat = Region::wholeFile;
    }
    auto formatted = FormatText(rawCode, filepath, regionToFormat);
    if (!formatted) {
        return false;
    }
    sourceFormat = formatted.value();
    return true;
}

std::string GetTargetName(
    const std::string& file, const std::string& fileName, const std::string& fmtDirPath, const std::string& outPath)
{
    std::string outPathRel, targetName;
    if (!outPath.empty()) {
        std::string relPathNotFName = file.substr(0, file.size() - fileName.size());
        auto relPath = FileUtil::GetRelativePath(fmtDirPath, relPathNotFName) | FileUtil::IdenticalFunc;
        outPathRel = outPath + DELIMITER + relPath;
        int creatRel = 0;
        if (!FileUtil::IsDir(outPathRel)) {
            creatRel = FileUtil::CreateDirs(outPathRel);
        }
        if (creatRel != -1) {
            targetName = outPath + DELIMITER + relPath + fileName;
        }
    } else {
        targetName = file;
    }
    return targetName;
}

int FmtDir(const std::string& fmtDirPath, const std::string& dirOutputPath)
{
    std::regex reg = std::regex("[*|;&$><`?!\\n]+");
    if (std::regex_search(fmtDirPath, reg)) {
        Errorln("The path cannot contain invalid characters: *|;&$><`?!");
        return ERR;
    }
    auto absDirPath = FileUtil::GetAbsPath(fmtDirPath) | FileUtil::IdenticalFunc;
    std::map<std::string, std::string> filesMap;
    /* Obtain the path and name of the source file. Save to file map. */
    TraveDepthLimitedDirs(absDirPath, filesMap, DEPTH_OF_RECURSION, MAX_RECURSION_DEPTH);
    for (auto& file : filesMap) {
        std::string sourceFormat, targetName, rawCode;
        /* Format the file content and save the formatted content to sourceFormat. */
        if (!FormatFile(rawCode, file.first, sourceFormat, Region::wholeFile)) {
            sourceFormat = rawCode;
        }
        /* Obtain the output file path based on the output path and retain the source file directory structure. */
        targetName = GetTargetName(file.first, file.second, absDirPath, dirOutputPath);
        if (targetName.empty()) {
            Errorln("Create target file error.");
            return ERR;
        }
        std::ofstream outStream(targetName, std::ios::out | std::ofstream::binary);
        if (!outStream) {
            Errorln("Create target file error.");
            return ERR;
        }
        auto length = sourceFormat.size();
        (void)outStream.write(sourceFormat.c_str(), static_cast<long>(length));
        outStream.close();
    }
    return OK;
}
} // namespace Codira::Format
