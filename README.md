# jank

jank is a simple, compiled programming language created for my own learning purposes. It is called "jank" for a reason: prior to this, I had never tried to create a programming language, so this is a learning experience for me.

## Features

- Simple syntax
- Compiled to a native executable (64-bit)
- Basic data types: integers, floats, strings
- Uses QBE as the backend for compilation

### Planned Features

- Functions
- Control flow (if/else, loops)
- Excellent C interoperability
- Standard library for common tasks

### Limitations

- Support for only 64-bit integers and floats
- No support for complex data structures (arrays, structs, etc.)
- No built-in error handling
- No support for concurrency or parallelism
- Not optimized for performance

## Installation

To install jank, clone the repository and build the project using the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

This will create a `jank` executable in the `build` directory.

## Usage

To compile a jank program, use the following command:

```bash
./jank <source_file.jank>
```

This will generate a QBE assembly file. You can then use the QBE compiler to generate a native executable:

```bash
qbe <assembly_file.qbe>
```

## Syntax

```rs
// This is a comment

// Define global variables
let x = 5; // 64-bit integer
let y = 3.14; // 64-bit float
let z = "Hello, World!"; // String

// Define a function
fn add(a, b) {
    return a + b; // Return the sum of a and b
}

// Main function
fn main() {
    // Call the add function and store the result
    let result = add(x, y);

    // Print the result
    print("The result is: " + result);
}
```
