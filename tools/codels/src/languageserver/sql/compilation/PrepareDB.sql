/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "sql/Prologue.inc.sql"

SQL(
  PRAGMA encoding = 'UTF-8';
  PRAGMA synchronous = OFF;
  PRAGMA journal_mode = DELETE;
  PRAGMA temp_store = FILE;
  PRAGMA mmap_size = 1073741824;
  PRAGMA page_size = 4096;
  PRAGMA cache_size = 4096;
)
