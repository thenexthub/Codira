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
#include "poly.impl.hpp"
#include <iostream>
#include "poly.proj.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"

namespace {
class AnimalImpl {
public:
    AnimalImpl() {}

    void Speak()
    {
        std::cout << "Animal Speak" << std::endl;
    }

    void Move()
    {
        std::cout << "Animal Move" << std::endl;
    }
};

class DogImpl {
public:
    DogImpl() {}

    void Fetch()
    {
        std::cout << "Dog Fetch" << std::endl;
    }

    void Speak()
    {
        std::cout << "Dog Speak" << std::endl;
    }

    void Move()
    {
        std::cout << "Dog Move" << std::endl;
    }
};

class CatImpl {
public:
    CatImpl() {}

    void Fetch()
    {
        std::cout << "Cat Fetch" << std::endl;
    }

    void Speak()
    {
        std::cout << "Cat Speak" << std::endl;
    }

    void Move()
    {
        std::cout << "Cat Move" << std::endl;
    }
};

::poly::Animal MakeAnimal()
{
    return taihe::make_holder<AnimalImpl, ::poly::Animal>();
}

::poly::Dog MakeDog()
{
    return taihe::make_holder<DogImpl, ::poly::Dog>();
}

::poly::Cat MakeCat()
{
    return taihe::make_holder<CatImpl, ::poly::Cat>();
}

::poly::AnimalType GetAnimal(::poly::AnimalTag tag)
{
    switch (tag.get_key()) {
        case ::poly::AnimalTag::key_t::ANIMAL:
            return ::poly::AnimalType::make_animal(taihe::make_holder<AnimalImpl, ::poly::Animal>());
        case ::poly::AnimalTag::key_t::DOG:
            return ::poly::AnimalType::make_dog(taihe::make_holder<DogImpl, ::poly::Dog>());
        case ::poly::AnimalTag::key_t::CAT:
            return ::poly::AnimalType::make_cat(taihe::make_holder<CatImpl, ::poly::Cat>());
    }
}

::taihe::string InteractAnimal(::poly::AnimalType const &a)
{
    switch (a.get_tag()) {
        case ::poly::AnimalType::tag_t::animal:
            a.get_animal_ref()->Speak();
            a.get_animal_ref()->Move();
            return "Unknown Animal";
        case ::poly::AnimalType::tag_t::dog:
            ::poly::Animal(a.get_dog_ref())->Speak();
            ::poly::Animal(a.get_dog_ref())->Move();
            a.get_dog_ref()->Fetch();
            return "Dog";
        case ::poly::AnimalType::tag_t::cat:
            ::poly::Animal(a.get_cat_ref())->Speak();
            ::poly::Animal(a.get_cat_ref())->Move();
            a.get_cat_ref()->Fetch();
            return "Cat";
    }
}

::poly::PersonType GetPerson(::poly::PersonTag tag, ::taihe::string_view name, int32_t age)
{
    switch (tag.get_key()) {
        case ::poly::PersonTag::key_t::PERSON: {
            ::poly::Person person {name, age};
            return ::poly::PersonType::make_person(person);
        }
        case ::poly::PersonTag::key_t::WORKER: {
            ::poly::Worker worker {{name, age}, 12};
            return ::poly::PersonType::make_worker(worker);
        }
        case ::poly::PersonTag::key_t::STUDENT: {
            ::poly::Student student {{name, age}, 1};
            return ::poly::PersonType::make_student(student);
        }
    }
}

::taihe::string IntroduceYourself(::poly::PersonType const &p)
{
    switch (p.get_tag()) {
        case ::poly::PersonType::tag_t::person:
            std::cout << "My name is " << p.get_person_ref().name << " and my age is " << p.get_person_ref().age
                      << std::endl;
            return "Unknown Person";
        case ::poly::PersonType::tag_t::worker:
            std::cout << "My name is " << p.get_worker_ref().person.name << " and my age is "
                      << p.get_worker_ref().person.age << std::endl;
            std::cout << "I'm a worker, and have worked for " << p.get_worker_ref().serviceLen << " years" << std::endl;
            return "Worker";
        case ::poly::PersonType::tag_t::student:
            std::cout << "My name is " << p.get_student_ref().person.name << " and my age is "
                      << p.get_student_ref().person.age << std::endl;
            std::cout << "I'm a student, and grade ranking: " << p.get_student_ref().rank << std::endl;
            return "Student";
    }
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_MakeAnimal(MakeAnimal);
TH_EXPORT_CPP_API_MakeDog(MakeDog);
TH_EXPORT_CPP_API_MakeCat(MakeCat);
TH_EXPORT_CPP_API_GetAnimal(GetAnimal);
TH_EXPORT_CPP_API_InteractAnimal(InteractAnimal);
TH_EXPORT_CPP_API_GetPerson(GetPerson);
TH_EXPORT_CPP_API_IntroduceYourself(IntroduceYourself);
// NOLINTEND