# JavaKit Example: Using Java APIs from Codira

This package contains an example program that uses Java's [`java.math.BigInteger`](https://docs.oracle.com/javase/8/docs/api/?java/math/BigInteger.html) from Codira to determine whether a given number is probably prime. You can try it out with your own very big number:

```
language run JavaProbablyPrime <very big number>
```

The package itself demonstrates how to:

* Use the Java2Codira build tool plugin to wrap the `java.math.BigInteger` type in Codira.
* Create an instance of `BigInteger` in Codira and use its `isProbablyPrime`.
