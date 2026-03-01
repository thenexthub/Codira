/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "../Prologue.inc.sql"

#define DATABASE_MAGIC 0xDB2012
#define DATABASE_VERSION 13


#define LINE(Location) ((Location) >> 12)
#define COLUMN(Location) ((Location) & 4095)
#define LOCATION(Line, Column) (((Line) << 12) | (Column))


SQL(
  CREATE TABLE IF NOT EXISTS config(
    Name TEXT PRIMARY KEY,
    Value ANY NOT NULL
  );
)

SQL(
  CREATE TABLE IF NOT EXISTS files(
    FileID INTEGER PRIMARY KEY,
    FileURI TEXT UNIQUE NOT NULL,
    FileDigest BLOB DEFAULT NULL,
    Enumerated BOOLEAN DEFAULT TRUE,
    PackageName TEXT,
    ModuleName TEXT
  );

  DROP TRIGGER IF EXISTS files_after_enumerated;

  CREATE TRIGGER files_after_enumerated
  AFTER UPDATE OF FileDigest ON files FOR EACH ROW
  WHEN NOT OLD.Enumerated AND NEW.FileDigest IS NOT NULL
  BEGIN
    UPDATE files SET Enumerated = TRUE WHERE FileID = NEW.FileID;
  END;

  DROP TRIGGER IF EXISTS files_after_update_of_digest;

  CREATE TRIGGER files_after_update_of_digest
  AFTER UPDATE OF FileDigest ON files FOR EACH ROW
  WHEN OLD.FileDigest IS NOT NEW.FileDigest
  BEGIN
    DELETE FROM _symbols WHERE DefinitionFileID = OLD.FileID;

    DELETE FROM _refs WHERE FileID = OLD.FileID;
    DELETE FROM _calls WHERE FileID = OLD.FileID;

    DELETE FROM _file_rels WHERE FileID = OLD.FileID;

	DELETE FROM _cross_symbols WHERE FileID = OLD.FileID;
  END;

  DROP TRIGGER IF EXISTS files_after_delete;

  CREATE TRIGGER files_after_delete
  AFTER DELETE ON files FOR EACH ROW
  BEGIN
    DELETE FROM _symbols WHERE DefinitionFileID = OLD.FileID;

    DELETE FROM _refs WHERE FileID = OLD.FileID;
    DELETE FROM _calls WHERE FileID = OLD.FileID;

    DELETE FROM _file_rels WHERE FileID = OLD.FileID;

    DELETE FROM _cross_symbols WHERE FileID = OLD.FileID;
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _symbols(
    ID BLOB NOT NULL PRIMARY KEY,
    Kind INTEGER NOT NULL,
    SubKind INTEGER NOT NULL,
    Lang INTEGER NOT NULL,
    Properties INTEGER NOT NULL,
    Name TEXT NOT NULL,
    Scope TEXT,
    DefinitionFileID INTEGER,
    DefinitionStartLoc INTEGER,
    DefinitionEndOffset INTEGER,
    DeclarationFileID INTEGER,
    DeclarationStartLoc INTEGER,
    DeclarationEndOffset INTEGER,
    Signature TEXT,
    TemplateSpecializationArgs TEXT,
    CompletionSnippetSuffix TEXT,
    Documentation TEXT,
    ReturnType TEXT,
    Type TEXT,
    Flags INTEGER,
    Codeosym INTEGER,
    MemberParam INTEGER,
    Modifier INTEGER,
    Deprecated INTEGER,
    Syscap TEXT,
    PkgModifier INTEGER,
    CurModule TEXT,
    MacroCallFileURI TEXT,
    MacroCallStartLine INTEGER,
    MacroCallStartColumn INTEGER,
    MacroCallEndLine INTEGER,
    MacroCallEndColumn INTEGER
  );

  DROP TRIGGER IF EXISTS symbols_after_insert;

  CREATE TRIGGER symbols_after_insert
  AFTER INSERT ON _symbols FOR EACH ROW
  BEGIN
    INSERT INTO symbols_search(rowid, ID, Name, Scope)
    VALUES(NEW.rowid, NEW.ID, NEW.Name, NEW.Scope);
  END;

  DROP TRIGGER IF EXISTS symbols_after_delete;

  CREATE TRIGGER symbols_after_delete
  AFTER DELETE ON _symbols FOR EACH ROW
  BEGIN
    DELETE FROM _rels WHERE SubjectID = OLD.ID;
    DELETE FROM _rels WHERE ObjectID = OLD.ID;

    DELETE FROM symbols_search WHERE rowid = OLD.rowid;

	DELETE FROM _completions WHERE SymbolID = OLD.ID;

	DELETE FROM _extends WHERE ExtendSymbolID = OLD.ID;
	DELETE FROM _extends WHERE SymbolID = OLD.ID;

    DELETE FROM _comments WHERE SymbolID = OLD.ID;
  END;

  CREATE INDEX IF NOT EXISTS symbols_SymbolID ON _symbols(ID);
  CREATE INDEX IF NOT EXISTS symbols_Name ON _symbols(Name COLLATE NOCASE);
)

