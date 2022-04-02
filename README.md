This is an implementation of the lox language built in the brilliant book "Crafting Interpreters" by Bob Nystrom. Used modern c++ & functional data structures.

The build outputs two binaries.

- ilox: The interpreted lox language implementation.
- vlox: The VM based lox language implementation.

To build

```
mkdir build && cd build
cmake ..
make
```

To run
```
./ilox <script>.lox
```
Or
```
./vlox <script>.lox
```
