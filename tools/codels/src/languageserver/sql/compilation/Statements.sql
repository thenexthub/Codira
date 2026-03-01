/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "sql/Prologue.inc.sql"


SQL(InsertCommandFile,
  INSERT INTO "cmdfile"("FilePath") VALUES(:FilePath) RETURNING "CmdID"
);

SQL(SelectCommandFiles, SELECT "FilePath" FROM "cmdfile");

SQL(InsertCompileCommand,
  INSERT INTO "cmd" VALUES(:CmdID, :Filename, :Directory, :Output)
);

SQL(SelectCompileCommands,
  SELECT "Filename", "Directory", "Output", "CmdID" FROM "cmd"
);

SQL(SelectCompileCommand,
  SELECT "Filename", "Directory", "Output", "CmdID" FROM "cmd"
  INNER JOIN "cmdfile" USING("CmdID") WHERE "FilePath" = :FilePath
);

SQL(InsertArgumentText,
  INSERT INTO "arg"("ArgText") VALUES(:ArgText) RETURNING "ArgID"
);

SQL(InsertCommandArgument,
  INSERT INTO "cmdarg" VALUES(:CmdID, :ArgID, :ArgNo)
);

SQL(SelectCommandArguments,
  SELECT "ArgText" FROM "cmdarg" INNER JOIN "arg" USING("ArgID")
  WHERE "CmdID" = :CmdID ORDER BY "ArgNo";
);
