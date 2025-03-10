# Zyclops C++ Library

Zyclops is a C++ library for working with hypercomplex numbers and matrices. It provides a set of classes and functions to handle various algebraic structures and layouts efficiently.

## Features

- Support for hypercomplex numbers and matrices
- Flexible algebraic structures (Real, Complex, Dual, Split, Quaternions, etc.)
- Multiple matrix layouts (Native, Inset, Block)
- Efficient operations on hypercomplex arrays
- Template-based implementation for type flexibility

## Main Components

### 1. Zomplex

`Zomplex` is a structure that represents hypercomplex number/array information. It includes:

- Algebraic family (Real, Complex, Dual, etc.)
- Order of the hypercomplex number
- Layout for hypercomplex arrays

### 2. Zmat

`Zmat` is a class that represents hypercomplex matrices or vectors. It provides:

- Matrix dimensions (rows and columns)
- Algebraic family and order
- Layout information

### 3. Utility Functions

The library includes various utility functions for:

- Hypercomplex array operations (addition, subtraction, multiplication, division)
- Multidual Cauchy-Riemann (CR) matrix operations
- TODO Power and exponent calculations

## Usage

To use the Zyclops library, include the main header file in your C++ project:

```cpp
#include "zyclops.hpp"
Then, you can create and manipulate hypercomplex numbers and matrices using the provided classes and functions.

## Requirements
C++11 or later
OpenMP support (optional, for parallel processing)

## TODO Installation


## TODO License
[Insert your chosen license information here]

## TODO Contributing
[Insert contribution guidelines or contact information for potential contributors]

## TODO Authors
[Insert author information here]

## TODO Acknowledgments
[Insert any acknowledgments or credits here]