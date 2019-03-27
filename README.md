# Logistic Chaos bit Generator (LCG)

Implementation example of embedding original data into unestimateable random data set.

## Overview

This program focuses on the randomness of chaotic maps and applies the a = 4 logistic map to bit string generation and data hiding.

Please read __README-verbose.md__ for more details.

### Decode

It's very fast.

### Encode

It takes time because the bit string can only be calculated from the initial value x0 round robin.

### Benefit

By hiding the bit string in the chaos map and performing random XOR operation, the original data can not be estimated.


## To Build

```
./autogen.sh
./configure
make
```

## Install

```
sudo make install
```

## Clean

```
git clean -d -f
```
The state of the directory is restored before running ./autogen.sh.

## Usage

### Encode

```
lcg input.bin output.bin -s 2
```
-s 2 option means the block size 2bytes(16bits).

### Decode

```
lcg -d input.bin output.bin -s 2
```

### Encode and create decrypt key

```
lcg input.bin output.bin -s 2 -k decrypt.key
```

The program create decrypt.key.

#### Note

It is not encryption, an XOR with just random data.

### Decode with decrypt key

```
lcg -d input.bin output.bin -s 2 -k decrypt.key
```

## Licence

lcg is released under the terms of the MIT license. See COPYING for more information or see https://opensource.org/licenses/MIT.
