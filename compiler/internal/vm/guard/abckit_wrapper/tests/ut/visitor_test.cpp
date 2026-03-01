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
#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/module_static.abc";
abckit_wrapper::FileView fileView;
}  // namespace

class BaseTestVisitor : public abckit_wrapper::ModuleVisitor {
public:
    void AddTargetObjectName(const std::string &objectName)
    {
        this->targetObjectNames_.insert(objectName);
    }

    virtual void InitTargetObjectNames() = 0;

    void Validate()
    {
        this->InitTargetObjectNames();
        ASSERT_EQ(this->objects_.size(), this->targetObjectNames_.size());
        for (const auto &obj : this->objects_) {
            ASSERT_TRUE(this->targetObjectNames_.find(obj->GetFullyQualifiedName()) != this->targetObjectNames_.end());
        }
    }

protected:
    std::vector<abckit_wrapper::Object *> objects_;
    std::set<std::string> targetObjectNames_;
};

class TestVisitor : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        ASSERT_TRUE(visitor_ != nullptr);
        visitor_->Validate();
    }

protected:
    std::unique_ptr<BaseTestVisitor> visitor_ = nullptr;
};

class TestLocalModuleVisitor final : public BaseTestVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        LOG_I << "ModuleName:" << module->GetFullyQualifiedName();
        objects_.push_back(module);
        return true;
    }
};

