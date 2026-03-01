/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "../Prologue.inc.sql"

SQL(
  PRAGMA encoding = 'UTF-8';
  PRAGMA recursive_triggers = ON;
  PRAGMA locking_mode = NORMAL;
  PRAGMA synchronous = OFF;
  PRAGMA journal_mode = WAL;
  PRAGMA temp_store = MEMORY;
  PRAGMA page_size = 4096;
  PRAGMA cache_size = 16384;
  PRAGMA mmap_size = 10737418240;
  PRAGMA analysis_limit = 1000;
)
