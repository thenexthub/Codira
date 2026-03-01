### 本 cookbook 的主要目标是为主要支持的功能提供使用示例，帮助开发者快速开始使用 LibAbcKit API。

## 获取 API 实现

```cpp
enum AbckitApiVersion version = ABCKIT_VERSION_RELEASE_1_0_0;
auto impl = AbckitGetApiImpl(version);                 // 顶级 API，包含入口点
auto implI = AbckitGetInspectApiImpl(version);         // 语言无关的检查 API
auto implM = AbckitGetModifyApiImpl(version);          // 语言无关的修改 API
auto implArkI = AbckitGetArkTSInspectApiImpl(version); // 语言相关的检查 API
auto implArkM = AbckitGetArkTSModifyApiImpl(version);  // 语言相关的修改 API

auto implG = AbckitGetGraphApiImpl(version);           // 语言无关的图 API
auto dynG = AbckitGetIsaApiDynamicImpl(version);       // 语言相关的图 API
auto statG = AbckitGetIsaApiStaticImpl(version);       // 语言相关的图 API
```

在 .cpp 文件开头为每个 API 创建一个静态实例即可

## 错误处理

libabckit 中的 API 在执行后都会设置错误值。您可以使用 `impl->getLastError()` 获取它。如果没有错误，返回值将是 ABCKIT_STATUSNO_ERROR。
使用以下方式检查执行状态：

```cpp
assert(impl->getLastError() == ABCKIT_STATUSNO_ERROR);
```

## LibAbcKit 中的字符串

以下是 libabckit 中处理字符串的自定义函数的有用示例：

```cpp
AbckitString *str = implI->createString(file, "new_string", strlen("new_string"));

std::string AbckitStringToString(AbckitFile *file, AbckitString *str)
{
    std::size_t len = 0;
    implI->abckitStringToString(file, str, nullptr, &len);
    auto name = malloc(len + 1);
    implI->abckitStringToString(file, str, name, &len);
    std::string res {name};
    free(name);

    return res;
}
```

# 元数据

## 打开/关闭/写入

### _打开文件_

该接口是初始化的唯一入口，必须首先调用。

```cpp
AbckitFile *file = implI->openAbc(path_to_abc, path_to_abc_length);
```

文件处理完成后，调用 WriteAbc() 或 CloseFile() 函数，以防止内存泄漏。

### _写入/关闭_

```cpp
impl->writeAbc(file, path_to_output_file, path_to_output_file_length); // 保存更改并关闭
// 或者
impl->closeFile(file); // 关闭而不保存更改
```

## 遍历

libabckit 对象的层次结构：

1. 文件包含模块（"文件"是 ".abc 文件"，"模块"是"源代码文件"的抽象）
2. 模块包含：命名空间、顶级类和顶级函数
3. 命名空间包含：嵌套命名空间、类和函数
4. 类包含函数（方法）
5. 函数包含嵌套函数

<!-- file module  class  function -> introduce hierarchy -->

libabckit 中的所有枚举器都接受 libabckit 对象、用户数据的 void* 指针和回调 lambda：

```cpp
void XXXEnumerateYYY(AbckitCoreXXX* xxx, void *data, bool(*cb)(AbckitCoreYYY* yyy, void *data))
```

### _枚举类中的方法_

```cpp
// AbckitCoreClass *klass
implI->classEnumerateMethods(klass, data, [](AbckitCoreMethod *method, void *data) {
    return true;
});
```

### _收集模块中的所有顶级类_

```cpp
// AbckitCoreModule *module
std::vector<AbckitCoreClass *> classes;
implI->moduleEnumerateClasses(module, (void*)&classes, [](AbckitCoreClass *klass, void *data) {
    ((std::vector<AbckitCoreClass*>*)data)->emplace_back(klass);
    return true;
});
```

## 检查

### _获取方法的父类_

```cpp
// AbckitCoreMethod *method
AbckitCoreClass *klass = implI->functionGetParentClass(method);
```

### _获取值_

<!-- Test: LiteralGetU32_2 -->