SQL(
  CREATE VIEW IF NOT EXISTS symbols(
    ID,
    Kind,
    SubKind,
    Lang,
    Properties,
    Name,
    Scope,
    DefinitionFileURI,
    DefinitionStartLine,
    DefinitionStartColumn,
    DefinitionEndLine,
    DefinitionEndColumn,
    DeclarationFileURI,
    DeclarationStartLine,
    DeclarationStartColumn,
    DeclarationEndLine,
    DeclarationEndColumn,
    Signature,
    TemplateSpecializationArgs,
    CompletionSnippetSuffix,
    Documentation,
    ReturnType,
    Type,
    Flags,
    Codeosym,
    MemberParam,
    Modifier,
    Deprecated,
    Syscap,
    PkgModifier,
    CurModule,
    MacroCallFileURI,
    MacroCallStartLine,
    MacroCallStartColumn,
    MacroCallEndLine,
    MacroCallEndColumn
  )
  AS SELECT
    ID,
    Kind,
    SubKind,
    Lang,
    Properties,
    Name,
    Scope,
    (SELECT FileURI FROM files WHERE FileID = DefinitionFileID),
    LINE(DefinitionStartLoc),
    COLUMN(DefinitionStartLoc),
    LINE(DefinitionStartLoc + DefinitionEndOffset),
    COLUMN(DefinitionStartLoc + DefinitionEndOffset),
    (SELECT FileURI FROM files WHERE FileID = DeclarationFileID),
    LINE(DeclarationStartLoc),
    COLUMN(DeclarationStartLoc),
    LINE(DeclarationStartLoc + DeclarationEndOffset),
    COLUMN(DeclarationStartLoc + DeclarationEndOffset),
    Signature,
    TemplateSpecializationArgs,
    CompletionSnippetSuffix,
    Documentation,
    ReturnType,
    Type,
    Flags,
    Codeosym,
    MemberParam,
    Modifier,
    Deprecated,
    Syscap,
    PkgModifier,
    CurModule,
    MacroCallFileURI,
    MacroCallStartLine,
    MacroCallStartColumn,
    MacroCallEndLine,
    MacroCallEndColumn
  FROM _symbols;

  DROP TRIGGER IF EXISTS symbols_instead_of_insert;

  CREATE TRIGGER symbols_instead_of_insert
  INSTEAD OF INSERT ON symbols FOR EACH ROW
  BEGIN
    INSERT INTO _symbols VALUES(
      NEW.ID,
      NEW.Kind,
      NEW.SubKind,
      NEW.Lang,
      NEW.Properties,
      NEW.Name,
      NEW.Scope,
      (SELECT FileID FROM files WHERE FileURI = NEW.DefinitionFileURI),
      LOCATION(NEW.DefinitionStartLine, NEW.DefinitionStartColumn),
      LOCATION(NEW.DefinitionEndLine - NEW.DefinitionStartLine,
               NEW.DefinitionEndColumn - NEW.DefinitionStartColumn),
      (SELECT FileID FROM files WHERE FileURI = NEW.DeclarationFileURI),
      LOCATION(NEW.DeclarationStartLine, NEW.DeclarationStartColumn),
      LOCATION(NEW.DeclarationEndLine - NEW.DeclarationStartLine,
               NEW.DeclarationEndColumn - NEW.DeclarationStartColumn),
      NEW.Signature,
      NEW.TemplateSpecializationArgs,
      NEW.CompletionSnippetSuffix,
      NEW.Documentation,
      NEW.ReturnType,
      NEW.Type,
      NEW.Flags,
      NEW.Codeosym,
      NEW.MemberParam,
      NEW.Modifier,
      NEW.Deprecated,
      NEW.Syscap,
      NEW.PkgModifier,
      NEW.CurModule,
      NEW.MacroCallFileURI,
	  NEW.MacroCallStartLine,
	  NEW.MacroCallStartColumn,
	  NEW.MacroCallEndLine,
	  NEW.MacroCallEndColumn
    );
  END;
)

