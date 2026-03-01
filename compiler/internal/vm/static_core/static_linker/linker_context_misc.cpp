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

#include <numeric>
#include <sstream>

#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkbase/os/mutex.h"

#include "linker.h"
#include "linker_context.h"

namespace ark::static_linker {

namespace {

void DemangleName(std::ostream &o, std::string_view s)
{
    if (s.empty()) {
        return;
    }
    if (s.back() == 0) {
        s = s.substr(0, s.size() - 1);
    }
    if (s.empty()) {
        return;
    }

    if (s.front() == '[') {
        DemangleName(o, s.substr(1));
        o << "[]";
        return;
    }

    if (s.size() == 1) {
        auto ty = panda_file::Type::GetTypeIdBySignature(s.front());
        o << ty;
        return;
    }

    if (s.front() == 'L' && s.back() == ';') {
        s = s.substr(1, s.size() - 2UL);
        while (!s.empty()) {
            const auto to = s.find('/');
            o << s.substr(0, to);
            if (to != std::string::npos) {
                o << ".";
                s = s.substr(to + 1);
            } else {
                break;
            }
        }
        return;
    }

    o << "<unknown>";
}

void ReprItem(std::ostream &o, const panda_file::BaseItem *i);

void ReprMethod(std::ostream &o, panda_file::StringItem *name, panda_file::BaseClassItem *clz, panda_file::ProtoItem *p)
{
    auto typs = Helpers::BreakProto(p);
    auto refs = p->GetRefTypes();
    size_t numRefs = 0;
    auto reprType = [&typs, &refs, &numRefs, &o](size_t ii) {
        auto &t = typs[ii];
        if (t.IsPrimitive()) {
            o << t;
        } else {
            ReprItem(o, refs[numRefs++]);
        }
    };
    reprType(0);
    o << " ";
    ReprItem(o, clz);
    o << "." << name->GetData();
    o << "(";
    for (size_t ii = 1; ii < typs.size(); ii++) {
        reprType(ii);
        if (ii + 1 != typs.size()) {
            o << ", ";
        }
    }
    o << ")";
}

void ReprValueItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto j = static_cast<const panda_file::ValueItem *>(i);
    switch (j->GetType()) {
        case panda_file::ValueItem::Type::ARRAY: {
            auto *arr = static_cast<const panda_file::ArrayValueItem *>(j);
            const auto &its = arr->GetItems();
            o << "[";
            for (size_t k = 0; k < its.size(); k++) {
                if (k != 0) {
                    o << ", ";
                }
                ReprItem(o, &its[k]);
            }
            o << "]";
            break;
        }
        case panda_file::ValueItem::Type::INTEGER: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<uint32_t>() << " as int";
            break;
        }
        case panda_file::ValueItem::Type::LONG: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<uint64_t>() << " as long";
            break;
        }
        case panda_file::ValueItem::Type::FLOAT: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<float>() << " as float";
            break;
        }
        case panda_file::ValueItem::Type::DOUBLE: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<double>() << " as double";
            break;
        }
        case panda_file::ValueItem::Type::ID: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            ReprItem(o, scalar->GetIdItem());
            break;
        }
        default:
            UNREACHABLE();
    }
}

void ReprAnnotationItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto j = static_cast<const panda_file::AnnotationItem *>(i);
    ReprItem(o, j->GetClassItem());
    o << "<";
    bool first = true;
    for (auto &a : *j->GetElements()) {
        if (first) {
            first = false;
        } else {
            o << ", ";
        }
        o << a.GetName()->GetData() << "=";
        ReprItem(o, a.GetValue());
    }
    o << ">";
}

void ReprStringItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto str = static_cast<const panda_file::StringItem *>(i);
    auto view = std::string_view(str->GetData());
    o << '"';
    while (!view.empty()) {
        auto pos = view.find_first_of("\"\\\n");
        o << view.substr(0, pos);
        if (pos != std::string::npos) {
            if (view[pos] == '\n') {
                o << "\\n";
            } else {
                o << '\\' << view[pos];
            }
            view = view.substr(pos + 1);
        } else {
            view = "";
        }
    }
    o << '"';
}

template <typename T>
void ReprFieldItem(std::ostream &o, const T *j)
{
    ReprItem(o, j->GetTypeItem());
    o << " ";
    ReprItem(o, j->GetClassItem());
    o << "." << j->GetNameItem()->GetData();
}