```cpp
// AbckitLiteral *res
void GetValues(AbckitValue *u32_res, AbckitValue *double_res, AbckitValue *string_res) {
    uint32_t val = implI->literalGetU32(u32_res);
    double d = implI->valueGetDouble(double_res);
    AbckitString *s = implI->valueGetString(string_res);
}
```

### _获取模块名称_

```cpp
// AbckitCoreModule *mod
AbckitString *GetModuleName(AbckitCoreModule *mod) {
    return implI->moduleGetName(mod);
}
```

## 元数据语言 API

### _为 ArkTS 方法添加注解_

```cpp
void AddAnno(AbckitModifyContext *file, AbckitCoreMethod *method) {
    auto mod = implI->functionGetModule(method);

    // 查找名称为 "Anno" 的注解接口
    AbckitCoreAnnotationInterface *ai;
    implI->moduleEnumerateAnnotationInterfaces(mod, &ai, [](AbckitCoreAnnotationInterface *annoI, void *data) {
        auto ai1 = (AbckitCoreAnnotationInterface **)data;
        auto file = implI->annotationInterfaceGetInspectContext(annoI);
        auto str = implI->annotationInterfaceGetName(annoI);
        auto name = AbckitStringToString(file, str);
        if (name == "Anno") {
            (*ai1) = annoI;
        }
        return true;
    });

    // 为方法添加 "@Anno" 注解
    struct AbckitArktsAnnotationCreateParams annoCreateParams;
    annoCreateParams.ai = implArkI->coreAnnotationInterfaceToArkTsAnnotationInterface(ai);
    AbckitArktsAnnotation *anno = implArkM->functionAddAnnotation(file,
        implArkI->coreFunctionToArkTsFunction(method), &annoCreateParams);
}
```

### _转换 AbckitArtksXXX -> AbckitXXX / AbckitXXX -> AbckitArktsXXX_

```cpp
AbckitCoreModule *CastToAbcKit(AbckitArktsInspectApi implArkI, AbckitArktsModule *mod) {
    return implArkI->arkTsModuleToCoreModule(mod); }

AbckitArktsAnnotation *CastToArkTS(AbckitArktsInspectApi implArkI, AbckitCoreAnnotation *anno) {
    return implArkI->coreAnnotationToArkTsAnnotation(anno); }
```

## 修改

### _创建值_

```cpp
AbckitLiteral *res1 = implM->createLiteralString(file, "asdf");
AbckitLiteral *res2 = implM->createLiteralDouble(file, 1.0);
AbckitLiteral *res = implM->createLiteralU32(file, 1);
```

```cpp
AbckitValue *res_u = implM->createValueU1(file, true);
AbckitValue *res_d = implM->createValueDouble(file, 1.2);
```

### _获取值类型_

<!-- Test: ValueGetType_1 -->

```cpp
AbckitType *s_type = implI->valueGetType(res_s); // val_s->id == ABCKIT_TYPE_ID_STRING
AbckitType *u_type = implI->valueGetType(res_u); // val_u->id == ABCKIT_TYPE_ID_U1
AbckitType *d_type = implI->valueGetType(res_d); // val_d->id == ABCKIT_TYPE_ID_F32,
```

# 图

## _创建和销毁图_

```cpp
AbckitGraph *graph = implI->createGraphFromFunction(method);
// ...
impl->destroyGraph(graph);
```

## _图遍历_

```cpp
implG->gVisitBlocksRpo(ctxG, &bbs, [](AbckitBasicBlock *bb, void *data) {
    // 用户 lambda
});
```

```cpp
implG->bbVisitSuccBlocks(bb, &succBBs, [](AbckitBasicBlock *succBasicBlock, void *d) {
    // 用户 lambda
});
```

```cpp
implG->bbVisitPredBlocks(bb, &predBBs,
    [](AbckitBasicBlock *succBasicBlock, void *d) {
        // 用户 lambda
    });

// ...
```

### _收集图中的所有基本块_

```cpp
std::vector<AbckitBasicBlock *> bbs;
implG->gVisitBlocksRPO(ctxG, &bbs, [](AbckitBasicBlock *bb, void *data) {
    ((std::vector<AbckitBasicBlock *> *)data)->emplace_back(bb);
});
```

