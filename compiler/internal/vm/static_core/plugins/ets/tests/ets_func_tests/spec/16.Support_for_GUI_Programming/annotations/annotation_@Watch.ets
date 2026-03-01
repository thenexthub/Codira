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
 desc: 06.01.02 @Watch example with @Link
 name: 06.other _state_management_functionality/06.01_get_notified_of_state_variable_changes_@Watch/06.01.02_@Watch_example_with_@Link/annotation_@Watch
 tags: [compile-only]
 ---*/

class PurchaseItem {
  static NextId : number = 0;

  public id: number;
  public price: number;

  constructor(price : number) {
    this.id = PurchaseItem.NextId++;
    this.price = price;
  }
}

@Component
struct BasketViewer {

  @Link @Watch("onBasketUpdated") shopBasket: PurchaseItem[];
  @State totalPurchase : number = 0;

  updateTotal() : number {
    let total = this.shopBasket.reduce((sum, i) =>  sum + i.price , 0);

    // Apply discount if over 100EUR
    if (total >= 100) {
      total = 0.9 * total;
    }
      return total;
  }
    
  // @Watch cb
  onBasketUpdated(propName: string) : void {
    this.totalPurchase = this.updateTotal();
  }

  build() {
    Column() {
      ForEach(this.shopBasket,
        (item) => {
          Text(`Price: ${item.price.toFixed(2)} €`)
        },
        item => item.id.toString()
      )
      Text(`Total: ${this.totalPurchase.toFixed(2)} €`)
    }
  }
}


@Entry
@Component
struct BasketModifier {

  @State shopBasket : PurchaseItem[] = [];
    build() {
    Column() {
      Button("Add to basket")
        .onClick(() => { this.shopBasket.push(new PurchaseItem(Math.round(100 * Math.random()))) })
        BasketViewer({ shopBasket: this.$shopBasket })
    }
  }
}