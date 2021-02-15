# SIMD_Matrix
SIMD Matrix to Matrix Multiplication

## Overview and Prelimiaries

The SIMD code is designed for AVX and uses single point precision
floating point data values. The code runs both non-optimized
standard c++ code and SIMD-optimized code. The code is designed
to operate on any square matrix and perform matrix to matrix
multiplication. The assumtion is that both inputs are square
matrices of the same dimensions.

Note, in order to properly compile the program one of the 
following three commands need to be included in the compiler
instruction:

```
-march=sandybridge
-march=haswell
-march=knl
```

Below is the compiler line I used:

```
g++ simd_mul.cpp -g -Wpsabi -march=knl -o test.out
```

There is code to perform the 100000x100000 matrix operations, but
it took too long and required too much memory for me to run it
on my computer.

## Code Structure

In the main function I create a loop that three times to test
non optimized and SIMD optimized code with 1000x1000, 10000x10000,
and 100000x100000 sized matrices to get performance information.

The non-optimized matrix multiplication is done by looping over 
the rows and columns to get the correct output. It has a runtime
of O(n^3) where n is the size of the matrix.

The SIMD optimized code performs a transpose of the second matrix,
and then runs a SIMD optimized function which loops over the code 
similarly to the non-optimized function except for the fact that
SIMD instructions take 8-float/integer values to perform the calculations.
The following instructions load in the corresponding values to be multiplied:

```c++
__m256 a_line = _mm256_loadu_ps(&a[i][k]);
__m256 b_line = _mm256_loadu_ps(&b[j][k]);
```

With the appropriate data loaded in I can then multiply and add the
values to the running total:

```c++
result = _mm256_add_ps(result, _mm256_mul_ps(a_line, b_line));
```

SIMD data values store the floats in a fixed vector form and
add and multiply by the fixed positions, as seen below:

```
---------------------------------
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
---------------------------------
  |   |   |   |   |   |   |   |        -------------------------------------------------
  +   +   +   +   +   +   +   +  ----> | 1+1 | 2+2 | 3+3 | 4+4 | 5+5 | 6+6 | 7+7 | 8+8 |
  |   |   |   |   |   |   |   |        -------------------------------------------------
---------------------------------
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
---------------------------------


---------------------------------
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
---------------------------------
  |   |   |   |   |   |   |   |        -------------------------------------------------
  x   x   x   x   x   x   x   x  ----> | 1x1 | 2x2 | 3x3 | 4x4 | 5x5 | 6x6 | 7x7 | 8x8 |
  |   |   |   |   |   |   |   |        -------------------------------------------------
---------------------------------
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
---------------------------------
```

So if becomes more obvious that I am performing the same operations
on 8 values at a time, which is a significant speed up. But I need
to somehow sum up all of the values that are added up. I could do that 
by simply adding them all up in a running total, but instead I do it
with SIMD instruction listed below:

```c++
result = _mm256_hadd_ps(result, result);
result = _mm256_hadd_ps(result, result);
result[1] = result[4];
result = _mm256_hadd_ps(result, result);
```

The `_mm256_hadd_ps` instuction adds up values that are next to each other
horizontially and the `result[1] = result[4];` instruction just moves the
low 4 values total to front to be added with the high 4 values total.
Below is a visualization of how the `_mm256_hadd_ps` function works:

```
           __mm256 1                                    __mm256 2
---------------------------------           ---------------------------------
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |           | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |  
---------------------------------           ---------------------------------
  |   |   |   |   |   |   |   |               |   |   |   |   |   |   |   |
  --+--   --+--   --+--   --+--               --+--   --+--   --+--   --+--
    |       |       |       |                   |       |       |       |
    |       |       |       |   |----------------       |       |       |
    |       |       |       ----|---------------        |       |       |
    |       |       ------------|---------     |        |       |       |
    |       ------------        |  |-----|-----|--------|       |       |
    --------------     |     ---|  |     |     |     ------------       |
                 |     |     |     |     |     |     |     --------------
                 |     |     |     |     |     |     |     |
              -------------------------------------------------      
              | 1+2 | 3+4 | 1+2 | 3+4 | 5+6 | 7+8 | 5+6 | 7+8 |
              -------------------------------------------------
```

I also include some debugging code at the end of the function. 

## Output Results

Below is the output when it is run. I ran it twice an included both outputs:

```
RUN 1
---------------------
Matrix size: 1000x1000
SIMD Implementation Runtime: 11.582sec

Matrix size: 10000x10000
SIMD Implementation Runtime: 11226.322462sec

RUN 2
------------------------
Matrix size: 1000x1000
SIMD Implementation Runtime: 12.182sec

Matrix size: 10000x10000
SIMD Implementation Runtime: 12252.504719sec
```

I then run a comparison test once between non-optimized matrix
multiplication code and SIMD-optimized matrix multiplication code,
the results are below:

```
Comparison RUN
----------------
Matrix size: 1000x1000
Standard Implementation Runtime: 95.572037000sec
SIMD Implementation Runtime: 12.275941000sec
```

## Analysis
