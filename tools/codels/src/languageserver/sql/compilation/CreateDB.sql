/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "sql/Prologue.inc.sql"


SQL(
  CREATE TABLE "cmdfile"(
    "CmdID" INTEGER PRIMARY KEY,
    "FilePath" TEXT NOT NULL
  );
)


SQL(
  CREATE TABLE "cmd"(
    "CmdID" INTEGER PRIMARY KEY,
    "Filename" TEXT NOT NULL,
    "Directory" TEXT NOT NULL,
    "Output" TEXT NOT NULL
  );
)


SQL(
  CREATE TABLE "arg"(
    "ArgID" INTEGER PRIMARY KEY,
    "ArgText" TEXT NOT NULL
  );
)


SQL(
  CREATE TABLE "cmdarg"(
    "CmdID" INTEGER NOT NULL,
    "ArgID" INTEGER NOT NULL,
    "ArgNo" INTEGER NOT NULL
  );
)