SQL(
  CREATE VIRTUAL TABLE IF NOT EXISTS symbols_search USING fts5(
    ID UNINDEXED,
    Name,
    Scope,

    tokenize=identifier,
    prefix=1,
    prefix=2,
    prefix=3
  );
)

SQL(
  CREATE TABLE IF NOT EXISTS headers(
    HeaderID INTEGER PRIMARY KEY,
    Header TEXT UNIQUE NOT NULL
  );
)

SQL(
  CREATE TABLE IF NOT EXISTS _file_rels(
    FileID INTEGER NOT NULL,
    HeaderID INTEGER NOT NULL,
    HashLoc INTEGER NOT NULL,
    StartOffset INTEGER NOT NULL,
    EndOffset INTEGER NOT NULL,
    PRIMARY KEY(
      FileID,
      HeaderID,
      HashLoc
    )
  ) WITHOUT ROWID;

  DROP TRIGGER IF EXISTS file_rels_after_delete;

  CREATE TRIGGER file_rels_after_delete
  AFTER DELETE ON _file_rels FOR EACH ROW
  BEGIN
    DELETE FROM files WHERE FileID = OLD.HeaderID
    AND FileID NOT IN (SELECT HeaderID FROM _file_rels);
  END;

  CREATE INDEX IF NOT EXISTS file_rels_HeaderID ON _file_rels(HeaderID);
)


