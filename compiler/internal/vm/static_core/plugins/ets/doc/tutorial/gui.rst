..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

Support for |UIFW|
==================

|

This section demonstrates mechanisms that |LANG| provides for
creating graphical user interface (GUI) programs. The section is based on
the |UIFW| declarative framework. |UIFW| provides a set of extensions of
the standard |TS| to declaratively describe the GUI of applications,
and the interaction between the GUI components.

|UIFW| Example
--------------

The following example provides a complete |UIFW|-based application as an
illustration of the GUI programming capabilities. For more details of the
|UIFW| features, refer to the |UIFW|
`Tutorial <https://gitee.com/openharmony/docs/blob/master/en/application-dev/quick-start/arkts-get-started.md>`_.

.. code-block:: typescript

    // ViewModel classes ---------------------------

    let nextId: number = 0

    @Observed class ObservedArray<T> extends Array<T> {
        constructor(arr: T[]) {
            super(arr)
        }
    }

    @Observed class Address {
        street : string
        zip    : number
        city   : string

        constructor(street: string, zip: number, city: string) {
            this.street = street
            this.zip = zip
            this.city = city
        }
    }

    @Observed class Person {
        id_     : string
        name    : string
        address : Address
        phones  : ObservedArray<string>

        constructor(
            name   : string,
            street : string,
            zip    : number,
            city   : string,
            phones : string[]
        ) {
            this.id_ = nextId.toString()
            nextId++

            this.name = name
            this.address = new Address(street, zip, city)
            this.phones = new ObservedArray<string>(phones)
        }
    }

    class AddressBook {
        me       : Person
        contacts : ObservedArray<Person>

        constructor(me: Person, contacts: Person[]) {
            this.me = me
            this.contacts = new ObservedArray<Person>(contacts)
        }
    }

    // @Components -----------------------

    // Renders the name of a Person object and the first number in the phones
    // ObservedArray<string>
    // For also the phone number to update we need two @ObjectLink here,
    // person and phones, cannot use this.person.phones.
    // Changes of inner Array not observed.
    // onClick updates selectedPerson also in AddressBookView, PersonEditView
    @Component
    struct PersonView {

        @ObjectLink person   : Person
        @ObjectLink phones   : ObservedArray<string>
        @Link selectedPerson : Person

        build() {
            Flex({
              direction: FlexDirection.Row, 
              justifyContent: FlexAlign.SpaceBetween
            }) {
                Text(this.person.name)
                Select(this.phones.length != 0, Text(this.phones[0]), null)
            }
            .height(55)
            .backgroundColor(
                this.selectedPerson.name == this.person.name ?
                   "#ffa0a0" : "#ffffff"
            )
            .onClick(() => {
                this.selectedPerson = this.person
            })
        }
    }

    // Renders all details
    // @Prop get initialized from parent AddressBookView, TextInput onChange
    // modifies local copies only on "Save Changes" copy all data from @Prop
    // to @ObjectLink, syncs to selectedPerson in other @Components.
    @Component
    struct PersonEditView {

        @Consume addrBook: AddressBook

        /* Person object and sub-objects owned by the parent Component */
        @Link selectedPerson: Person

        /* editing on local copy until save is handled */
        @Prop name    : string
        @Prop address : Address
        @Prop phones  : ObservedArray<string>

        selectedPersonIndex() : number {
            return this.addrBook.contacts.findIndex(
                (person) => person.id_ == this.selectedPerson.id_
            )
        }

        build() {
            Column() {
                TextInput({text: this.name})
                    .onChange((value) => {
                        this.name = value
                    })

                TextInput({text: this.address.street})
                    .onChange((value) => {
                        this.address.street = value
                    })

                TextInput({text: this.address.city})
                    .onChange((value) => {
                        this.address.city = value
                    })

                TextInput({text: this.address.zip.toString()})
                    .onChange((value) => {
                        const result = Number.parseInt(value)
                        this.address.zip = Number.isNaN(result) ? 0 : result
                    })

                Select(this.phones.length > 0,
                    ForEach(this.phones, (phone, index) => {
                        TextInput({text: phone})
                            .width(150)
                            .onChange((value) => {
                                console.log(
                                  `${index}.${value} value has changed`
                                )
                                this.phones[index] = value
                            })
                    }, (phone, index) => `${index}-${phone}`),
                    null)

                Flex({
                    direction: FlexDirection.Row,
                    justifyContent: FlexAlign.SpaceBetween
                }) {
                    Text("Save Changes")
                        .onClick(() => {
                            // copy values from local copy to the provided ref
                            // to Person object owned by parent Component.
                            // Avoid creating new Objects, modify properties of
                            // the existing ones:
                            this.selectedPerson.name           = this.name
                            this.selectedPerson.address.street = 
                                                       this.address.street
                            this.selectedPerson.address.city   = 
                                                         this.address.city
                            this.selectedPerson.address.zip    = 
                                                          this.address.zip
                            this.phones.forEach(
                                (phone : string, index : number) => {
                                this.selectedPerson.phones[index] = phone
                            })
                        })
                    Select(this.selectedPersonIndex() != -1,
                        Text("Delete Contact")
                            .onClick(() => {
                                let index = this.selectedPersonIndex()
                                console.log(`delete contact at index ${index}`)

                                // delete found contact
                                this.addrBook.contacts.splice(index, 1)

                                // determine new selectedPerson
                                index = (index < this.addrBook.contacts.length)
                                    ? index
                                    : index - 1

                                // if no contact left, set me as selectedPerson
                                this.selectedPerson = (index >= 0)
                                    ? this.addrBook.contacts[index]
                                    : this.addrBook.me
                            }),
                        null)
                }
            }
        }
    }

    @Component
    struct AddressBookView {

        @ObjectLink me        : Person
        @ObjectLink contacts  : ObservedArray<Person>
        @State selectedPerson : Person = null

        aboutToAppear() {
            this.selectedPerson = this.me
        }

        build() {
            Flex({
              direction: FlexDirection.Column,
              justifyContent: FlexAlign.Start
            }) {
                Text("Me:")
                PersonView({
                    person: this.me,
                    phones: this.me.phones,
                    selectedPerson: this.$selectedPerson})

                Divider().height(8)

                Flex({
                    direction: FlexDirection.Row,
                    justifyContent: FlexAlign.SpaceBetween
                }) {
                    Text("Contacts:")
                    Text("Add")
                        .onClick(() => {
                            this.selectedPerson = new Person (
                              "", "", 0, "", ["+86"]
                            )
                            this.contacts.push(this.selectedPerson)
                        })
                }.height(50)

                ForEach(this.contacts,
                    contact => {
                        PersonView({
                            person: contact,
                            phones: contact.phones,
                            selectedPerson: this.$selectedPerson
                        })
                    },
                    contact => contact.id_
                )

                Divider().height(8)

                Text("Edit:")
                PersonEditView({
                    selectedPerson: this.$selectedPerson,
                    name: this.selectedPerson.name,
                    address: this.selectedPerson.address,
                    phones: this.selectedPerson.phones
                })
            }
                .borderStyle(BorderStyle.Solid)
                .borderWidth(5)
                .borderColor(0xAFEEEE)
                .borderRadius(5)
        }
    }

    @Entry
    @Component
    struct PageEntry {
        @Provide addrBook : AddressBook = new AddressBook(
            new Person(
                "Mighty Panda",
                "Wonder str., 8",
                888,
                "Shanghai",
                ["+8611122223333", "+8677788889999", "+8655566667777"]
            ),
            [
                new Person(
                    "Curious Squirrel",
                    "Wonder str., 8",
                    888,
                    "Hangzhou",
                    ["+8611122223332", "+8677788889998", "+8655566667776"]
                ),
                new Person(
                    "Wise Tiger",
                    "Wonder str., 8",
                    888,
                    "Nanjing",
                    ["+8610101010101", "+8620202020202", "+8630303030303"]
                ),
                new Person(
                    "Mysterious Dragon",
                    "Wonder str., 8",
                    888,
                    "Suzhou",
                    [ "+8610000000000", "+8680000000000"]
                ),
            ]);

        build() {
            AddressBookView({
                me: this.addrBook.me,
                contacts: this.addrBook.contacts,
                selectedPerson: this.addrBook.me
            })
        }
    }