// CC-OFFNXT(G.FUN.01, huge_method) big switch case
void ReprItem(std::ostream &o, const panda_file::BaseItem *i)
{
    if (i == nullptr) {
        o << "<null>";
        return;
    }
    switch (i->GetItemType()) {
        case panda_file::ItemTypes::FOREIGN_CLASS_ITEM: {
            auto j = static_cast<const panda_file::ForeignClassItem *>(i);
            DemangleName(o, j->GetNameItemData());
            break;
        }
        case panda_file::ItemTypes::CLASS_ITEM: {
            auto j = static_cast<const panda_file::ClassItem *>(i);
            DemangleName(o, j->GetNameItemData());
            break;
        }
        case panda_file::ItemTypes::FOREIGN_FIELD_ITEM: {
            auto j = static_cast<const panda_file::ForeignFieldItem *>(i);
            ReprFieldItem(o, j);
            break;
        }
        case panda_file::ItemTypes::FIELD_ITEM: {
            auto j = static_cast<const panda_file::FieldItem *>(i);
            ReprFieldItem(o, j);
            break;
        }
        case panda_file::ItemTypes::PRIMITIVE_TYPE_ITEM: {
            auto j = static_cast<const panda_file::PrimitiveTypeItem *>(i);
            o << j->GetType();
            break;
        }
        case panda_file::ItemTypes::FOREIGN_METHOD_ITEM: {
            auto j = static_cast<const panda_file::ForeignMethodItem *>(i);
            ReprMethod(o, j->GetNameItem(), j->GetClassItem(), j->GetProto());
            break;
        }
        case panda_file::ItemTypes::METHOD_ITEM: {
            auto j = static_cast<const panda_file::MethodItem *>(i);
            ReprMethod(o, j->GetNameItem(), j->GetClassItem(), j->GetProto());
            break;
        }
        case panda_file::ItemTypes::ANNOTATION_ITEM: {
            ReprAnnotationItem(o, i);
            break;
        }
        case panda_file::ItemTypes::VALUE_ITEM: {
            ReprValueItem(o, i);
            break;
        }
        case panda_file::ItemTypes::STRING_ITEM: {
            ReprStringItem(o, i);
            break;
        }
        default:
            o << "<unknown:" << i->GetOffset() << ">";
    }
}
}  // namespace

Context::Context(Config conf) : conf_(std::move(conf)) {}

Context::~Context() = default;

void Context::Write(const std::string &out)
{
    auto writer = panda_file::FileWriter(out);
    if (!writer) {
        Error("Can't write", {ErrorDetail("file", out)});
        return;
    }
    cont_.Write(&writer, true, false);

    result_.stats.itemsCount = knownItems_.size();
    result_.stats.classCount = cont_.GetClassMap()->size();
}

void Context::Read(const std::vector<std::string> &input)
{
    for (const auto &f : input) {
        auto rd = panda_file::File::Open(f);
        if (rd == nullptr) {
            Error("Can't open file", {ErrorDetail("location", f)});
            break;
        }
        auto reader = &readers_.emplace_front(std::move(rd));
        if (!reader->ReadContainer(false)) {
            Error("can't read container", {}, reader);
            break;
        }
    }

    ASSERT(input.size() ==
           std::accumulate(readers_.begin(), readers_.end(), (size_t)0, [](size_t c, const auto &) { return c + 1; }));
}

std::ostream &operator<<(std::ostream &o, const static_linker::Context::ErrorToStringWrapper &self)
{
    std::fill_n(std::ostream_iterator<char>(o), self.indent_, '\t');
    o << self.error_.GetName();
    std::visit(
        [&](const auto &v) {
            using T = std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
            if constexpr (std::is_same_v<T, std::string>) {
                if (!v.empty()) {
                    o << ": " << v;
                }
            } else {
                o << ": ";
                ReprItem(o, v);
                for (auto [beg, end] = self.ctx_->cameFrom_.equal_range(v); beg != end; ++beg) {
                    o << "\n";
                    std::fill_n(std::ostream_iterator<char>(o), self.indent_ + 1, '\t');
                    o << "declared at `" << beg->second->GetFilePtr()->GetFilename() << "`";
                }
            }
        },
        self.error_.GetInfo());
    return o;
}

void Context::Error(const std::string &msg, const std::vector<ErrorDetail> &details,
                    const panda_file::FileReader *reader)
{
    auto o = std::stringstream();
    o << msg;

    for (const auto &d : details) {
        o << "\n" << ErrorToString(d, 1);
    }
    if (reader != nullptr) {
        o << "\n\tat `" << reader->GetFilePtr()->GetFilename() << "`";
    }

    result_.errors.emplace_back(o.str());
}

}  // namespace ark::static_linker
