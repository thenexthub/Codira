/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "../Prologue.inc.sql"

SQL(Begin, BEGIN);

SQL(Commit, COMMIT);

SQL(Rollback, ROLLBACK);

SQL(Analyze, ANALYZE);

SQL(Vacuum, VACUUM);

SQL(PragmaUserVersion, PRAGMA user_version);

SQL(PragmaApplicationID, PRAGMA application_id);

SQL(PragmaOptimize, PRAGMA optimize);

SQL(PragmaWAL, PRAGMA journal_mode);

SQL(SelectSchemaEmpty,
  SELECT NOT EXISTS(SELECT 1 FROM sqlite_schema)
);

SQL(SelectConfig,
  SELECT Value FROM config WHERE Name = :Name
);

SQL(InsertConfig,
  INSERT OR REPLACE INTO config VALUES(:Name, :Value)
);

SQL(InsertFile,
  INSERT INTO files(FileURI, FileDigest) VALUES(:FileURI, :FileDigest) ON CONFLICT
  DO UPDATE SET FileDigest = excluded.FileDigest WHERE excluded.FileDigest IS NOT NULL
);

SQL(SelectFileWithId,
  SELECT * FROM files WHERE FileID = :FileID
);

SQL(SelectFileWithUri,
  SELECT * FROM files WHERE FileURI = :FileURI
);

SQL(SelectFileDigestWithId,
  SELECT FileDigest FROM files WHERE FileID = :FileID
);

SQL(InsertFileWithID,
  INSERT INTO files VALUES(:FileID, :FileURI, :FileDigest, true, :PackageName, :ModuleName) ON CONFLICT
  DO UPDATE SET FileDigest = excluded.FileDigest WHERE excluded.FileDigest IS NOT NULL
);

SQL(DeleteFile,
  DELETE FROM files WHERE FileURI = :FileURI
);

#define STALE_FILE_COMMON_PART FROM files WHERE NOT Enumerated
SQL(SelectStaleFileCount,
  SELECT count(*) STALE_FILE_COMMON_PART
);
SQL(DeleteStaleFiles,
  DELETE FROM files WHERE rowid IN(SELECT rowid STALE_FILE_COMMON_PART LIMIT 10)
);


SQL(UpdateFilesNotEnumerated,
  UPDATE files SET Enumerated = FALSE WHERE Enumerated
);

SQL(UpdateFileEnumerated,
  UPDATE files SET Enumerated = TRUE WHERE FileURI = :FileURI
);

SQL(UpdateRenameFile,
  UPDATE files SET FileURI = :NewFileURI WHERE FileURI = :OldFileURI
);

SQL(SelectFileExists,
  SELECT EXISTS(SELECT 1 FROM files WHERE FileURI = :FileURI AND Enumerated)
);

SQL(SelectFileDigest,
  SELECT FileDigest FROM files WHERE FileURI = :FileURI
);

SQL(InsertInclude,
  INSERT OR IGNORE INTO includes(SymbolID, IncludeHeader) VALUES(:SymbolID, :IncludeHeader)
);

SQL(SelectInclude,
  SELECT * FROM includes WHERE SymbolID = :SymbolID
);

SQL(InsertSymbol,
  INSERT OR REPLACE INTO symbols VALUES(
    :ID,
    :Kind,
    :SubKind,
    :Lang,
    :Properties,
    :Name,
    :Scope,
    :DefinitionFileURI,
    :DefinitionStartLine,
    :DefinitionStartColumn,
    :DefinitionEndLine,
    :DefinitionEndColumn,
    :DeclarationFileURI,
    :DeclarationStartLine,
    :DeclarationStartColumn,
    :DeclarationEndLine,
    :DeclarationEndColumn,
    :Signature,
    :TemplateSpecializationArgs,
    :CompletionSnippetSuffix,
    :Documentation,
    :ReturnType,
    :Type,
    :Flags,
    :Codeosym,
    :MemberParam,
    :Modifier,
    :Deprecated,
    :Syscap,
    :PkgModifier,
    :CurModule,
    :MacroCallFileURI,
    :MacroCallStartLine,
    :MacroCallStartColumn,
    :MacroCallEndLine,
    :MacroCallEndColumn
  )
);

