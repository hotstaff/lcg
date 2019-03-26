# Logistic Chaos bit Generator (LCG)

Implementation example of embedding original data into unestimateable random data set.

# Overview

This program focuses on the randomness of chaotic maps and applies the a = 4 logistic map to bit string generation and data hiding.

Please read __doc/README\_verbose.md__ for more details.

## Decode

It's very fast.

## Encode

It takes time because the bit string can only be calculated from the initial value x0 round robin.

## Benefit

By hiding the bit string in the chaos map and performing random XOR operation, the original data can not be estimated.


# To Build

```
./autogen.sh
./congigure
make
```

# Install

```
sudo make install
```

# Usage

## Encode

```
lcg input.bin output.bin -s 2
```
-s 2 option means the block size 2bytes(16bits).

## Decode

```
lcg -d input.bin output.bin -s 2
```

## Encode and create decrypt key

```
lcg input.bin output.bin -s 2 -k decrypt.key
```

The program create decrypt.key.

### Note

It is not encryption, an XOR with just random data.

## Decode with decrypt key

```
lcg -d input.bin output.bin -s 2 -k decrypt.key
```

# Licence

The MIT License (MIT)
Copyright (c) 2019 Hideto Manjo

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