### _收集基本块中的所有指令_

```cpp
std::vector<AbckitInst *> insts;
for (auto *inst = implG->bbGetFirstInst(bb); inst != nullptr; inst = implG->iGetNext(inst)) {
    insts.emplace_back(inst);
}
```

### _收集块的后继_

```cpp
std::vector<AbckitBasicBlock *> succBBs;
implG->bbVisitSuccBlocks(bb, &succBBs,
    [](AbckitBasicBlock *succBasicBlock, void *d) {
        auto *succs = (std::vector<AbckitBasicBlock *> *)d;
        succs->emplace_back(succBasicBlock);
    });
```

## 图检查

### _获取方法_

```cpp
AbckitCoreMethod *method = implG->iGetFunction(curInst);
```

## 图修改

### _创建基本块_

```cpp
AbckitBasicBlock *empty = implG->bbCreateEmpty(ctxG);
```

### _使用常量创建指令_

```cpp
AbckitInst *new_inst = implG->gFindOrCreateConstantI64(ctxG, 1U);
```

### _连接和断开块_

<!-- Test: BBcreateEmptyBlock_1 -->

```cpp
auto *start = implG->gGetStartBasicBlock(ctxG);
auto *bb = implG->bbCreateEmpty(ctxG);

implG->bbInsertSuccBlock(start, bb, 0);
implG->bbDisconnectSuccBlock(start, 0);
```

### _插入指令_

```cpp
implG->bbAddInstBack(bb, some_inst_1);
implG->iInsertAfter(some_inst_1, some_inst_2);
```

## 图语言

### _创建指令_

```cpp
// 用于静态
AbckitInst *neg_inst = statG->iCreateNeg(ctxG, new_inst);
AbckitInst *add_inst = statG->iCreateAdd(ctxG, neg_inst, new_inst);
AbckitInst *ret = statG->iCreateReturnVoid(ctxG);

// 用于动态
AbckitInst *neg_inst = dynG->iCreateNeg(ctxG, new_inst);
AbckitInst *add_inst = dynG->iCreateAdd2(ctxG, neg_inst, new_inst);
AbckitInst *ret = dynG->iCreateReturnundefined(ctxG);
```

### _为 ArkTS 创建 'print("Hello")'_

```cpp
AbckitInst *str = dynG->iCreateLoadString(ctxG, implM->createString(file, "Hello", strlen("Hello")));
AbckitInst *print = dynG->iCreateTryldglobalbyname(ctxG, implM->createString(file, "print", strlen("print")));
AbckitInst *callArg = dynG->iCreateCallarg1(ctxG, print, str);
```

# 用户场景

### _枚举模块中方法的名称_

```cpp
std::vector<std::string> functionNames;

std::function<void(AbckitCoreFunction *)> cbFunc = [&](AbckitCoreFunction *f) {
    auto funcName = helpers::AbckitStringToString(implI->functionGetName(f));
    functionNames.emplace_back(funcName);
};

std::function<void(AbckitCoreClass *)> cbClass = [&](AbckitCoreClass *c) {
    implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
        return true;
    });
};

std::function<void(AbckitCoreNamespace *)> cbNamespace;
cbNamespace = [&](AbckitCoreNamespace *n) {
    implI->namespaceEnumerateNamespaces(n, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
        return true;
    });
    implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
        return true;
    });
    implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(f);
        return true;
    });
};

std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
    implI->moduleEnumerateNamespaces(m, &cbNamespace, [](AbckitCoreNamespace *n, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
        return true;
    });
    implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
        return true;
    });
    implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
        return true;
    });
};

implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
    (*reinterpret_cast<std::function<void(AbckitCoreModule *)> *>(cb))(m);
    return true;
});
```

### _收集前驱基本块_

```cpp
std::vector<AbckitBasicBlock *> predBBs;
implG->bbVisitPredBlocks(bb, &predBBs,
    [](AbckitBasicBlock *succBasicBlock, void *d) {
    auto *preds = (std::vector<AbckitBasicBlock *> *)d;
    preds->emplace_back(succBasicBlock);
    });
``` 
