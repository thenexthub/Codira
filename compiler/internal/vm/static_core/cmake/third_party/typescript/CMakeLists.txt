# Copyright (c) 2024-2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10)

project(typescript)

set(SOURCES
  ${TYPESCRIPT_ROOT}/lib/.gitattributes
  ${TYPESCRIPT_ROOT}/lib/README.md
  ${TYPESCRIPT_ROOT}/lib/cancellationToken.js
  ${TYPESCRIPT_ROOT}/lib/cs/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/de/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/dynamicImportCompat.js
  ${TYPESCRIPT_ROOT}/lib/es/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/fr/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/it/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/ja/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/ko/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/lib.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.dom.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.dom.iterable.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.collection.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.core.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.generator.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.iterable.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.promise.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.proxy.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.reflect.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.symbol.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2015.symbol.wellknown.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2016.array.include.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2016.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2016.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.object.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.sharedmemory.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2017.typedarrays.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.asyncgenerator.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.asynciterable.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.promise.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2018.regexp.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.array.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.object.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2019.symbol.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.bigint.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.date.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.number.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.promise.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.sharedmemory.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2020.symbol.wellknown.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.promise.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2021.weakref.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.array.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.error.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.object.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.sharedmemory.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es2022.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es5.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.es6.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.full.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.intl.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.promise.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.string.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.esnext.weakref.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.scripthost.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.webworker.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.webworker.importscripts.d.ts
  ${TYPESCRIPT_ROOT}/lib/lib.webworker.iterable.d.ts
  ${TYPESCRIPT_ROOT}/lib/pl/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/protocol.d.ts
  ${TYPESCRIPT_ROOT}/lib/pt-br/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/ru/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/tr/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/tsc.js
  ${TYPESCRIPT_ROOT}/lib/tsserver.js
  ${TYPESCRIPT_ROOT}/lib/tsserverlibrary.d.ts
  ${TYPESCRIPT_ROOT}/lib/tsserverlibrary.js
  ${TYPESCRIPT_ROOT}/lib/typesMap.json
  ${TYPESCRIPT_ROOT}/lib/typescript.d.ts
  ${TYPESCRIPT_ROOT}/lib/typescript.js
  ${TYPESCRIPT_ROOT}/lib/typescriptServices.d.ts
  ${TYPESCRIPT_ROOT}/lib/typescriptServices.js
  ${TYPESCRIPT_ROOT}/lib/typingsInstaller.js
  ${TYPESCRIPT_ROOT}/lib/watchGuard.js
  ${TYPESCRIPT_ROOT}/lib/zh-cn/diagnosticMessages.generated.json
  ${TYPESCRIPT_ROOT}/lib/zh-tw/diagnosticMessages.generated.json
)

add_custom_command(
    OUTPUT ${TYPESCRIPT_ROOT}/bin/tsc
    COMMENT "Compile typescript"
    COMMAND ${TYPESCRIPT_ROOT}/compile_typescript.py ${TYPESCRIPT_ROOT}
    DEPENDS ${SOURCES}
)
add_custom_target(compile_typescript
    DEPENDS ${TYPESCRIPT_ROOT}/bin/tsc
)
