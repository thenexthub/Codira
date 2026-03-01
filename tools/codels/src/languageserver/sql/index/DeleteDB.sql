/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
* This source file is part of the Codira project, licensed under Apache-2.0
* with Runtime Library Exception.
*
* See https://cangjie-lang.cn/pages/LICENSE for license information.
*/

#include "../Prologue.inc.sql"

SQL(DROP VIEW IF EXISTS refs)
SQL(DROP VIEW IF EXISTS rels)
SQL(DROP VIEW IF EXISTS calls)
SQL(DROP VIEW IF EXISTS includes)
SQL(DROP VIEW IF EXISTS symbols)
SQL(DROP VIEW IF EXISTS file_rels)
SQL(DROP VIEW IF EXISTS extends)
SQL(DROP VIEW IF EXISTS cross_symbols)

SQL(DROP TABLE IF EXISTS _refs)
SQL(DROP TABLE IF EXISTS _rels)
SQL(DROP TABLE IF EXISTS _calls)
SQL(DROP TABLE IF EXISTS _includes)
SQL(DROP TABLE IF EXISTS _symbols)
SQL(DROP TABLE IF EXISTS _file_rels)
SQL(DROP TABLE IF EXISTS _extends)
SQL(DROP TABLE IF EXISTS _cross_symbols)

SQL(DROP TABLE IF EXISTS files)
SQL(DROP TABLE IF EXISTS headers)
SQL(DROP TABLE IF EXISTS config)