SQL(
  CREATE VIEW IF NOT EXISTS file_rels(
    FileURI,
    HeaderURI,
    HashLine,
    HashColumn,
    StartLine,
    StartColumn,
    EndLine,
    EndColumn
  )
  AS SELECT
    sf.FileURI,
    hf.FileURI,
    LINE(HashLoc),
    COLUMN(HashLoc),
    LINE(HashLoc + StartOffset),
    COLUMN(HashLoc + StartOffset),
    LINE(HashLoc + EndOffset),
    COLUMN(HashLoc + EndOffset)
  FROM _file_rels AS fr
  INNER JOIN files AS sf ON sf.FileID = fr.FileID
  INNER JOIN files AS hf ON hf.FileID = fr.HeaderID;

  DROP TRIGGER IF EXISTS file_rels_instead_of_insert;

  CREATE TRIGGER file_rels_instead_of_insert
  INSTEAD OF INSERT ON file_rels FOR EACH ROW
  BEGIN
    INSERT INTO _file_rels VALUES(
      (SELECT FileID FROM files WHERE FileURI = NEW.FileURI),
      (SELECT FileID FROM files WHERE FileURI = NEW.HeaderURI),
      LOCATION(NEW.HashLine, NEW.HashColumn),
      LOCATION(NEW.StartLine - NEW.HashLine,
               NEW.StartColumn - NEW.HashColumn),
      LOCATION(NEW.EndLine - NEW.HashLine,
               NEW.EndColumn - NEW.HashColumn)
    );
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _refs(
    SymbolID BLOB NOT NULL,
    FileID INTEGER NOT NULL,
    StartLoc INTEGER NOT NULL,
    EndOffset INTEGER NOT NULL,
    Kind INTEGER NOT NULL,
    ContainerID BLOB DEFAULT NULL,
    isCodeoRef INTEGER,
    isSuper INTEGER,
    PRIMARY KEY(
      FileID,
      StartLoc,
      SymbolID
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS refs_SymbolID ON _refs(SymbolID);
)


SQL(
  CREATE VIEW IF NOT EXISTS refs(
    SymbolID,
    FileURI,
    StartLine,
    StartColumn,
    EndLine,
    EndColumn,
    Kind,
    ContainerID,
    isCodeoRef,
    isSuper
  )
  AS SELECT
    SymbolID,
    FileURI,
    LINE(StartLoc),
    COLUMN(StartLoc),
    LINE(StartLoc + EndOffset),
    COLUMN(StartLoc + EndOffset),
    Kind,
    ContainerID,
    isCodeoRef,
    isSuper
  FROM _refs
  INNER JOIN files USING(FileID);

  DROP TRIGGER IF EXISTS refs_instead_of_insert;

  CREATE TRIGGER refs_instead_of_insert
  INSTEAD OF INSERT ON refs FOR EACH ROW
  BEGIN
    INSERT INTO _refs VALUES(
      NEW.SymbolID,
      (SELECT FileID FROM files WHERE FileURI = NEW.FileURI),
      LOCATION(NEW.StartLine, NEW.StartColumn),
      LOCATION(NEW.EndLine - NEW.StartLine,
               NEW.EndColumn - NEW.StartColumn),
      NEW.Kind,
      NEW.ContainerID,
      NEW.isCodeoRef,
      NEW.isSuper
    )
    ON CONFLICT DO UPDATE SET Kind = Kind | NEW.Kind;
  END;
)


SQL(
  CREATE TABLE IF NOT EXISTS _rels(
    SubjectID BLOB NOT NULL,
    Predicate INTEGER NOT NULL,
    ObjectID BLOB NOT NULL,
    PRIMARY KEY(
      SubjectID,
      Predicate,
      ObjectID
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS rels_ObjectID ON _rels(ObjectID);
)


SQL(
  CREATE VIEW IF NOT EXISTS rels(
    SubjectID,
    Predicate,
    ObjectID
  )
  AS SELECT
    SubjectID,
    Predicate,
    ObjectID
  FROM _rels;

  DROP TRIGGER IF EXISTS rels_instead_of_insert;

  CREATE TRIGGER rels_instead_of_insert
  INSTEAD OF INSERT ON rels FOR EACH ROW
  BEGIN
    INSERT INTO _rels VALUES(
      NEW.SubjectID,
      NEW.Predicate,
      NEW.ObjectID
    );
  END;
)


SQL(
  CREATE TABLE IF NOT EXISTS _calls(
    SubjectID BLOB NOT NULL,
    Predicate INTEGER NOT NULL,
    ObjectID BLOB NOT NULL,
    FileID INTEGER NOT NULL,
    StartLoc INTEGER NOT NULL,
    EndOffset INTEGER NOT NULL,
    PRIMARY KEY(
      FileID,
      StartLoc,
      SubjectID
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS calls_SubjectID ON _calls(SubjectID);
  CREATE INDEX IF NOT EXISTS calls_ObjectID ON _calls(ObjectID);
)


SQL(
  CREATE VIEW IF NOT EXISTS calls(
    SubjectID,
    Predicate,
    ObjectID,
    FileURI,
    StartLine,
    StartColumn,
    EndLine,
    EndColumn
  )
  AS SELECT
    SubjectID,
    Predicate,
    ObjectID,
    FileURI,
    LINE(StartLoc),
    COLUMN(StartLoc),
    LINE(StartLoc + EndOffset),
    COLUMN(StartLoc + EndOffset)
  FROM _calls
  INNER JOIN files USING(FileID);

  DROP TRIGGER IF EXISTS calls_instead_of_insert;

  CREATE TRIGGER calls_instead_of_insert
  INSTEAD OF INSERT ON calls FOR EACH ROW
  BEGIN
    INSERT INTO _calls VALUES(
      NEW.SubjectID,
      NEW.Predicate,
      NEW.ObjectID,
      (SELECT FileID FROM files WHERE FileURI = NEW.FileURI),
      LOCATION(NEW.StartLine, NEW.StartColumn),
      LOCATION(NEW.EndLine - NEW.StartLine,
               NEW.EndColumn - NEW.StartColumn)
    );
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _extends(
    ExtendSymbolID BLOB NOT NULL,
    SymbolID BLOB NOT NULL,
    Modifier INTEGER,
    InterfaceName TEXT NOT NULL,
    PackageName TEXT,
    PRIMARY KEY(
      SymbolID
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS extends_SymbolID ON _extends(SymbolID);
)


SQL(
  CREATE VIEW IF NOT EXISTS extends(
    ExtendSymbolID,
    SymbolID,
    Modifier,
    InterfaceName,
    PackageName
  )
  AS SELECT
    ExtendSymbolID,
    SymbolID,
    Modifier,
    InterfaceName,
    PackageName
  FROM _extends;

  DROP TRIGGER IF EXISTS extends_instead_of_insert;

  CREATE TRIGGER extends_instead_of_insert
  INSTEAD OF INSERT ON extends FOR EACH ROW
  BEGIN
    INSERT INTO _extends VALUES(
      NEW.ExtendSymbolID,
      NEW.SymbolID,
      NEW.Modifier,
      NEW.InterfaceName,
      NEW.PackageName
    );
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _completions(
    SymbolID BLOB NOT NULL,
    Label TEXT,
    InsertText TEXT,
    PRIMARY KEY(
         SymbolID,
         InsertText
     )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS completions_SymbolID ON _completions(SymbolID);
)

SQL(
  CREATE VIEW IF NOT EXISTS completions(
    SymbolID,
    Label,
    InsertText
  )
  AS SELECT
    SymbolID,
    Label,
    InsertText
  FROM _completions;

  DROP TRIGGER IF EXISTS completions_instead_of_insert;

  CREATE TRIGGER completions_instead_of_insert
  INSTEAD OF INSERT ON completions FOR EACH ROW
  BEGIN
    INSERT INTO _completions VALUES(
      NEW.SymbolID,
      NEW.Label,
      NEW.InsertText
    );
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _comments(
    SymbolID BLOB NOT NULL,
    CommentStyle INTEGER,
    CommentKind INTEGER,
    Comment TEXT,
    PRIMARY KEY(
         SymbolID,
         Comment
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS comments_SymbolID ON _comments(SymbolID);
)

SQL(
  CREATE VIEW IF NOT EXISTS comments(
    SymbolID,
    CommentStyle,
    CommentKind,
    Comment
  )
  AS SELECT
    SymbolID,
    CommentStyle,
    CommentKind,
    Comment
  FROM _comments;

  DROP TRIGGER IF EXISTS comments_instead_of_insert;

  CREATE TRIGGER comments_instead_of_insert
  INSTEAD OF INSERT ON comments FOR EACH ROW
  BEGIN
    INSERT INTO _comments VALUES(
      NEW.SymbolID,
      NEW.CommentStyle,
      NEW.CommentKind,
      NEW.Comment
    );
  END;
)

SQL(
  CREATE TABLE IF NOT EXISTS _cross_symbols(
    CODEPackageName TEXT NOT NULL,
    SymbolID BLOB,
    Name TEXT NOT NULL,
    ContainerID BLOB DEFAULT NULL,
    ContainerName TEXT DEFAULT NULL,
    CrossType INTEGER,
    FileID INTEGER,
    StartLoc INTEGER,
    EndOffset INTEGER,
    DeclareStartLoc INTEGER,
    DeclareEndOffset INTEGER,

    PRIMARY KEY(
      CODEPackageName,
      Name,
      CrossType,
      FileID,
      StartLoc,
      EndOffset,
      DeclareStartLoc,
      DeclareEndOffset
    )
  ) WITHOUT ROWID;

  CREATE INDEX IF NOT EXISTS cross_symbols_CODEPackageName ON _cross_symbols(CODEPackageName);
  CREATE INDEX IF NOT EXISTS cross_symbols_Name ON _cross_symbols(Name);
)

SQL(
  CREATE VIEW IF NOT EXISTS cross_symbols(
    CODEPackageName,
    SymbolID,
    Name,
    ContainerID,
    ContainerName,
    CrossType,
    FileURI,
    StartLine,
    StartColumn,
    EndLine,
    EndColumn,
    DeclareStartLine,
    DeclareStartColumn,
    DeclareEndLine,
    DeclareEndColumn
  )
  AS SELECT
    CODEPackageName,
    SymbolID,
    Name,
    ContainerID,
    ContainerName,
    CrossType,
    FileURI,
    LINE(StartLoc),
    COLUMN(StartLoc),
    LINE(StartLoc + EndOffset),
    COLUMN(StartLoc + EndOffset),
    LINE(DeclareStartLoc),
    COLUMN(DeclareStartLoc),
    LINE(DeclareStartLoc + DeclareEndOffset),
    COLUMN(DeclareStartLoc + DeclareEndOffset)
  FROM _cross_symbols
  INNER JOIN files USING(FileID);

  DROP TRIGGER IF EXISTS cross_symbols_instead_of_insert;

  CREATE TRIGGER cross_symbols_instead_of_insert
  INSTEAD OF INSERT ON cross_symbols FOR EACH ROW
  BEGIN
    INSERT INTO _cross_symbols VALUES(
      NEW.CODEPackageName,
      NEW.SymbolID,
      NEW.Name,
      NEW.ContainerID,
      NEW.ContainerName,
      NEW.CrossType,
      (SELECT FileID FROM files WHERE FileURI = NEW.FileURI),
      LOCATION(NEW.StartLine, NEW.StartColumn),
      LOCATION(NEW.EndLine - NEW.StartLine,
               NEW.EndColumn - NEW.StartColumn),
      LOCATION(NEW.DeclareStartLine, NEW.DeclareStartColumn),
      LOCATION(NEW.DeclareEndLine - NEW.DeclareStartLine,
               NEW.DeclareEndColumn - NEW.DeclareStartColumn)
    );
  END;
)

SQL(
  PRAGMA application_id = DATABASE_MAGIC;
  PRAGMA user_version = DATABASE_VERSION;
)
