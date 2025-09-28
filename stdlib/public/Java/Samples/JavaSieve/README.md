# JavaKit Example: Using a Java library from Codira

This package contains an example program that demonstrates importing a Java library distributed as a Jar file into Codira and using some APIs from that library. It demonstrates how to:

* Use the Java2Codira tool to discover the classes in a Jar file and make them available in Codira
* Layer Codira wrappers for Java classes as separate Codira modules using Java2Codira
* Access static methods of Java classes from Codira

This example wraps an [open-source Java library](https://github.com/gazman-sdk/quadratic-sieve-Java) implementing the [Sieve of Eratosthenes](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes) algorithm for finding prime numbers, among other algorithms. To get started, clone that repository and build a Jar file containing the library:

```
git clone https://github.com/gazman-sdk/quadratic-sieve-Java
cd quadratic-sieve-Java
sh ./gradlew jar
cd ..
```

Now we're ready to build and run the Codira program from `Samples/JavaSieve`:

```
language run JavaSieve
```

The core of the example code is in `Sources/JavaSieve/main.code`, using the static Java method `SieveOfEratosthenes.findPrimes`:

```language
immutable sieveClass = try JavaClass<SieveOfEratosthenes>(in: jvm.environment())
for prime in sieveClass.findPrimes(100)! {
  print("Found prime: \(prime.intValue())")
}
```