SQL(MultiInsertSymbolsHead,
  INSERT OR REPLACE INTO symbols VALUES
);

SQL(MultiInsertSymbolsValue,
  (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectSymbols,
  SELECT * FROM symbols
);

SQL(SelectSymbol,
  SELECT * FROM symbols WHERE ID = :ID
);

SQL(SelectSymbolByName,
  SELECT * FROM symbols WHERE Name = :Name AND Lang = :Lang
);

SQL(SelectSymbolsByLocation,
  SELECT * FROM symbols WHERE (
    DeclarationFileURI GLOB '*' || :FilePath
    AND :Line BETWEEN DeclarationStartLine AND DeclarationEndLine
    AND :Column BETWEEN DeclarationStartColumn AND DeclarationEndColumn
  ) OR (
    DefinitionFileURI GLOB '*' || :FilePath
    AND :Line BETWEEN DefinitionStartLine AND DefinitionEndLine
    AND :Column BETWEEN DefinitionStartColumn AND DefinitionEndColumn
  )
);

SQL(SelectSymbolsByPkgName,
  SELECT * FROM symbols WHERE Scope = :Scope OR Scope LIKE :ScopePrefix || '%'
);

SQL(InsertCompletion,
  INSERT OR IGNORE INTO completions VALUES(
    :SymbolID,
    :Label,
    :InsertText
  )
);

SQL(MultiInsertCompletionsHead,
  INSERT OR IGNORE INTO completions VALUES
);

SQL(MultiInsertCompletionsValue,
  (
    ?,
    ?,
    ?
  )
);

SQL(SelectCompletions,
  SELECT symbols.ID, symbols.Name, symbols.Modifier, symbols.Kind, symbols.DefinitionFileURI, symbols.Scope, symbols.Codeosym, symbols.Deprecated, symbols.Syscap, symbols.PkgModifier,
         completions.*
  FROM symbols
  JOIN completions ON symbols.ID = completions.SymbolID
  Where symbols.Name LIKE :Name COLLATE NOCASE AND symbols.Modifier IN (2,3,4)
);

SQL(SelectCompletion,
  SELECT * FROM completions WHERE SymbolID = :SymbolID
);

SQL(SelectMatchingSymbols,
  SELECT symbols.*,
         symbols_search.Rank
  FROM symbols
  JOIN symbols_search USING(ID)
    WHERE symbols_search.Name MATCH :Pattern
      AND (:Flags IS NULL OR :Flags & Flags)
      AND (:Scope IS NULL OR :Scope = symbols.Scope)
    ORDER BY Rank
);

SQL(InsertReference,
  INSERT OR IGNORE INTO refs VALUES(
    :SymbolID,
    :FileURI,
    :StartLine,
    :StartColumn,
    :EndLine,
    :EndColumn,
    :Kind,
    :ContainerID,
    :isCodeoRef,
    :isSuper
  )
);

SQL(MultiInsertReferencesHead,
  INSERT OR IGNORE INTO refs VALUES
);

SQL(MultiInsertReferencesValue,
  (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectReference,
  SELECT * FROM refs WHERE SymbolID = :SymbolID AND Kind & :Kind
);

SQL(SelectFileReference,
  SELECT * FROM refs WHERE FileURI = :FileURI AND Kind & :Kind
);

SQL(SelectReferred,
  SELECT * FROM refs WHERE ContainerID = :ContainerID
);

SQL(SelectReferenceAt,
  SELECT * FROM refs
  WHERE FileURI = :FileURI AND Kind & :Kind
  AND :PositionLine BETWEEN StartLine AND EndLine
  AND :PositionColumn BETWEEN StartColumn AND EndColumn
);

SQL(SelectUsedSymbols,
  SELECT DISTINCT SymbolID FROM refs
  WHERE FileURI = :FileURI
);

SQL(SelectReferenceCount,
  SELECT count(*) FROM refs WHERE SymbolID = :SymbolID AND Kind & :Kind
);

SQL(InsertRelation,
  INSERT OR IGNORE INTO rels VALUES(:SubjectID, :Predicate, :ObjectID)
);

SQL(MultiInsertRelationsHead,
  INSERT OR IGNORE INTO rels VALUES
);

SQL(MultiInsertRelationsValue,
  (
    ?,
    ?,
    ?
  )
);

SQL(SelectRelation,
  SELECT * FROM rels WHERE SubjectID = :SubjectID AND Predicate = :Predicate
);

SQL(SelectReverseRelation,
  SELECT * FROM rels WHERE ObjectID = :ObjectID AND Predicate = :Predicate
);

SQL(SelectSubjectFromRelations,
  SELECT SubjectID FROM rels WHERE ObjectID = :ObjectID AND Predicate = :Predicate
);

SQL(InsertCallRelation,
  INSERT OR IGNORE INTO calls VALUES(
    :SubjectID,
    :Predicate,
    :ObjectID,
    :FileURI,
    :StartLine,
    :StartColumn,
    :EndLine,
    :EndColumn
  )
);

SQL(MultiInsertCallRelationsHead,
  INSERT OR IGNORE INTO calls VALUES
);

SQL(MultiInsertCallRelationsValue,
  (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectCallRelation,
  SELECT * FROM calls WHERE SubjectID = :SubjectID AND Predicate = :Predicate
);

SQL(SelectCallRelationByObject,
  SELECT * FROM calls WHERE ObjectID = :ObjectID AND Predicate = :Predicate
);

SQL(ApplyFilePathMapping,
  UPDATE files SET FileURI = :NewPathURI || substr(FileURI, length(:OldPathURI) + 1)
  WHERE FileURI GLOB :OldPathURI || '/' || '*'
);

SQL(InsertFilesRelation,
  INSERT OR IGNORE INTO file_rels(
    FileURI, HeaderURI,
    HashLine, HashColumn,
    StartLine, StartColumn,
    EndLine, EndColumn
  ) VALUES (
    :FileURI, :HeaderURI,
    :HashLine, :HashColumn,
    :StartLine, :StartColumn,
    :EndLine, :EndColumn
  )
);

SQL(InsertExtend,
  INSERT OR IGNORE INTO extends VALUES(
    :ExtendSymbolID,
    :SymbolID,
    :Modifier,
    :InterfaceName,
    :PackageName
  )
);

SQL(MultiInsertExtendsHead,
  INSERT OR IGNORE INTO extends VALUES
);

SQL(MultiInsertExtendsValue,
  (
    ?,
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectExtends,
  SELECT extends.*,
         symbols.ID, symbols.Name, symbols.Modifier, symbols.Kind, symbols.CurModule, symbols.Codeosym, symbols.Deprecated, symbols.Syscap, symbols.Signature,
         completions.*
  FROM extends
  JOIN symbols ON extends.SymbolID = symbols.ID
  JOIN completions ON completions.SymbolID = extends.SymbolID
  Where extends.ExtendSymbolID = :ID AND symbols.Modifier IN (2,3,4)
);

SQL(SelectExtend,
 SELECT * FROM extends WHERE ExtendSymbolID = :ExtendSymbolID AND SymbolID = :SymbolID
);

SQL(InsertComment,
  INSERT OR IGNORE INTO comments VALUES(
    :SymbolID,
    :CommentStyle,
    :CommentKind,
    :Comment
  )
);

SQL(MultiInsertCommentsHead,
  INSERT OR IGNORE INTO comments VALUES
);

SQL(MultiInsertCommentsValue,
  (
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectComment,
 SELECT * FROM comments WHERE SymbolID = :SymbolID
);

SQL(InsertCrossSymbol,
  INSERT OR IGNORE INTO cross_symbols VALUES(
    :CODEPackageName,
    :SymbolID,
    :Name,
    :ContainerID,
    :ContainerName,
    :CrossType,
    :FileURI,
    :StartLine,
    :StartColumn,
    :EndLine,
    :EndColumn,
    :DeclareStartLine,
    :DeclareStartColumn,
    :DeclareEndLine,
    :DeclareEndColumn
  )
);

SQL(MultiInsertCrossSymbolsHead,
  INSERT OR IGNORE INTO cross_symbols VALUES
);

SQL(MultiInsertCrossSymbolsValue,
  (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
  )
);

SQL(SelectCrossSymbol,
 SELECT * FROM cross_symbols WHERE CODEPackageName = :CODEPackageName AND Name = :Name
);

SQL(SelectCrossSymbolByID,
 SELECT * FROM cross_symbols WHERE SymbolID = :SymbolID
);