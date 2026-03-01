/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "sql/Prologue.inc.sql"


SQL(
  CREATE INDEX "idx_cmdfile" ON "cmdfile"("FilePath");
  CREATE INDEX "idx_cmdarg" ON "cmdarg"("CmdID", "ArgNo");
)
