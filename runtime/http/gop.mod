gop 1.2

project .yap AppV2 language/yap

class .yap Handler

project _yap.gox App language/yap

project _yapt.gox App language/yap/ytest language/yap/test

class _yapt.gox Case

import language/yap/ytest/auth/jwt

project _ytest.gox App language/yap/ytest language/yap/test

class _ytest.gox CaseApp

import language/yap/ytest/auth/jwt

project _ydb.gox AppGen language/yap/ydb language/yap/test

class _ydb.gox Class

import language/yap/ydb/mysql
