/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <gtest/gtest.h>

#include <filesystem>

#include "obfuscator/obfuscator.h"
#include "test_util/execute_util.h"
#include "configuration/configuration_parser.h"
#include "configuration/keep_option_parser.h"
#include "util/string_util.h"

using namespace testing::ext;

namespace {
using ElementVariant =
    std::variant<abckit_wrapper::Module *, abckit_wrapper::Namespace *, abckit_wrapper::Class *,
                 abckit_wrapper::Method *, abckit_wrapper::Field *, abckit_wrapper::AnnotationInterface *>;

struct NameGetter {
    std::string operator()(abckit_wrapper::Module *module) const
    {
        return module->GetFullyQualifiedName();
    }

    std::string operator()(abckit_wrapper::Namespace *ns) const
    {
        return ns->GetObjectName();
    }

    std::string operator()(abckit_wrapper::Class *cls) const
    {
        return cls->GetObjectName();
    }

    std::string operator()(abckit_wrapper::Method *method) const
    {
        return method->GetRawName();
    }

    std::string operator()(abckit_wrapper::Field *field) const
    {
        return field->GetRawName();
    }

    std::string operator()(abckit_wrapper::AnnotationInterface *ai) const
    {
        return ai->GetObjectName();
    }
};
}  // namespace

/**
 * @brief ObfuscatorTest
 * By default, only the consistency of objects before and after confusion is verified, and abc will not be run.
 * If you want to verify that the running results before and after obfuscation are consistent, please call the
 * InitRunAbcConfig method before calling the obfuscation method
 */
class ObfuscatorTest : public ::testing::Test {
public:
    void SetUp() override
    {
        obfAbcPath_.clear();
        nameCachePath_.clear();
        elements_.clear();
        removeLog_ = false;
    }

    void TearDown() override {}

    void Init(const std::string &abcPath, const std::string &obfAbcPath, const std::string &nameCachePath,
              bool removeLog = false)
    {
        abcPath_ = abcPath;
        obfAbcPath_ = obfAbcPath;
        nameCachePath_ = nameCachePath;
        removeLog_ = removeLog;

        config_.defaultNameCachePath = nameCachePath_;
        config_.obfuscationRules.fileNameOption.enable = true;
        config_.obfuscationRules.removeLog = removeLog_;

        std::filesystem::remove(obfAbcPath_);
        std::filesystem::remove(nameCachePath_);

        ASSERT_EQ(fileView_.Init(abcPath), AbckitWrapperErrorCode::ERR_SUCCESS);
    }

    // funcName actual is the function full name, If value is "main", the signature will be automatically concatenated,
    // while in other scenarios, a complete signature needs to be passed in.
    void InitRunAbcConfig(const std::string &moduleName, const std::string &funcName)
    {
        const auto module = fileView_.GetModule(moduleName);
        ASSERT_TRUE(module.has_value()) << "module not found:" << moduleName;
        module_ = module.value();

        std::string funcFullName;
        if (funcName == "main") {
            funcFullName.append(moduleName).append(".").append(funcName).append(":void;");
        } else {
            funcFullName = funcName;
        }
        const auto method = fileView_.Get<abckit_wrapper::Method>(funcFullName);
        ASSERT_TRUE(method.has_value()) << "method not found:" << funcFullName;
        method_ = method.value();
    }

#ifndef PANDA_TARGET_WINDOWS
    void VerifyAbcRunResult(const std::string &oriModuleName, const std::string &oriMethodName) const
    {
        const auto oriOutput = ark::guard::ExecuteUtil::ExecuteStaticAbc(abcPath_, oriModuleName, oriMethodName);
        ASSERT_FALSE(oriOutput.empty());

        const auto obfModuleName = module_->GetName();
        const auto obfMethodName = method_->GetRawName();
        const auto obfOutput = ark::guard::ExecuteUtil::ExecuteStaticAbc(obfAbcPath_, obfModuleName, obfMethodName);
        if (removeLog_) {
            ASSERT_TRUE(obfOutput.empty());
            ASSERT_NE(oriOutput, obfOutput);
        } else {
            ASSERT_FALSE(obfOutput.empty());
            ASSERT_EQ(oriOutput, obfOutput);
        }
    }
#endif

    void VerifyObfuscated()
    {
        VerifyOriginalNamesMatch();
#ifndef PANDA_TARGET_WINDOWS
        std::string oriModuleName;
        std::string oriMethodName;
        if (module_ && method_) {
            oriModuleName = module_->GetName();
            oriMethodName = method_->GetRawName();
        }
#endif
        ExecuteObfuscator();
        VerifyObfuscatedFilesExist();
        VerifyObfuscatedNamesChanged();
        VerifyObfuscatedKept();
#ifndef PANDA_TARGET_WINDOWS
        if (module_ && method_) {
            VerifyAbcRunResult(oriModuleName, oriMethodName);
        }
#endif
    }

    void VerifyOriginalNamesMatch()
    {
        NameGetter nameGetter;
        for (const auto &[elem, expectedName] : elements_) {
            std::string currentName = std::visit(nameGetter, elem);
            ASSERT_EQ(currentName, expectedName) << "Initial name check failed";
        }
    }

    void ExecuteObfuscator()
    {
        ark::guard::Obfuscator obfuscator(config_);
        ASSERT_TRUE(obfuscator.Execute(fileView_));
    }

    void VerifyObfuscatedFilesExist() const
    {
        fileView_.Save(obfAbcPath_);
        ASSERT_TRUE(std::filesystem::exists(obfAbcPath_)) << obfAbcPath_ << ", obf abc file not exist after obfuscated";
        ASSERT_TRUE(std::filesystem::exists(nameCachePath_))
            << nameCachePath_ << ", cache file not exist after obfuscated";
    }

    void VerifyObfuscatedNamesChanged()
    {
        NameGetter nameGetter;
        for (const auto &[elem, expectedName] : elements_) {
            std::string currentName = std::visit(nameGetter, elem);
            ASSERT_NE(currentName, expectedName) << "Obfuscation name check failed";
        }
    }

    void VerifyObfuscatedKept()
    {
        NameGetter nameGetter;
        for (const auto &[elem, expectedKeptPart] : keptElements_) {
            std::string currentName = std::visit(nameGetter, elem);
            ASSERT_TRUE(ark::guard::StringUtil::IsSubStrMatched(currentName, expectedKeptPart))
                << "Obfuscation name keep check failed";
        }
    }

    template <typename T>
    void AddElement(const std::string &fullName, const std::string &rawName)
    {
        auto object = fileView_.Get<T>(fullName);
        ASSERT_TRUE(object.has_value()) << "object not found:" << fullName;
        elements_.emplace_back(object.value(), rawName);
    }

    void AddModuleElement(const std::string &fullName, const std::string &rawName)
    {
        auto object = fileView_.GetModule(fullName);
        ASSERT_TRUE(object.has_value()) << "module not found:" << fullName;
        elements_.emplace_back(object.value(), rawName);
    }

    void AddKeptModuleElement(const std::string &fullName, const std::string &keptPart)
    {
        auto object = fileView_.GetModule(fullName);
        ASSERT_TRUE(object.has_value()) << "module not found:" << fullName;
        keptElements_.emplace_back(object.value(), keptPart);
    }

    void AddFilePathTestModified()
    {
        AddElement<abckit_wrapper::Method>("entry.A.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.A.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.A1.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.A1.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.dir.bar.A.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.dir.bar.A.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.dir.bar.foo.A.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.dir.bar.foo.A.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.dir.foo.A.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.dir.foo.A.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.dir.foo.B.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.dir.foo.B.m1", "m1");
        AddElement<abckit_wrapper::Method>("entry.foo.A.foo1:i32;i32;void;", "foo1");
        AddElement<abckit_wrapper::Field>("entry.foo.A.m1", "m1");
    }

    void SetConfigFile(const string &path, const bool removeLogOverride = true)
    {
        ark::guard::ConfigurationParser parser(path);
        parser.Parse(config_);
        config_.defaultNameCachePath = nameCachePath_;
        if (removeLogOverride) {
            config_.obfuscationRules.removeLog = removeLog_;
        }
    }

    void AddKeepOptions(const ark::guard::ClassSpecification &option)
    {
        config_.obfuscationRules.keepOptions.classSpecifications.emplace_back(option);
    }

    void ClearKeepOptions()
    {
        config_.obfuscationRules.keepOptions.classSpecifications.clear();
    }

    abckit_wrapper::FileView fileView_;

    std::string abcPath_;

    std::string obfAbcPath_;

    std::string nameCachePath_;

    bool removeLog_ = false;

    ark::guard::Configuration config_ {};

    abckit_wrapper::Module *module_ = nullptr;

