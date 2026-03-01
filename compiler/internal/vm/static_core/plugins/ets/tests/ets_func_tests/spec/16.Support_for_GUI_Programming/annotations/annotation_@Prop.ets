/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
 desc: 04.02.05 Scenario - Class type @Prop synced from @State array item in parent component
 name: 04.managing_state_owned_by_a_component/04.02_component_state_uni-directionally_synced_from_parent_component_@Prop/04.02.05_scenario_-_Class_type_@Prop_synced_from_@State_array_item_in_parent_component/annotation_@Prop
 tags: [compile-only]
 ---*/

let nextId : number = 1;

@Observed
class Book {
  public id : number;
  public title: string;
  public pages: number;
	public readIt : boolean = false;

  constructor(title: string, pages: number) {
    this.id = nextId++;
    this.title = title;
    this.pages = pages;
  }
}

@Component
struct ReaderComp {
	@Prop book: Book;

	build() {
		Row() {
			Text(this.book.title)
			Text(` ... has ${this.book.pages} pages!`)
			Text(` ... ${this.book.readIt ? "I have read" : "I have not read it"}`)
			  .onClick(() => this.book.readIt = true )
		}
	}
}

@Entry
@Component
struct Library {
	@State allBooks: Book[] = [ new Book("100 secrects of C++", 765), new Book("Effective C++", 651), new Book("The C++ programming language", 1765) ];

	build() {
    Column() {
			Text(`Library's all time favorite`)
			ReaderComp({ book: this.allBooks[2] })
			
			Divider()
			
			Text(`Books on loan to a reader`)
		    ForEach(this.allBooks, 
                book => {
                    ReaderComp({ book: book })
                },
                book => book.id
            )

            Button(`Add new`)
            .onClick(() => {
                this.allBooks.push(new Book("The C++ Standard Library", 512));
            })
            Button(`Remove first book`)
            .onClick(() => {
                this.allBooks.shift();
            })
            Button(`Mark unread for everyone`)
            .onClick(() => {
                this.allBooks.forEach((book) => book.readIt = false)
            })
        }
    }
}