/**
 * @tc.name: module_visitor_001
 * @tc.desc: test visit all local modules of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, module_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalModuleVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalNamespaceVisitor final : public BaseTestVisitor, public abckit_wrapper::NamespaceVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1");
        AddTargetObjectName("module_static.Ns1.Ns2");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->NamespacesAccept(*this);
    }

    bool Visit(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }
};

/**
 * @tc.name: namespace_visitor_001
 * @tc.desc: test visit all local namespaces of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, namespace_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalNamespaceVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestMethodVisitor final : public BaseTestVisitor, public abckit_wrapper::MethodVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Color1.valueOf:module_static.Color1;i32;");
        AddTargetObjectName("module_static.Ns1.Color2.getName:module_static.Ns1.Color2;std.core.String;");
        AddTargetObjectName("module_static.Color1.$_get:module_static.Color1;std.core.String;");
        AddTargetObjectName("module_static.Ns1.ClassB._ctor_:module_static.Ns1.ClassB;void;");
        AddTargetObjectName("module_static.Ns1.Color2.toString:module_static.Ns1.Color2;std.core.String;");
        AddTargetObjectName("module_static.Ns1.Color2._cctor_:void;");
        AddTargetObjectName("module_static.Ns1.Color2.valueOf:module_static.Ns1.Color2;i32;");
        AddTargetObjectName("module_static.Ns1.Color2._ctor_:module_static.Ns1.Color2;i32;i32;void;");
        AddTargetObjectName("module_static.Ns1.Interface2.iMethod2:module_static.Ns1.Interface2;void;");
        AddTargetObjectName("module_static.Color1._ctor_:module_static.Color1;i32;i32;void;");
        AddTargetObjectName("module_static.Ns1.Ns2._cctor_:void;");
        AddTargetObjectName("module_static.Ns1._cctor_:void;");
        AddTargetObjectName("module_static._cctor_:void;");
        AddTargetObjectName("module_static.Ns1.Color2.$_get:module_static.Ns1.Color2;std.core.String;");
        AddTargetObjectName("module_static.Ns1.ClassB._cctor_:void;");
        AddTargetObjectName("module_static.Ns1.ClassB.sMethod2:i32;void;");
        AddTargetObjectName("module_static.Ns1.ClassB.%%set-iField2:module_static.Ns1.ClassB;std.core.String;void;");
        AddTargetObjectName("module_static.Ns1.ClassB.%%get-iField2:module_static.Ns1.ClassB;std.core.String;");
        AddTargetObjectName("module_static.Ns1.ClassB.iMethod2:module_static.Ns1.ClassB;void;");
        AddTargetObjectName("module_static.ClassA._ctor_:module_static.ClassA;void;");
        AddTargetObjectName("module_static.ClassA.sMethod1:i32;void;");
        AddTargetObjectName("module_static.ClassA.%%set-iField1:module_static.ClassA;i32;void;");
        AddTargetObjectName("module_static.ClassA.%%get-iField1:module_static.ClassA;i32;");
        AddTargetObjectName("module_static.Color1.values:module_static.Color1[];");
        AddTargetObjectName("module_static.Color1.fromValue:i32;module_static.Color1;");
        AddTargetObjectName("module_static.Ns1.Color2.fromValue:i32;module_static.Ns1.Color2;");
        AddTargetObjectName("module_static.Ns1.foo2:i32;i32;void;");
        AddTargetObjectName("module_static.Ns1.Interface2.%%get-iField2:module_static.Ns1.Interface2;std.core.String;");
        AddTargetObjectName("module_static.Interface1.iMethod1:module_static.Interface1;void;");
        AddTargetObjectName("module_static.Color1.getValueOf:std.core.String;module_static.Color1;");
        AddTargetObjectName(
            "module_static.Ns1.Interface2.%%set-iField2:module_static.Ns1.Interface2;std.core.String;void;");
        AddTargetObjectName("module_static.foo1:i32;i32;void;");
        AddTargetObjectName("module_static.main:void;");
        AddTargetObjectName("module_static.Ns1.ClassB.method2:module_static.Ns1.ClassB;void;");
        AddTargetObjectName("module_static.ClassA._cctor_:void;");
        AddTargetObjectName("module_static.ClassA.iMethod1:module_static.ClassA;void;");
        AddTargetObjectName("module_static.Ns1.Color2.getOrdinal:module_static.Ns1.Color2;i32;");
        AddTargetObjectName("module_static.ClassA.method1:module_static.ClassA;void;");
        AddTargetObjectName("module_static.Ns1.Color2.getValueOf:std.core.String;module_static.Ns1.Color2;");
        AddTargetObjectName("module_static.Color1.getOrdinal:module_static.Color1;i32;");
        AddTargetObjectName("module_static.Ns1.Color2.values:module_static.Ns1.Color2[];");
        AddTargetObjectName("module_static.Color1.toString:module_static.Color1;std.core.String;");
        AddTargetObjectName("module_static.Interface1.%%get-iField1:module_static.Interface1;i32;");
        AddTargetObjectName("module_static.Interface1.%%set-iField1:module_static.Interface1;i32;void;");
        AddTargetObjectName("module_static.Color1._cctor_:void;");
        AddTargetObjectName("module_static.Color1.getName:module_static.Color1;std.core.String;");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->MethodsAccept(*this);
    }

    bool Visit(abckit_wrapper::Method *method) override
    {
        LOG_I << "MethodName:" << method->GetFullyQualifiedName();
        objects_.push_back(method);
        return true;
    }
};

/**
 * @tc.name: method_visitor_001
 * @tc.desc: test visit all methods(functions & methods) of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, method_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestMethodVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(*visitor_));
}

class TestFieldVisitor final : public BaseTestVisitor, public abckit_wrapper::FieldVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Color1.#StringValuesArray");
        AddTargetObjectName("module_static.Color1.#ValuesArray");
        AddTargetObjectName("module_static.Color1.RED");
        AddTargetObjectName("module_static.Ns1.Color2.YELLOW");
        AddTargetObjectName("module_static.Color1.#NamesArray");
        AddTargetObjectName("module_static.Ns1.m2");
        AddTargetObjectName("module_static.Ns1.Color2.#ordinal");
        AddTargetObjectName("module_static.Ns1.Color2.#StringValuesArray");
        AddTargetObjectName("module_static.Ns1.ClassB.sField2");
        AddTargetObjectName("module_static.Ns1.ClassB.%%property-iField2");
        AddTargetObjectName("module_static.Interface1.%%property-iField1");
        AddTargetObjectName("module_static.Color1.#ordinal");
        AddTargetObjectName("module_static.Ns1.Color2.#NamesArray");
        AddTargetObjectName("module_static.Ns1.ClassB.field2");
        AddTargetObjectName("module_static.Ns1.Color2.#ItemsArray");
        AddTargetObjectName("module_static.Color1.#ItemsArray");
        AddTargetObjectName("module_static.m1");
        AddTargetObjectName("module_static.ClassA.field1");
        AddTargetObjectName("module_static.ClassA.%%property-iField1");
        AddTargetObjectName("module_static.Ns1.Interface2.%%property-iField2");
        AddTargetObjectName("module_static.ClassA.sField1");
        AddTargetObjectName("module_static.Ns1.Color2.#ValuesArray");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->FieldsAccept(*this);
    }

    bool Visit(abckit_wrapper::Field *field) override
    {
        LOG_I << "FieldName:" << field->GetFullyQualifiedName();
        objects_.push_back(field);
        return true;
    }
};

/**
 * @tc.name: field_visitor_001
 * @tc.desc: test visit all fields of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, field_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestFieldVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(*visitor_));
}

class TestLocalClassVisitor final : public BaseTestVisitor, public abckit_wrapper::ClassVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1.Interface2");
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Interface1");
        AddTargetObjectName("module_static.Color1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->ClassesAccept(*this);
    }

    bool Visit(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: class_visitor_001
 * @tc.desc: test visit all local classes of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, class_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalClassVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalLeafClassVisitor final : public BaseTestVisitor, public abckit_wrapper::ClassVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Color1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool Visit(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }
};

/**
 * @tc.name: class_visitor_002
 * @tc.desc: test visit all local leaf classes of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, class_visitor_002, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestLocalLeafClassVisitor>();
    abckit_wrapper::ClassVisitor *classVisitor = testVisitor.get();
    ASSERT_TRUE(fileView.ModulesAccept(classVisitor->Wrap<abckit_wrapper::LeafClassVisitor>()
                                           .Wrap<abckit_wrapper::ModuleClassVisitor>()
                                           .Wrap<abckit_wrapper::LocalModuleFilter>()));
    visitor_ = std::move(testVisitor);
}

class TestLocalAnnotationInterfaceVisitor final : public BaseTestVisitor,
                                                  public abckit_wrapper::AnnotationInterfaceVisitor {
public:
    void InitTargetObjectNames() override
    {
        AddTargetObjectName("module_static.MyAnno1");
        AddTargetObjectName("module_static.Ns1.MyAnno2");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->AnnotationInterfacesAccept(*this);
    }

    bool Visit(abckit_wrapper::AnnotationInterface *ai) override
    {
        LOG_I << "AnnotationInterfaceName:" << ai->GetFullyQualifiedName();
        objects_.push_back(ai);
        return true;
    }
};

/**
 * @tc.name: annotation_interface_visitor_001
 * @tc.desc: test visit all local annotationInterface of the fileView
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, annotation_interface_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalAnnotationInterfaceVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalModuleChildVisitor final : public BaseTestVisitor, public abckit_wrapper::ChildVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Namespaces
        AddTargetObjectName("module_static.Ns1");
        // Functions
        AddTargetObjectName("module_static.main:void;");
        AddTargetObjectName("module_static.foo1:i32;i32;void;");
        AddTargetObjectName("module_static._cctor_:void;");
        // Fields
        AddTargetObjectName("module_static.m1");
        // Classes
        AddTargetObjectName("module_static.ClassA");
        AddTargetObjectName("module_static.Color1");
        AddTargetObjectName("module_static.Interface1");
        // AnnotationInterfaces
        AddTargetObjectName("module_static.MyAnno1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return module->ChildrenAccept(*this);
    }

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }

    bool VisitMethod(abckit_wrapper::Method *method) override
    {
        LOG_I << "MethodName:" << method->GetFullyQualifiedName();
        objects_.push_back(method);
        return true;
    }

    bool VisitField(abckit_wrapper::Field *field) override
    {
        LOG_I << "FieldName:" << field->GetFullyQualifiedName();
        objects_.push_back(field);
        return true;
    }

    bool VisitClass(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override
    {
        LOG_I << "AnnotationInterfaceName:" << ai->GetFullyQualifiedName();
        objects_.push_back(ai);
        return true;
    }
};

/**
 * @tc.name: child_visitor_001
 * @tc.desc: test visit all directly child of the local module
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, child_visitor_001, TestSize.Level1)
{
    visitor_ = std::make_unique<TestLocalModuleChildVisitor>();
    ASSERT_TRUE(fileView.ModulesAccept(visitor_->Wrap<abckit_wrapper::LocalModuleFilter>()));
}

class TestLocalNamespaceChildVisitor final : public BaseTestVisitor, public abckit_wrapper::ChildVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Namespaces
        AddTargetObjectName("module_static.Ns1.Ns2");
        // Functions
        AddTargetObjectName("module_static.Ns1._cctor_:void;");
        AddTargetObjectName("module_static.Ns1.foo2:i32;i32;void;");
        // Fields
        AddTargetObjectName("module_static.Ns1.m2");
        // Classes
        AddTargetObjectName("module_static.Ns1.ClassB");
        AddTargetObjectName("module_static.Ns1.Color2");
        AddTargetObjectName("module_static.Ns1.Interface2");
        // AnnotationInterfaces
        AddTargetObjectName("module_static.Ns1.MyAnno2");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool VisitNamespace(abckit_wrapper::Namespace *ns) override
    {
        LOG_I << "NamespaceName:" << ns->GetFullyQualifiedName();
        objects_.push_back(ns);
        return true;
    }

    bool VisitMethod(abckit_wrapper::Method *method) override
    {
        LOG_I << "MethodName:" << method->GetFullyQualifiedName();
        objects_.push_back(method);
        return true;
    }

    bool VisitField(abckit_wrapper::Field *field) override
    {
        LOG_I << "FieldName:" << field->GetFullyQualifiedName();
        objects_.push_back(field);
        return true;
    }

    bool VisitClass(abckit_wrapper::Class *clazz) override
    {
        LOG_I << "ClassName:" << clazz->GetFullyQualifiedName();
        objects_.push_back(clazz);
        return true;
    }

    bool VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai) override
    {
        LOG_I << "AnnotationInterfaceName:" << ai->GetFullyQualifiedName();
        objects_.push_back(ai);
        return true;
    }
};

/**
 * @tc.name: child_visitor_002
 * @tc.desc: test visit all directly child of the local namespace
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, child_visitor_002, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestLocalNamespaceChildVisitor>();
    const auto ns = fileView.Get<abckit_wrapper::Namespace>("module_static.Ns1");
    ASSERT_TRUE(ns.has_value());
    ASSERT_TRUE(ns.value()->ChildrenAccept(*testVisitor));
    visitor_ = std::move(testVisitor);
}

class TestLocalClassChildVisitor final : public BaseTestVisitor, public abckit_wrapper::ChildVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Methods
        AddTargetObjectName("module_static.ClassA.sMethod1:i32;void;");
        AddTargetObjectName("module_static.ClassA.method1:module_static.ClassA;void;");
        AddTargetObjectName("module_static.ClassA._ctor_:module_static.ClassA;void;");
        AddTargetObjectName("module_static.ClassA._cctor_:void;");
        AddTargetObjectName("module_static.ClassA.%%set-iField1:module_static.ClassA;i32;void;");
        AddTargetObjectName("module_static.ClassA.%%get-iField1:module_static.ClassA;i32;");
        AddTargetObjectName("module_static.ClassA.iMethod1:module_static.ClassA;void;");
        // Fields
        AddTargetObjectName("module_static.ClassA.field1");
        AddTargetObjectName("module_static.ClassA.sField1");
        AddTargetObjectName("module_static.ClassA.%%property-iField1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool VisitMethod(abckit_wrapper::Method *method) override
    {
        LOG_I << "MethodName:" << method->GetFullyQualifiedName();
        objects_.push_back(method);
        return true;
    }

    bool VisitField(abckit_wrapper::Field *field) override
    {
        LOG_I << "FieldName:" << field->GetFullyQualifiedName();
        objects_.push_back(field);
        return true;
    }
};

/**
 * @tc.name: child_visitor_003
 * @tc.desc: test visit all directly child of the local class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, child_visitor_003, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestLocalClassChildVisitor>();
    const auto clazz = fileView.Get<abckit_wrapper::Class>("module_static.ClassA");
    ASSERT_TRUE(clazz.has_value());
    ASSERT_TRUE(clazz.value()->ChildrenAccept(*testVisitor));
    visitor_ = std::move(testVisitor);
}

class TestClassAMemberVisitor final : public BaseTestVisitor, public abckit_wrapper::MemberVisitor {
public:
    void InitTargetObjectNames() override
    {
        // Methods
        AddTargetObjectName("module_static.ClassA.sMethod1:i32;void;");
        AddTargetObjectName("module_static.ClassA.method1:module_static.ClassA;void;");
        AddTargetObjectName("module_static.ClassA._ctor_:module_static.ClassA;void;");
        AddTargetObjectName("module_static.ClassA._cctor_:void;");
        AddTargetObjectName("module_static.ClassA.%%set-iField1:module_static.ClassA;i32;void;");
        AddTargetObjectName("module_static.ClassA.%%get-iField1:module_static.ClassA;i32;");
        AddTargetObjectName("module_static.ClassA.iMethod1:module_static.ClassA;void;");
        // Fields
        AddTargetObjectName("module_static.ClassA.field1");
        AddTargetObjectName("module_static.ClassA.sField1");
        AddTargetObjectName("module_static.ClassA.%%property-iField1");
    }

    bool Visit(abckit_wrapper::Module *module) override
    {
        return true;
    }

    bool Visit(abckit_wrapper::Member *member) override
    {
        LOG_I << "MemberName:" << member->GetFullyQualifiedName();
        objects_.emplace_back(member);
        return true;
    }
};

class TestClassAFilter final : public abckit_wrapper::ClassFilter {
public:
    explicit TestClassAFilter(ClassVisitor &visitor) : ClassFilter(visitor) {}

    bool Accepted(abckit_wrapper::Class *clazz) override
    {
        return clazz->GetFullyQualifiedName() == "module_static.ClassA";
    }
};

/**
 * @tc.name: member_visitor_001
 * @tc.desc: test visit all directly child of the local class
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestVisitor, member_visitor_001, TestSize.Level1)
{
    auto testVisitor = std::make_unique<TestClassAMemberVisitor>();
    abckit_wrapper::MemberVisitor *memberVisitor = testVisitor.get();
    fileView.ModulesAccept(memberVisitor->Wrap<abckit_wrapper::ClassMemberVisitor>()
                               .Wrap<TestClassAFilter>()
                               .Wrap<abckit_wrapper::ModuleClassVisitor>());
    visitor_ = std::move(testVisitor);
}
