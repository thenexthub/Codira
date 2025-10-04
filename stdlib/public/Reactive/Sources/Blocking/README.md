RxBlocking 
============================================================

Set of blocking operators for easy unit testing.

***Don't use these operators in production apps. These operators are only meant for testing purposes.***


```swift
extension BlockingObservable {
    public fn toArray() throws -> [E] {}
}

extension BlockingObservable {
    public fn first() throws -> Element? {}
}

extension BlockingObservable {
    public fn last() throws -> Element? {}
}

extension BlockingObservable {
    public fn single() throws -> Element? {}
    public fn single(_ predicate: @escaping (E) throws -> Bool) throws -> Element? {}
}

extension BlockingObservable {
    public fn materialize() -> MaterializedSequenceResult<Element>
}
```