    abckit_wrapper::Method *method_ = nullptr;

    std::vector<std::pair<ElementVariant, std::string>> elements_ {};

    std::vector<std::pair<ElementVariant, std::string>> keptElements_ {};
};

// obfuscator test with graph begin
// If obfuscator involves graph operations, please add the use case to this label interval to avoid crashes caused
// by runtime memory leaks

/**
 * @tc.name: obfuscator_test_001
 * @tc.desc: test remove log
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_001, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/remove_log_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath, true);

    this->VerifyObfuscated();
}

// obfuscator test with graph end

// config obf test begin
// these test use the same abc file to test config file obf result
/*
 * @tc.name: obfuscator_test_101
 * @tc.desc: test obf with reservedFileNames options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_101, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.updated.01.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.01.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("entry.main", "main");

    std::string configFilePath = GUARD_UNIT_TEST_DIR "projects/file_path_test/filepath_test_001_config.json";
    SetConfigFile(configFilePath);

    AddFilePathTestModified();

    AddModuleElement("entry.A1", "entry.A1");
    AddModuleElement("entry.dir.bar.A", "entry.dir.bar.A");
    AddModuleElement("entry.dir.bar.foo.A", "entry.dir.bar.foo.A");
    AddModuleElement("entry.dir.foo.A", "entry.dir.foo.A");
    AddModuleElement("entry.dir.foo.B", "entry.dir.foo.B");

    AddKeptModuleElement("entry.A", "entry.A");
    AddKeptModuleElement("entry.foo.A", "entry.foo.A");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_102
 * @tc.desc: test obf with universalReservedFileNames options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_102, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.updated.02.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.02.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("entry.main", "main");

    std::string configFilePath = GUARD_UNIT_TEST_DIR "projects/file_path_test/filepath_test_002_config.json";
    SetConfigFile(configFilePath);

    AddFilePathTestModified();

    AddModuleElement("entry.A", "entry.A");
    AddModuleElement("entry.A1", "entry.A1");
    AddModuleElement("entry.dir.bar.foo.A", "entry.dir.bar.foo.A");
    AddModuleElement("entry.foo.A", "entry.foo.A");

    AddKeptModuleElement("entry.dir.bar.A", "entry.dir.bar.A");
    AddKeptModuleElement("entry.dir.foo.A", "entry.dir.foo.A");
    AddKeptModuleElement("entry.dir.foo.B", "entry.dir.foo.B");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_103
 * @tc.desc: test obf with reservedPaths options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_103, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.updated.03.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.03.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("entry.main", "main");

    std::string configFilePath = GUARD_UNIT_TEST_DIR "projects/file_path_test/filepath_test_003_config.json";
    SetConfigFile(configFilePath);

    AddFilePathTestModified();

    AddModuleElement("entry.A", "entry.A");
    AddModuleElement("entry.A1", "entry.A1");
    AddModuleElement("entry.dir.bar.A", "entry.dir.bar.A");
    AddModuleElement("entry.dir.bar.foo.A", "entry.dir.bar.foo.A");
    AddModuleElement("entry.dir.foo.A", "entry.dir.foo.A");
    AddModuleElement("entry.dir.foo.B", "entry.dir.foo.B");
    AddModuleElement("entry.foo.A", "entry.foo.A");

    AddKeptModuleElement("entry.foo.A", "entry.foo.");
    AddKeptModuleElement("entry.dir.bar.A", "entry.dir.bar.");
    AddKeptModuleElement("entry.dir.bar.foo.A", "entry.dir.bar.");
    AddKeptModuleElement("entry.dir.foo.A", "entry.dir.");
    AddKeptModuleElement("entry.dir.foo.B", "entry.dir.");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_104
 * @tc.desc: test obf with universalReservedPaths options
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_104, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.updated.04.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "projects/file_path_test/modules_static.04.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("entry.main", "main");

    std::string configFilePath = GUARD_UNIT_TEST_DIR "projects/file_path_test/filepath_test_004_config.json";
    SetConfigFile(configFilePath);

    AddFilePathTestModified();

    AddModuleElement("entry.A", "entry.A");
    AddModuleElement("entry.A1", "entry.A1");
    AddModuleElement("entry.dir.bar.A", "entry.dir.bar.A");
    AddModuleElement("entry.dir.bar.foo.A", "entry.dir.bar.foo.A");
    AddModuleElement("entry.dir.foo.A", "entry.dir.foo.A");
    AddModuleElement("entry.dir.foo.B", "entry.dir.foo.B");
    AddModuleElement("entry.foo.A", "entry.foo.A");

    AddKeptModuleElement("entry.foo.A", "entry.foo.");

    this->VerifyObfuscated();
}
// config obf test end

/**
 * @tc.name: obfuscator_test_011
 * @tc.desc: test obfuscator, only verify obfuscated names
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_011, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/obfuscator_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/obfuscator_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/obfuscator_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    AddElement<abckit_wrapper::Field>("obfuscator_test.field1", "field1");
    AddElement<abckit_wrapper::Field>("obfuscator_test.field2", "field2");
    AddElement<abckit_wrapper::Field>("obfuscator_test.field3", "field3");
    AddElement<abckit_wrapper::Method>("obfuscator_test.foo1:void;", "foo1");
    AddElement<abckit_wrapper::Namespace>("obfuscator_test.ns1", "ns1");
    AddElement<abckit_wrapper::Field>("obfuscator_test.ns1.nsField1", "nsField1");
    AddElement<abckit_wrapper::Method>("obfuscator_test.ns1.nsFoo1:void;", "nsFoo1");
    AddElement<abckit_wrapper::Class>("obfuscator_test.cl1", "cl1");
    AddElement<abckit_wrapper::Field>("obfuscator_test.cl1.clField1", "clField1");
    AddElement<abckit_wrapper::Method>("obfuscator_test.cl1.cl1Foo1:obfuscator_test.cl1;void;", "cl1Foo1");
    AddElement<abckit_wrapper::AnnotationInterface>("obfuscator_test.Anno1", "Anno1");
    AddElement<abckit_wrapper::AnnotationInterface>("obfuscator_test.Anno2", "Anno2");
    AddElement<abckit_wrapper::Class>("obfuscator_test.BaseInterface", "BaseInterface");
    AddElement<abckit_wrapper::Class>("obfuscator_test.BaseClass", "BaseClass");
    AddElement<abckit_wrapper::Class>("obfuscator_test.Class01", "Class01");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_012
 * @tc.desc: test module link field obfuscator, verify linked field name is same after obfuscated
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_012, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_field_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_field_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_field_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    const auto classAField1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_field_test.ClassA.%%set-field1:module_link_field_test.ClassA;i32;void;");
    ASSERT_TRUE(classAField1.has_value());
    const auto interface1Field1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_field_test.Interface1.%%set-field1:module_link_field_test.Interface1;i32;void;");
    ASSERT_TRUE(interface1Field1.has_value());

    const auto interface2Field1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_field_test.Interface2.%%set-field1:module_link_field_test.Interface2;i32;void;");
    ASSERT_TRUE(interface2Field1.has_value());
    const auto classCField1 = this->fileView_.Get<abckit_wrapper::Field>("module_link_field_test.ClassC.field1");
    ASSERT_TRUE(classCField1.has_value());

    this->ExecuteObfuscator();

    this->VerifyObfuscatedFilesExist();

    ASSERT_EQ(classAField1.value()->GetRawName(), interface1Field1.value()->GetRawName());
    ASSERT_EQ(classCField1.value()->GetRawName(), interface2Field1.value()->GetRawName());
}

/**
 * @tc.name: obfuscator_test_013
 * @tc.desc: test module link method obfuscator, verify linked method name is same after obfuscated
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_013, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_method_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_method_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/module_link_method_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    const auto classAMethod1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_method_test.ClassA.method1:module_link_method_test.ClassA;void;");
    ASSERT_TRUE(classAMethod1.has_value());
    const auto interface1Method1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_method_test.Interface1.method1:module_link_method_test.Interface1;void;");
    ASSERT_TRUE(interface1Method1.has_value());

    const auto classBMethod1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_method_test.ClassB.method1:module_link_method_test.ClassB;void;");
    ASSERT_TRUE(classBMethod1.has_value());
    const auto interface2Method1 = this->fileView_.Get<abckit_wrapper::Method>(
        "module_link_method_test.Interface2.method1:module_link_method_test.Interface2;void;");
    ASSERT_TRUE(interface2Method1.has_value());

    this->ExecuteObfuscator();

    ASSERT_EQ(classAMethod1.value()->GetRawName(), interface1Method1.value()->GetRawName());
    ASSERT_EQ(classBMethod1.value()->GetRawName(), interface2Method1.value()->GetRawName());

    this->VerifyObfuscatedFilesExist();
}

/**
 * @tc.name: obfuscator_test_014
 * @tc.desc: test export and import
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_014, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "projects/import_test/modules_static.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "projects/import_test/modules_static_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "projects/import_test/modules_static.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("entry.main", "main");

    AddModuleElement("entry.export_file", "entry.export_file");
    AddModuleElement("entry.main", "entry.main");
    AddModuleElement("entry.common.export_file", "entry.common.export_file");

    AddElement<abckit_wrapper::Method>("entry.export_file.process_field:entry.export_file.A;void;", "process_field");
    AddElement<abckit_wrapper::Class>("entry.export_file.A", "A");
    AddElement<abckit_wrapper::Method>("entry.export_file.A.foo:entry.export_file.A;std.core.String;", "foo");
    AddElement<abckit_wrapper::Class>("entry.export_file.B", "B");
    AddElement<abckit_wrapper::Field>("entry.export_file.B.a", "a");
    AddElement<abckit_wrapper::Method>("entry.export_file.B.foo:entry.export_file.B;void;", "foo");

    AddElement<abckit_wrapper::Field>("entry.main.b", "b");
    AddElement<abckit_wrapper::Field>("entry.main.var1", "var1");
    AddElement<abckit_wrapper::Method>("entry.main.foo1:void;", "foo1");
    AddElement<abckit_wrapper::Method>("entry.main.importTest:void;", "importTest");
    AddElement<abckit_wrapper::Class>("entry.main.C", "C");
    AddElement<abckit_wrapper::Method>("entry.main.C.foo:entry.main.C;void;", "foo");
    AddElement<abckit_wrapper::Field>("entry.main.str1", "str1");
    AddElement<abckit_wrapper::Field>("entry.main.str2", "str2");
    AddElement<abckit_wrapper::Field>("entry.main.uni1", "uni1");
    AddElement<abckit_wrapper::Field>("entry.main.uni2", "uni2");
    AddElement<abckit_wrapper::Field>("entry.main.uni3", "uni3");
    AddElement<abckit_wrapper::Field>("entry.main.ct", "ct");
    AddElement<abckit_wrapper::Field>("entry.main.mct", "mct");

    AddElement<abckit_wrapper::Method>("entry.common.export_file.f1:f64;void;", "f1");
    AddElement<abckit_wrapper::Method>("entry.common.export_file.f2:std.core.String;void;", "f2");
    AddElement<abckit_wrapper::Namespace>("entry.common.export_file.Space1", "Space1");
    AddElement<abckit_wrapper::Field>("entry.common.export_file.Space1.variable", "variable");
    AddElement<abckit_wrapper::Field>("entry.common.export_file.Space1.constant", "constant");
    AddElement<abckit_wrapper::Method>("entry.common.export_file.Space1.foo:void;", "foo");
    AddElement<abckit_wrapper::Namespace>("entry.common.export_file.Space2", "Space2");
    AddElement<abckit_wrapper::Field>("entry.common.export_file.Space2.tag", "tag");
    AddElement<abckit_wrapper::Field>("entry.common.export_file.Space2.variable", "variable");
    AddElement<abckit_wrapper::AnnotationInterface>("entry.common.export_file.MyAnno", "MyAnno");
    AddElement<abckit_wrapper::Class>("entry.common.export_file.SomeClass", "SomeClass");
    AddElement<abckit_wrapper::Method>(
        "entry.common.export_file.SomeClass.foo:entry.common.export_file.SomeClass;void;", "foo");
    AddElement<abckit_wrapper::Class>("entry.common.export_file.ExportClass", "ExportClass");
    AddElement<abckit_wrapper::Field>("entry.common.export_file.ExportClass.memC", "memC");
    AddElement<abckit_wrapper::Method>(
        "entry.common.export_file.ExportClass.%%get-memA:entry.common.export_file.ExportClass;std.core.String;",
        "memA");
    AddElement<abckit_wrapper::Method>(
        "entry.common.export_file.ExportClass.memB:entry.common.export_file.ExportClass;std.core.String;", "memB");
    AddElement<abckit_wrapper::Class>("entry.common.export_file.ClassType1", "ClassType1");
    AddElement<abckit_wrapper::Method>(
        "entry.common.export_file.ClassType1.test:entry.common.export_file.ClassType1;std.core.String;", "test");
    AddElement<abckit_wrapper::Class>("entry.common.export_file.ClassType2", "ClassType2");
    AddElement<abckit_wrapper::Method>(
        "entry.common.export_file.ClassType2.test:entry.common.export_file.ClassType2;std.core.String;", "test");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_015
 * @tc.desc: fields in file's toplevel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_015, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/toplevel_field_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/toplevel_field_demo_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/toplevel_field_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("toplevel_field_demo", "main");

    AddModuleElement("toplevel_field_demo", "toplevel_field_demo");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.var1", "var1");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.con1", "con1");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.var2", "var2");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.var3", "var3");
    AddElement<abckit_wrapper::Method>("toplevel_field_demo.foo1:std.core.Array;std.core.String;", "foo1");
    AddElement<abckit_wrapper::Class>("toplevel_field_demo.Cls1", "Cls1");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.Cls1.field1", "field1");
    AddElement<abckit_wrapper::Field>("toplevel_field_demo.Cls1.field2", "field2");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_016
 * @tc.desc: namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_016, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_demo_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("namespace_demo", "main");

    AddModuleElement("namespace_demo", "namespace_demo");

    AddElement<abckit_wrapper::Namespace>("namespace_demo.Foo", "Foo");
    AddElement<abckit_wrapper::Field>("namespace_demo.Foo.var1", "var1");
    AddElement<abckit_wrapper::Method>("namespace_demo.Foo.foo:std.core.String;std.core.String;", "foo");
    AddElement<abckit_wrapper::Namespace>("namespace_demo.Foo.Inner", "Inner");
    AddElement<abckit_wrapper::Field>("namespace_demo.Foo.Inner.var2", "var2");
    AddElement<abckit_wrapper::Method>("namespace_demo.Foo.Inner.foo:std.core.String;", "foo");
    AddElement<abckit_wrapper::Namespace>("namespace_demo.Foo.External", "External");
    AddElement<abckit_wrapper::Method>("namespace_demo.Foo.External.foo:std.core.String;", "foo");
    AddElement<abckit_wrapper::Class>("namespace_demo.Foo.Foo", "Foo");
    AddElement<abckit_wrapper::Field>("namespace_demo.Foo.Foo.name", "name");

    AddElement<abckit_wrapper::Namespace>("namespace_demo.Bar", "Bar");
    AddElement<abckit_wrapper::Field>("namespace_demo.Bar.count", "count");
    AddElement<abckit_wrapper::Field>("namespace_demo.Bar.base", "base");
    AddElement<abckit_wrapper::Method>("namespace_demo.Bar.foo:std.core.String;std.core.String;", "foo");
    AddElement<abckit_wrapper::Method>("namespace_demo.Bar.getNowCount:i32;", "getNowCount");

    AddElement<abckit_wrapper::Namespace>("namespace_demo.ns", "ns");
    AddElement<abckit_wrapper::Class>("namespace_demo.ns.A", "A");
    AddElement<abckit_wrapper::Method>("namespace_demo.ns.A.foo:namespace_demo.ns.A;std.core.String;", "foo");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_017
 * @tc.desc: functions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_017, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_demo_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("function_demo", "main");

    AddModuleElement("function_demo", "function_demo");
    AddElement<abckit_wrapper::Field>("function_demo.myDeck", "myDeck");
    AddElement<abckit_wrapper::Field>("function_demo.cls1", "cls1");
    AddElement<abckit_wrapper::Field>("function_demo.suits", "suits");
    AddElement<abckit_wrapper::Field>("function_demo.pickedCard1", "pickedCard1");
    AddElement<abckit_wrapper::Field>("function_demo.pickedCard2", "pickedCard2");
    AddElement<abckit_wrapper::Field>("function_demo.employeeName", "employeeName");
    AddElement<abckit_wrapper::Field>("function_demo.z", "z");
    AddElement<abckit_wrapper::Method>("function_demo.pickCard:f64;function_demo.Obj1;", "pickCard");
    AddElement<abckit_wrapper::Method>("function_demo.pickCard:function_demo.Obj1;f64;", "pickCard");
    AddElement<abckit_wrapper::Method>("function_demo.addToZ:f64;f64;f64;", "addToZ");
    AddElement<abckit_wrapper::Method>("function_demo.buildName2:std.core.String;std.core.String;std.core.String;",
                                       "buildName2");
    AddElement<abckit_wrapper::Method>("function_demo.math1:f64;f64;std.core.Double;f64;", "math1");
    AddElement<abckit_wrapper::Method>("function_demo.math1:f64;f64;std.core.Double;f64;", "math1");
    AddElement<abckit_wrapper::Method>("function_demo.add1:f64;f64;std.core.Object;", "add1");
    AddElement<abckit_wrapper::Method>("function_demo.buildName3:std.core.String;std.core.Array;std.core.String;",
                                       "buildName3");
    AddElement<abckit_wrapper::Method>("function_demo.buildName1:std.core.String;std.core.String;std.core.String;",
                                       "buildName1");
    AddElement<abckit_wrapper::Method>(
        "function_demo.foo1:i32;function_demo.ClassA;std.core.String;std.core.Array;void;", "foo1");
    AddElement<abckit_wrapper::Method>("function_demo.fun1:i32;", "fun1");
    AddElement<abckit_wrapper::Method>("function_demo.fun3:i32;", "fun3");
    AddElement<abckit_wrapper::Method>("function_demo.fun2:i32;", "fun2");
    AddElement<abckit_wrapper::Class>("function_demo.Obj1", "Obj1");
    AddElement<abckit_wrapper::Field>("function_demo.Obj1.%%property-suit", "suit");
    AddElement<abckit_wrapper::Field>("function_demo.Obj1.%%property-card", "card");
    AddElement<abckit_wrapper::Method>("function_demo.Obj1.%%set-card:function_demo.Obj1;f64;void;", "card");
    AddElement<abckit_wrapper::Method>("function_demo.Obj1.%%set-suit:function_demo.Obj1;std.core.String;void;",
                                       "suit");
    AddElement<abckit_wrapper::Class>("function_demo.ClassA", "ClassA");
    AddElement<abckit_wrapper::Method>(
        "function_demo.ClassA.method1:function_demo.ClassA;i32;std.core.String;std.core.Array;std.core.String;",
        "method1");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_018
 * @tc.desc: test obfuscated for class, class method, class field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_018, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo1.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo1_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo1.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("class_demo1", "main");

    AddModuleElement("class_demo1", "class_demo1");
    AddElement<abckit_wrapper::Class>("class_demo1.ClassA1", "ClassA1");
    AddElement<abckit_wrapper::Method>("class_demo1.ClassA1.ClassA:i32;", "ClassA");
    AddElement<abckit_wrapper::Method>("class_demo1.ClassA1.add:class_demo1.ClassA1;i32;", "add");

    AddElement<abckit_wrapper::Field>("class_demo1.obj", "obj");
    AddElement<abckit_wrapper::Field>("class_demo1.obj2", "obj2");

    AddElement<abckit_wrapper::Class>("class_demo1.ClassA2", "ClassA2");
    AddElement<abckit_wrapper::Field>("class_demo1.ClassA2.sFieldA0", "sFieldA0");
    AddElement<abckit_wrapper::Field>("class_demo1.ClassA2.fieldA1", "fieldA1");
    AddElement<abckit_wrapper::Method>("class_demo1.ClassA2.methodA1:class_demo1.ClassA2;f64;", "methodA1");
    AddElement<abckit_wrapper::Method>("class_demo1.ClassA2.%%get-value1:class_demo1.ClassA2;f64;", "value1");

    AddElement<abckit_wrapper::Field>("class_demo1.a", "a");

    AddElement<abckit_wrapper::Class>("class_demo1.C", "C");
    AddElement<abckit_wrapper::Method>("class_demo1.C.foo:class_demo1.C;class_demo1.C;", "foo");

    AddElement<abckit_wrapper::Class>("class_demo1.D", "D");
    AddElement<abckit_wrapper::Method>("class_demo1.D.foo:class_demo1.D;class_demo1.D;", "foo");

    AddElement<abckit_wrapper::Field>("class_demo1.x", "x");
    AddElement<abckit_wrapper::Field>("class_demo1.y", "y");

    AddElement<abckit_wrapper::Class>("class_demo1.ClassLong", "ClassLong");
    AddElement<abckit_wrapper::Field>("class_demo1.ClassLong.field1", "field1");
    AddElement<abckit_wrapper::Class>("class_demo1.ClassFloat", "ClassFloat");
    AddElement<abckit_wrapper::Field>("class_demo1.ClassFloat.field1", "field1");
    AddElement<abckit_wrapper::Class>("class_demo1.BaseClass", "BaseClass");
    AddElement<abckit_wrapper::Field>("class_demo1.BaseClass.field1", "field1");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_019
 * @tc.desc: test obfuscated for class access_flag and class hierarchy
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_019, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo2.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo2_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo2.json";
    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("class_demo2", "main");

    AddModuleElement("class_demo2", "class_demo2");

    AddElement<abckit_wrapper::Class>("class_demo2.AbstractClass", "AbstractClass");
    AddElement<abckit_wrapper::Field>("class_demo2.AbstractClass.field", "field");
    AddElement<abckit_wrapper::Method>(
        "class_demo2.AbstractClass.methodFinal:class_demo2.AbstractClass;std.core.String;", "methodFinal");
    AddElement<abckit_wrapper::Method>("class_demo2.AbstractClass.methodAbs:class_demo2.AbstractClass;std.core.String;",
                                       "methodAbs");
    AddElement<abckit_wrapper::Method>("class_demo2.AbstractClass.methodSync:class_demo2.AbstractClass;void;",
                                       "methodSync");
    AddElement<abckit_wrapper::Method>("class_demo2.AbstractClass.methodNative:class_demo2.AbstractClass;void;",
                                       "methodNative");

    AddElement<abckit_wrapper::Class>("class_demo2.Interface1", "Interface1");
    AddElement<abckit_wrapper::Field>("class_demo2.Interface1.%%property-field_inf", "field_inf");
    AddElement<abckit_wrapper::Method>("class_demo2.Interface1.%%get-field_inf:class_demo2.Interface1;std.core.String;",
                                       "field_inf");
    AddElement<abckit_wrapper::Method>(
        "class_demo2.Interface1.%%set-field_inf:class_demo2.Interface1;std.core.String;void;", "field_inf");
    AddElement<abckit_wrapper::Method>("class_demo2.Interface1.print:class_demo2.Interface1;void;", "print");

    AddElement<abckit_wrapper::Class>("class_demo2.Interface2", "Interface2");
    AddElement<abckit_wrapper::Method>("class_demo2.Interface2.getDescription:class_demo2.Interface2;std.core.String;",
                                       "getDescription");

    AddElement<abckit_wrapper::Class>("class_demo2.Interface3", "Interface3");
    AddElement<abckit_wrapper::Field>("class_demo2.Interface3.%%property-field_inf2", "field_inf2");
    AddElement<abckit_wrapper::Method>("class_demo2.Interface3.%%get-field_inf2:class_demo2.Interface3;f64;",
                                       "field_inf2");
    AddElement<abckit_wrapper::Method>("class_demo2.Interface3.%%set-field_inf2:class_demo2.Interface3;f64;void;",
                                       "field_inf2");

    AddElement<abckit_wrapper::Class>("class_demo2.ClassA", "ClassA");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.field0", "field0");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.field1", "field1");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.field2", "field2");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.field3", "field3");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.field4", "field4");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassA.%%property-field_inf", "field_inf");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.%%get-field_inf:class_demo2.ClassA;std.core.String;",
                                       "field_inf");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.%%set-field_inf:class_demo2.ClassA;std.core.String;void;",
                                       "field_inf");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.method0:std.core.String;", "method0");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.method1:class_demo2.ClassA;std.core.String;", "method1");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.method2:class_demo2.ClassA;std.core.String;", "method2");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.method3:class_demo2.ClassA;std.core.String;", "method3");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.method4:class_demo2.ClassA;i32;", "method4");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.methodAbs:class_demo2.ClassA;std.core.String;", "methodAbs");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassA.print:class_demo2.ClassA;void;", "print");

    AddElement<abckit_wrapper::Class>("class_demo2.ClassB", "ClassB");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassB.method1:class_demo2.ClassB;std.core.String;", "method1");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassB.getDescription:class_demo2.ClassB;std.core.String;",
                                       "getDescription");

    AddElement<abckit_wrapper::Class>("class_demo2.ClassC", "ClassC");
    AddElement<abckit_wrapper::Field>("class_demo2.ClassC.%%property-field_inf2", "field_inf2");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassC.%%get-field_inf2:class_demo2.ClassC;f64;", "field_inf2");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassC.%%set-field_inf2:class_demo2.ClassC;f64;void;",
                                       "field_inf2");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassC.method2:class_demo2.ClassC;std.core.String;", "method2");
    AddElement<abckit_wrapper::Method>("class_demo2.ClassC.getDescription:class_demo2.ClassC;std.core.String;",
                                       "getDescription");

    AddElement<abckit_wrapper::Class>("class_demo2.FinalClass", "FinalClass");
    AddElement<abckit_wrapper::Method>("class_demo2.FinalClass.methodFinal:class_demo2.FinalClass;void;",
                                       "methodFinal");
    AddElement<abckit_wrapper::Class>("class_demo2.ChildClass", "ChildClass");
    AddElement<abckit_wrapper::Field>("class_demo2.ChildClass.field2", "field2");
    const auto externalClass = this->fileView_.Get<abckit_wrapper::Class>("class_demo1.BaseClass");
    ASSERT_TRUE(externalClass.has_value());
    const auto externalField = this->fileView_.Get<abckit_wrapper::Field>("class_demo1.BaseClass.field1");
    ASSERT_TRUE(externalField.has_value());

    this->VerifyObfuscated();

    ASSERT_EQ(externalClass.value()->GetName(), "BaseClass");
    ASSERT_EQ(externalField.value()->GetRawName(), "field1");
}

/*
 * @tc.name: obfuscator_test_020
 * @tc.desc: test obfuscated for class get and set
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_020, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_get_set.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_get_set_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_get_set.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("class_get_set", "main");

    AddModuleElement("class_get_set", "class_get_set");

    AddElement<abckit_wrapper::Class>("class_get_set.Style1", "Style1");
    AddElement<abckit_wrapper::Field>("class_get_set.Style1.%%property-color", "color");
    AddElement<abckit_wrapper::Class>("class_get_set.StyleClass1", "StyleClass1");
    AddElement<abckit_wrapper::Field>("class_get_set.StyleClass1.%%property-color", "color");
    AddElement<abckit_wrapper::Class>("class_get_set.StyleClass2", "StyleClass2");
    AddElement<abckit_wrapper::Field>("class_get_set.StyleClass2.color_", "color_");
    AddElement<abckit_wrapper::Class>("class_get_set.StyleClass3", "StyleClass3");
    AddElement<abckit_wrapper::Field>("class_get_set.StyleClass3.colorClass3", "colorClass3");
    AddElement<abckit_wrapper::Class>("class_get_set.Style2", "Style2");
    AddElement<abckit_wrapper::Field>("class_get_set.Style2.%%property-color", "color");
    AddElement<abckit_wrapper::Field>("class_get_set.Style2.%%property-readable", "readable");
    AddElement<abckit_wrapper::Class>("class_get_set.Style2Class", "Style2Class");
    AddElement<abckit_wrapper::Field>("class_get_set.Style2Class.%%property-readable", "readable");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_021
 * @tc.desc: test obfuscated for function error
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_021, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_error_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_error_demo_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_error.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("function_error_demo", "main");

    AddModuleElement("function_error_demo", "function_error_demo");

    AddElement<abckit_wrapper::Class>("function_error_demo.ZeroDivisor", "ZeroDivisor");
    AddElement<abckit_wrapper::Method>("function_error_demo.divide:f64;f64;f64;", "divide");
    AddElement<abckit_wrapper::Method>("function_error_demo.process:f64;f64;f64;", "process");
    AddElement<abckit_wrapper::Class>("function_error_demo.SomeResource", "SomeResource");
    AddElement<abckit_wrapper::Method>("function_error_demo.SomeResource.close:function_error_demo.SomeResource;void;",
                                       "close");
    AddElement<abckit_wrapper::Method>("function_error_demo.processFile:std.core.String;void;", "processFile");
    AddElement<abckit_wrapper::Class>("function_error_demo.UnknownError", "UnknownError");
    AddElement<abckit_wrapper::Field>("function_error_demo.UnknownError.error", "error");
    AddElement<abckit_wrapper::Method>("function_error_demo.getArrayElement:std.core.Array;i32;std.core.Object;",
                                       "getArrayElement");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_022
 * @tc.desc: annotation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_022, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/annotation_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/annotation_demo_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/annotation_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("annotation_demo", "main");

    AddModuleElement("annotation_demo", "annotation_demo");

    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.MyAnno", "MyAnno");
    AddElement<abckit_wrapper::Class>("annotation_demo.A", "A");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.ClassPreamble", "ClassPreamble");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.Empty", "Empty");
    AddElement<abckit_wrapper::Class>("annotation_demo.C1", "C1");
    AddElement<abckit_wrapper::Class>("annotation_demo.C2", "C2");
    AddElement<abckit_wrapper::Class>("annotation_demo.C3", "C3");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.deprecated", "deprecated");
    AddElement<abckit_wrapper::Method>("annotation_demo.foo:void;", "foo");
    AddElement<abckit_wrapper::Method>("annotation_demo.goo:void;", "goo");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.SrcAnno", "SrcAnno");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.BtcAnno", "BtcAnno");
    AddElement<abckit_wrapper::Class>("annotation_demo.annoCls1", "annoCls1");
    AddElement<abckit_wrapper::Field>("annotation_demo.var1", "var1");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.SpecialCall", "SpecialCall");
    AddElement<abckit_wrapper::Class>("annotation_demo.specialCls", "specialCls");
    AddElement<abckit_wrapper::Field>("annotation_demo.specialCls.name", "name");
    AddElement<abckit_wrapper::Method>("annotation_demo.specialCls.foo:annotation_demo.specialCls;void;", "foo");
    AddElement<abckit_wrapper::Field>("annotation_demo.cls1", "cls1");
    AddElement<abckit_wrapper::AnnotationInterface>("annotation_demo.ListAnnotation", "ListAnnotation");
    AddElement<abckit_wrapper::Class>("annotation_demo.ListAnnotationClass", "ListAnnotationClass");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_023
 * @tc.desc: enum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_023, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/enum_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/enum_demo.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/enum_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("enum_demo", "main");

    AddModuleElement("enum_demo", "enum_demo");

    AddElement<abckit_wrapper::Class>("enum_demo.Flags", "Flags");
    AddElement<abckit_wrapper::Field>("enum_demo.Flags.Read", "Read");
    AddElement<abckit_wrapper::Field>("enum_demo.Flags.Write", "Write");
    AddElement<abckit_wrapper::Field>("enum_demo.Flags.ReadWrite", "ReadWrite");
    AddElement<abckit_wrapper::Class>("enum_demo.Empty", "Empty");
    AddElement<abckit_wrapper::Class>("enum_demo.Commands", "Commands");
    AddElement<abckit_wrapper::Field>("enum_demo.Commands.Open", "Open");
    AddElement<abckit_wrapper::Field>("enum_demo.Commands.Close", "Close");
    AddElement<abckit_wrapper::Field>("enum_demo.flag", "flag");
    AddElement<abckit_wrapper::Field>("enum_demo.command", "command");
    AddElement<abckit_wrapper::Class>("enum_demo.Color", "Color");
    AddElement<abckit_wrapper::Field>("enum_demo.Color.Red", "Red");
    AddElement<abckit_wrapper::Field>("enum_demo.Color.Green", "Green");
    AddElement<abckit_wrapper::Field>("enum_demo.Color.Blue", "Blue");
    AddElement<abckit_wrapper::Field>("enum_demo.c", "c");
    AddElement<abckit_wrapper::Class>("enum_demo.E", "E");
    AddElement<abckit_wrapper::Field>("enum_demo.E.One", "One");
    AddElement<abckit_wrapper::Field>("enum_demo.E.one", "one");
    AddElement<abckit_wrapper::Field>("enum_demo.E.oNe", "oNe");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_024
 * @tc.desc: generics
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_024, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo1.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo1_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo1.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("generics_demo1", "main");

    AddModuleElement("generics_demo1", "generics_demo1");
    AddElement<abckit_wrapper::Field>("generics_demo1.varY", "varY");
    AddElement<abckit_wrapper::Field>("generics_demo1.c3", "c3");
    AddElement<abckit_wrapper::Field>("generics_demo1.nums", "nums");
    AddElement<abckit_wrapper::Field>("generics_demo1.varInOut", "varInOut");
    AddElement<abckit_wrapper::Field>("generics_demo1.instanceA", "instanceA");
    AddElement<abckit_wrapper::Field>("generics_demo1.c2", "c2");
    AddElement<abckit_wrapper::Field>("generics_demo1.array", "array");
    AddElement<abckit_wrapper::Field>("generics_demo1.c1", "c1");
    AddElement<abckit_wrapper::Method>("generics_demo1.foo:std.core.Object;std.core.Object;void;", "foo");
    AddElement<abckit_wrapper::Method>("generics_demo1.loggingIdentity1:std.core.Array;std.core.Array;",
                                       "loggingIdentity1");
    AddElement<abckit_wrapper::Class>("generics_demo1.ClsA", "ClsA");
    AddElement<abckit_wrapper::Method>("generics_demo1.ClsA.method:generics_demo1.ClsA;std.core.Object;void;",
                                       "method");
    AddElement<abckit_wrapper::Class>("generics_demo1.InOutX", "InOutX");
    AddElement<abckit_wrapper::Field>("generics_demo1.InOutX.fldT2", "fldT2");
    AddElement<abckit_wrapper::Field>("generics_demo1.InOutX.fldT3", "fldT3");
    AddElement<abckit_wrapper::Method>("generics_demo1.InOutX.bar1:generics_demo1.InOutX;std.core.Object;", "bar1");
    AddElement<abckit_wrapper::Method>("generics_demo1.InOutX.foo:generics_demo1.InOutX;std.core.Object;void;", "foo");
    AddElement<abckit_wrapper::Method>(
        "generics_demo1.InOutX.method:generics_demo1.InOutX;std.core.Object;std.core.Object;", "method");
    AddElement<abckit_wrapper::Class>("generics_demo1.Y", "Y");
    AddElement<abckit_wrapper::Field>("generics_demo1.Y.f1", "f1");
    AddElement<abckit_wrapper::Field>("generics_demo1.Y.f2", "f2");
    AddElement<abckit_wrapper::Class>("generics_demo1.C2", "C2");
    AddElement<abckit_wrapper::Method>(
        "generics_demo1.C2.foo:generics_demo1.C2;std.core.Object;std.core.Object;std.core.Object;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_demo1.X", "X");
    AddElement<abckit_wrapper::Method>(
        "generics_demo1.X.tag:generics_demo1.X;std.core.Object;std.core.Object;std.core.String;", "tag");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_025
 * @tc.desc: generics with std libs
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_025, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo2.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo2_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_demo2.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("generics_demo2", "main");

    AddModuleElement("generics_demo2", "generics_demo2");
    AddElement<abckit_wrapper::Field>("generics_demo2.varX", "varX");
    AddElement<abckit_wrapper::Field>("generics_demo2.constY", "constY");
    AddElement<abckit_wrapper::Field>("generics_demo2.recordX", "recordX");
    AddElement<abckit_wrapper::Field>("generics_demo2.myIssue", "myIssue");
    AddElement<abckit_wrapper::Class>("generics_demo2.Issue", "Issue");
    AddElement<abckit_wrapper::Field>("generics_demo2.Issue.%%property-title", "title");
    AddElement<abckit_wrapper::Field>("generics_demo2.Issue.%%property-description", "description");
    AddElement<abckit_wrapper::Method>("generics_demo2.Issue.%%get-description:generics_demo2.Issue;std.core.String;",
                                       "description");
    AddElement<abckit_wrapper::Method>("generics_demo2.Issue.%%get-title:generics_demo2.Issue;std.core.String;",
                                       "title");
    AddElement<abckit_wrapper::Method>(
        "generics_demo2.Issue.%%set-description:generics_demo2.Issue;std.core.String;void;", "description");
    AddElement<abckit_wrapper::Method>("generics_demo2.Issue.%%set-title:generics_demo2.Issue;std.core.String;void;",
                                       "title");
    AddElement<abckit_wrapper::Class>("generics_demo2.A", "A");
    AddElement<abckit_wrapper::Field>("generics_demo2.A.f1", "f1");
    AddElement<abckit_wrapper::Field>("generics_demo2.A.f2", "f2");
    AddElement<abckit_wrapper::Field>("generics_demo2.A.f3", "f3");
    AddElement<abckit_wrapper::Class>("generics_demo2.Issue2", "Issue2");
    AddElement<abckit_wrapper::Field>("generics_demo2.Issue2.%%property-title", "title");
    AddElement<abckit_wrapper::Method>("generics_demo2.Issue2.%%get-title:generics_demo2.Issue2;std.core.String;",
                                       "title");
    AddElement<abckit_wrapper::Method>("generics_demo2.Issue2.%%set-title:generics_demo2.Issue2;std.core.String;void;",
                                       "title");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_026
 * @tc.desc: generics with extends
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_026, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_extends.abc";
    std::string obfAbcFilePath =
        GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_extends.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/generics/generics_extends.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("generics_extends", "main");

    AddModuleElement("generics_extends", "generics_extends");
    AddElement<abckit_wrapper::Field>("generics_extends.h2", "h2");
    AddElement<abckit_wrapper::Field>("generics_extends.b1", "b1");
    AddElement<abckit_wrapper::Field>("generics_extends.varDerived1", "varDerived1");
    AddElement<abckit_wrapper::Field>("generics_extends.x2", "x2");
    AddElement<abckit_wrapper::Field>("generics_extends.varA1", "varA1");
    AddElement<abckit_wrapper::Field>("generics_extends.b3", "b3");
    AddElement<abckit_wrapper::Field>("generics_extends.h3", "h3");
    AddElement<abckit_wrapper::Field>("generics_extends.e1", "e1");
    AddElement<abckit_wrapper::Field>("generics_extends.h1", "h1");
    AddElement<abckit_wrapper::Field>("generics_extends.y1", "y1");
    AddElement<abckit_wrapper::Field>("generics_extends.x1", "x1");
    AddElement<abckit_wrapper::Class>("generics_extends.Exotic", "Exotic");
    AddElement<abckit_wrapper::Method>("generics_extends.Exotic.foo:generics_extends.Exotic;std.core.String;void;",
                                       "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.A1", "A1");
    AddElement<abckit_wrapper::Field>("generics_extends.A0.data", "data");
    AddElement<abckit_wrapper::Method>("generics_extends.A1.foo:generics_extends.A1;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.H", "H");
    AddElement<abckit_wrapper::Method>("generics_extends.H.foo:generics_extends.H;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.Base2", "Base2");
    AddElement<abckit_wrapper::Method>(
        "generics_extends.Base2.bar:generics_extends.Base2;std.core.Object;std.core.Object;", "bar");
    AddElement<abckit_wrapper::Class>("generics_extends.Derived1", "Derived1");
    AddElement<abckit_wrapper::Method>(
        "generics_extends.Derived1.foo:generics_extends.Derived1;generics_extends.SomeType;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.G2", "G2");
    AddElement<abckit_wrapper::Method>("generics_extends.G2.foo:generics_extends.G2;std.core.Object;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.Derived", "Derived");
    AddElement<abckit_wrapper::Class>("generics_extends.B2", "B2");
    AddElement<abckit_wrapper::Method>("generics_extends.B2.foo:generics_extends.B2;std.core.String;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.G1", "G1");
    AddElement<abckit_wrapper::Method>("generics_extends.G1.foo:generics_extends.G1;generics_extends.Base;void;",
                                       "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.SomeType", "SomeType");
    AddElement<abckit_wrapper::Class>("generics_extends.Base", "Base");
    AddElement<abckit_wrapper::Class>("generics_extends.Interface", "Interface");
    AddElement<abckit_wrapper::Method>(
        "generics_extends.Interface.foo:generics_extends.Interface;std.core.Object;void;", "foo");
    AddElement<abckit_wrapper::Class>("generics_extends.B1", "B1");
    AddElement<abckit_wrapper::Field>("generics_extends.B1.f1", "f1");
    AddElement<abckit_wrapper::Field>("generics_extends.B1.f2", "f2");
    AddElement<abckit_wrapper::Field>("generics_extends.B1.f3", "f3");
    AddElement<abckit_wrapper::Class>("generics_extends.A0", "A0");
    AddElement<abckit_wrapper::Field>("generics_extends.A0.data", "data");
    AddElement<abckit_wrapper::Method>("generics_extends.A0.foo:generics_extends.A0;void;", "foo");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_027
 * @tc.desc: class with field access
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_027, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo3.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo3_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/class/class_demo3.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("class_demo3", "main");

    AddElement<abckit_wrapper::Class>("class_demo3.ExportClass", "ExportClass");
    AddElement<abckit_wrapper::Method>("class_demo3.ExportClass.%%get-memA:class_demo3.ExportClass;std.core.String;",
                                       "memA");

    AddElement<abckit_wrapper::Method>("class_demo3.ExportClass.memB:class_demo3.ExportClass;std.core.String;", "memB");
    AddElement<abckit_wrapper::Field>("class_demo3.ExportClass.memC", "memC");
    AddElement<abckit_wrapper::Field>("class_demo3.inst", "inst");
    AddElement<abckit_wrapper::Class>("class_demo3.Base", "Base");
    AddElement<abckit_wrapper::Method>("class_demo3.Base.%%get-field:class_demo3.Base;class_demo3.Base;", "field");
    AddElement<abckit_wrapper::Method>("class_demo3.Base.%%set-field:class_demo3.Base;class_demo3.Derived;void;",
                                       "field");
    AddElement<abckit_wrapper::Class>("class_demo3.Derived", "Derived");
    AddElement<abckit_wrapper::Method>("class_demo3.Derived.%%get-field:class_demo3.Derived;class_demo3.Derived;",
                                       "field");
    AddElement<abckit_wrapper::Method>("class_demo3.Derived.%%set-field:class_demo3.Derived;class_demo3.Base;void;",
                                       "field");
    AddElement<abckit_wrapper::Method>("class_demo3.foo:class_demo3.Base;void;", "foo");
    AddElement<abckit_wrapper::Class>("class_demo3.Point", "Point");
    AddElement<abckit_wrapper::Field>("class_demo3.Point.x", "x");
    AddElement<abckit_wrapper::Field>("class_demo3.Point.y", "y");
    AddElement<abckit_wrapper::Class>("class_demo3.ColoredPoint", "ColoredPoint");
    AddElement<abckit_wrapper::Field>("class_demo3.ColoredPoint.WHITE", "WHITE");
    AddElement<abckit_wrapper::Field>("class_demo3.ColoredPoint.BLACK", "BLACK");
    AddElement<abckit_wrapper::Field>("class_demo3.ColoredPoint.color", "color");
    AddElement<abckit_wrapper::Class>("class_demo3.BWPoint", "BWPoint");
    AddElement<abckit_wrapper::Field>("class_demo3.pt1", "pt1");
    AddElement<abckit_wrapper::Field>("class_demo3.pt2", "pt2");
    AddElement<abckit_wrapper::Class>("class_demo3.AbstractClass", "AbstractClass");
    AddElement<abckit_wrapper::Method>("class_demo3.getUnion:i32;{Uclass_demo3.Base,class_demo3.Point};", "getUnion");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_028
 * @tc.desc: stack recovery, this case is used to test for errors, when obfuscated, the module name will be changed, so
 * not run abc
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_028, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/stack_recovery.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/stack_recovery.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/stack_recovery.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    AddModuleElement("stack_recovery", "stack_recovery");
    AddElement<abckit_wrapper::Field>("stack_recovery.x", "x");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_029
 * @tc.desc: lambda function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_029, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/lambda_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/lambda_demo.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/lambda_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("lambda_demo", "main");

    AddElement<abckit_wrapper::Field>("lambda_demo.myIdentity", "myIdentity");
    AddElement<abckit_wrapper::Field>("lambda_demo.math2", "math2");
    AddElement<abckit_wrapper::Field>("lambda_demo.math1", "math1");
    AddElement<abckit_wrapper::Field>("lambda_demo.buildNameFun", "buildNameFun");
    AddElement<abckit_wrapper::Field>("lambda_demo.var2", "var2");
    AddElement<abckit_wrapper::Field>("lambda_demo.var1", "var1");
    AddElement<abckit_wrapper::Field>("lambda_demo.fun1", "fun1");
    AddElement<abckit_wrapper::Method>("lambda_demo.buildName:std.core.String;std.core.Array;std.core.String;",
                                       "buildName");
    AddElement<abckit_wrapper::Method>("lambda_demo.math3:f64;f64;std.core.Array;f64;", "math3");
    AddElement<abckit_wrapper::Method>("lambda_demo.foo1:std.core.Array;std.core.String;", "foo1");
    AddElement<abckit_wrapper::Method>("lambda_demo.identity:std.core.Object;std.core.Object;", "identity");
    AddElement<abckit_wrapper::Method>("lambda_demo.memB:std.core.String;", "memB");
    AddElement<abckit_wrapper::Class>("lambda_demo.C1", "C1");
    AddElement<abckit_wrapper::Field>("lambda_demo.C1.field1", "field1");
    AddElement<abckit_wrapper::Class>("lambda_demo.GenericIdentityFn", "GenericIdentityFn");
    AddElement<abckit_wrapper::Field>("lambda_demo.GenericIdentityFn.%%property-foo", "foo");
    AddElement<abckit_wrapper::Method>(
        "lambda_demo.GenericIdentityFn.%%get-foo:lambda_demo.GenericIdentityFn;std.core.Function1;", "foo");
    AddElement<abckit_wrapper::Method>(
        "lambda_demo.GenericIdentityFn.%%set-foo:lambda_demo.GenericIdentityFn;std.core.Function1;void;", "foo");

    this->VerifyObfuscated();
}

/*
 * @tc.name: obfuscator_test_030
 * @tc.desc: async function test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_030, TestSize.Level1) {}

/**
 * @tc.name: obfuscator_test_031
 * @tc.desc: test namespace link field obfuscator, verify linked field name is same after obfuscated
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_031, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_field_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_field_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_field_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    const auto classAField1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_field_test.Ns1.ClassA.%%set-field1:ns_link_field_test.Ns1.ClassA;i32;void;");
    ASSERT_TRUE(classAField1.has_value());
    const auto interface1Field1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_field_test.Ns1.Interface1.%%set-field1:ns_link_field_test.Ns1.Interface1;i32;void;");
    ASSERT_TRUE(interface1Field1.has_value());

    const auto interface2Field1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_field_test.Ns1.Interface2.%%set-field1:ns_link_field_test.Ns1.Interface2;i32;void;");
    ASSERT_TRUE(interface2Field1.has_value());
    const auto classCField1 = this->fileView_.Get<abckit_wrapper::Field>("ns_link_field_test.Ns1.ClassC.field1");
    ASSERT_TRUE(classCField1.has_value());

    this->ExecuteObfuscator();

    this->VerifyObfuscatedFilesExist();

    ASSERT_EQ(classAField1.value()->GetRawName(), interface1Field1.value()->GetRawName());
    ASSERT_EQ(classCField1.value()->GetRawName(), interface2Field1.value()->GetRawName());
}

/**
 * @tc.name: obfuscator_test_032
 * @tc.desc: test namespace link method obfuscator, verify linked method name is same after obfuscated
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_032, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_method_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_method_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/ns_link_method_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);

    const auto classAMethod1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_method_test.Ns1.ClassA.method1:ns_link_method_test.Ns1.ClassA;void;");
    ASSERT_TRUE(classAMethod1.has_value());
    const auto interface1Method1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_method_test.Ns1.Interface1.method1:ns_link_method_test.Ns1.Interface1;void;");
    ASSERT_TRUE(interface1Method1.has_value());

    const auto classBMethod1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_method_test.Ns1.ClassB.method1:ns_link_method_test.Ns1.ClassB;void;");
    ASSERT_TRUE(classBMethod1.has_value());
    const auto interface2Method1 = this->fileView_.Get<abckit_wrapper::Method>(
        "ns_link_method_test.Ns1.Interface2.method1:ns_link_method_test.Ns1.Interface2;void;");
    ASSERT_TRUE(interface2Method1.has_value());

    this->ExecuteObfuscator();

    ASSERT_EQ(classAMethod1.value()->GetRawName(), interface1Method1.value()->GetRawName());
    ASSERT_EQ(classBMethod1.value()->GetRawName(), interface2Method1.value()->GetRawName());

    this->VerifyObfuscatedFilesExist();
}

/**
 * @tc.name: obfuscator_test_033
 * @tc.desc: test incremental obfuscator, verify kept class name is same with name cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_033, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/incremental_test.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/incremental_test_updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/incremental_test.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    static const std::string keepStr1 = R"(-keep class incremental_test.Class1 {
        *;
    })";
    static const std::string keepStr2 = R"(-keep package incremental_test {
        field2:std.core.String;
        foo2():void;
    })";
    static const std::string keepStr3 = R"(-keep namespace incremental_test.Ns1 {
        *;
    })";

    ark::guard::KeepOptionParser parser1(keepStr1);
    ark::guard::KeepOptionParser parser2(keepStr2);
    ark::guard::KeepOptionParser parser3(keepStr3);
    this->AddKeepOptions(parser1.Parse().value());
    this->AddKeepOptions(parser2.Parse().value());
    this->AddKeepOptions(parser3.Parse().value());

    const auto class1 = this->fileView_.Get<abckit_wrapper::Class>("incremental_test.Class1");
    ASSERT_TRUE(class1.has_value());
    const auto class1Method1 =
        this->fileView_.Get<abckit_wrapper::Method>("incremental_test.Class1.foo1:incremental_test.Class1;void;");
    ASSERT_TRUE(class1Method1.has_value());
    const auto class1Field1 = this->fileView_.Get<abckit_wrapper::Field>("incremental_test.Class1.field1");
    ASSERT_TRUE(class1Field1.has_value());
    const auto class2 = this->fileView_.Get<abckit_wrapper::Class>("incremental_test.Class2");
    ASSERT_TRUE(class2.has_value());
    const auto interface1 = this->fileView_.Get<abckit_wrapper::Class>("incremental_test.Interface1");
    ASSERT_TRUE(interface1.has_value());
    const auto moduleField1 = this->fileView_.Get<abckit_wrapper::Field>("incremental_test.field2");
    ASSERT_TRUE(moduleField1.has_value());
    const auto moduleMethod1 = this->fileView_.Get<abckit_wrapper::Method>("incremental_test.foo2:void;");
    ASSERT_TRUE(moduleMethod1.has_value());
    const auto namespace1 = this->fileView_.Get<abckit_wrapper::Namespace>("incremental_test.Ns1");
    ASSERT_TRUE(namespace1.has_value());
    const auto namespace1Field1 = this->fileView_.Get<abckit_wrapper::Field>("incremental_test.Ns1.field3");
    ASSERT_TRUE(namespace1Field1.has_value());
    const auto namespace1Method1 = this->fileView_.Get<abckit_wrapper::Method>("incremental_test.Ns1.foo3:void;");
    ASSERT_TRUE(namespace1Method1.has_value());

    this->ExecuteObfuscator();

    ASSERT_EQ(class1.value()->GetName(), "Class1");
    ASSERT_EQ(class1Method1.value()->GetRawName(), "foo1");
    ASSERT_EQ(class1Field1.value()->GetRawName(), "field1");
    ASSERT_EQ(moduleField1.value()->GetRawName(), "field2");
    ASSERT_EQ(moduleMethod1.value()->GetRawName(), "foo2");
    ASSERT_EQ(namespace1.value()->GetName(), "Ns1");
    ASSERT_EQ(namespace1Field1.value()->GetRawName(), "field3");
    ASSERT_EQ(namespace1Method1.value()->GetRawName(), "foo3");
    ASSERT_NE(class2.value()->GetName(), "Class2");
    ASSERT_NE(interface1.value()->GetName(), "Interface1");

    this->VerifyObfuscatedFilesExist();

    this->ClearKeepOptions();
    this->ExecuteObfuscator();

    ASSERT_EQ(class1.value()->GetName(), "Class1");
    ASSERT_EQ(class1Method1.value()->GetRawName(), "foo1");
    ASSERT_EQ(class1Field1.value()->GetRawName(), "field1");
    ASSERT_EQ(moduleField1.value()->GetRawName(), "field2");
    ASSERT_EQ(moduleMethod1.value()->GetRawName(), "foo2");
    ASSERT_EQ(namespace1.value()->GetName(), "Ns1");
    ASSERT_EQ(namespace1Field1.value()->GetRawName(), "field3");
    ASSERT_EQ(namespace1Method1.value()->GetRawName(), "foo3");
    ASSERT_NE(class2.value()->GetName(), "Class2");
    ASSERT_NE(interface1.value()->GetName(), "Interface1");
}

/**
 * @tc.name: obfuscator_test_034
 * @tc.desc: test array objects scene
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_034, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/array_object_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/array_object_demo.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/array_object_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("array_object_demo", "main");

    AddModuleElement("array_object_demo", "array_object_demo");
    AddElement<abckit_wrapper::Field>("array_object_demo.myArr", "myArr");
    AddElement<abckit_wrapper::Field>("array_object_demo.pickedCard1", "pickedCard1");
    AddElement<abckit_wrapper::Field>("array_object_demo.arr2", "arr2");
    AddElement<abckit_wrapper::Field>("array_object_demo.myDeck", "myDeck");
    AddElement<abckit_wrapper::Field>("array_object_demo.suits", "suits");
    AddElement<abckit_wrapper::Field>("array_object_demo.pickedCard2", "pickedCard2");
    AddElement<abckit_wrapper::Method>("array_object_demo.pickCard:f64;array_object_demo.Obj1;", "pickCard");
    AddElement<abckit_wrapper::Method>("array_object_demo.pickCard:std.core.Array;std.core.String;", "pickCard");
    AddElement<abckit_wrapper::Method>("array_object_demo.pickCard:std.core.Array;f64;", "pickCard");
    AddElement<abckit_wrapper::Class>("array_object_demo.Obj2", "Obj2");
    AddElement<abckit_wrapper::Field>("array_object_demo.Obj2.%%property-suit", "suit");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj2.%%get-suit:array_object_demo.Obj2;std.core.String;",
                                       "suit");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj2.%%set-suit:array_object_demo.Obj2;std.core.String;void;",
                                       "suit");
    AddElement<abckit_wrapper::Class>("array_object_demo.Object1", "Object1");
    AddElement<abckit_wrapper::Class>("array_object_demo.Obj1", "Obj1");
    AddElement<abckit_wrapper::Field>("array_object_demo.Obj1.%%property-suit", "suit");
    AddElement<abckit_wrapper::Field>("array_object_demo.Obj1.%%property-card", "card");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj1.%%get-card:array_object_demo.Obj1;f64;", "card");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj1.%%set-card:array_object_demo.Obj1;f64;void;", "card");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj1.%%get-suit:array_object_demo.Obj1;std.core.String;",
                                       "suit");
    AddElement<abckit_wrapper::Method>("array_object_demo.Obj1.%%set-suit:array_object_demo.Obj1;std.core.String;void;",
                                       "suit");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_035
 * @tc.desc: test async function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_035, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_async_demo.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_async_demo.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/function_async_demo.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("function_async_demo", "main");

    AddModuleElement("function_async_demo", "function_async_demo");
    AddElement<abckit_wrapper::Method>("function_async_demo.sleep:i32;std.core.Promise;", "sleep");
    AddElement<abckit_wrapper::Method>("function_async_demo.async1:std.core.Promise;", "async1");
    AddElement<abckit_wrapper::Method>("function_async_demo.async2:std.core.Promise;", "async2");
    AddElement<abckit_wrapper::Method>("function_async_demo.async3:std.core.Promise;", "async3");
    AddElement<abckit_wrapper::Class>("function_async_demo.Exec", "Exec");
    AddElement<abckit_wrapper::Field>("function_async_demo.Exec.val", "val");
    AddElement<abckit_wrapper::Method>("function_async_demo.Exec.run:function_async_demo.Exec;void;", "run");
    AddElement<abckit_wrapper::Method>("function_async_demo.async4:std.core.Promise;", "async4");
    AddElement<abckit_wrapper::Method>("function_async_demo.async4Shell:std.core.Promise;", "async4Shell");
    AddElement<abckit_wrapper::Method>("function_async_demo.print:std.core.Promise;", "print");
    AddElement<abckit_wrapper::Class>("function_async_demo.AbstractClass", "AbstractClass");
    AddElement<abckit_wrapper::Method>(
        "function_async_demo.AbstractClass.methodAsync:function_async_demo.AbstractClass;std.core.Promise;",
        "methodAsync");
    AddElement<abckit_wrapper::Method>(
        "function_async_demo.AbstractClass.methodAbs:function_async_demo.AbstractClass;std.core.String;", "methodAbs");
    AddElement<abckit_wrapper::Class>("function_async_demo.ClassA", "ClassA");
    AddElement<abckit_wrapper::Method>("function_async_demo.ClassA.method2:function_async_demo.ClassA;std.core.String;",
                                       "method2");
    AddElement<abckit_wrapper::Method>(
        "function_async_demo.ClassA.methodAbs:function_async_demo.ClassA;std.core.String;", "methodAbs");
    AddElement<abckit_wrapper::Method>("function_async_demo.ClassA.print:function_async_demo.ClassA;void;", "print");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_036
 * @tc.desc: test namespace default symbol
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_036, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_default_symbol.abc";
    std::string obfAbcFilePath =
        GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_default_symbol.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/namespace_default_symbol.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("namespace_default_symbol", "main");

    AddModuleElement("namespace_default_symbol", "namespace_default_symbol");
    AddElement<abckit_wrapper::Namespace>("namespace_default_symbol.External", "External");

    this->VerifyObfuscated();
}

/**
 * @tc.name: obfuscator_test_037
 * @tc.desc: test union type ref
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ObfuscatorTest, obfuscator_test_037, TestSize.Level1)
{
    std::string abcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/union_type_ref.abc";
    std::string obfAbcFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/union_type_ref.updated.abc";
    std::string nameCacheFilePath = GUARD_ABC_FILE_DIR "ut/obfuscator/code_sample/union_type_ref.json";

    this->Init(abcFilePath, obfAbcFilePath, nameCacheFilePath);
    this->InitRunAbcConfig("union_type_ref", "main");

    AddModuleElement("union_type_ref", "union_type_ref");
    AddElement<abckit_wrapper::Class>("union_type_ref.BaseType", "BaseType");
    AddElement<abckit_wrapper::Class>("union_type_ref.SomeType", "SomeType");
    AddElement<abckit_wrapper::Field>("union_type_ref.var2", "var2");

    this->VerifyObfuscated();
}
