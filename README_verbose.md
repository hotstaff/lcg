# Logistic Chaos bit string Generator (LCG)

Implementation example of embedding original data into unestimateable random data set.

# Introduction
This program is a sample implementation of the Logistic Chaos Bit Generation Algorithm (LCG).
The logistic map is an example of the simplest chaotic map, and that is represented by the following recursion formula

	x_[n + 1] = a * x_[n] (1 - x_[n]).

Here, the expression:

	x_[n + 1]

 represents the (n + 1) th value of x, and the range of the value of x is 0 to 1.
It is well known that the orbit of the solution obtained by computing this recurrence formula bifurcates into a non-periodic extremely complex behavior called chaos.
If you calculate while changing the initial values of a and x, you can draw the famous beautiful chaotic bifurcation diagram.
In particular, when the value of a is 4, it is known as pure chaos, and the orbit of x in the map is known to change almost randomly.
This program focuses on this randomness and applies the logistic map in the case of a = 4 to bit string generation and data hiding.

# How to generate bit string

Bit string generation (decode) is very simple.
Starting from the initial value x0, put x obtained by the logistic map into the following simple judgment formula

	if (logistic(x) > 0.5) {
		return 1;
	} else {
		return 0;
	}

and decode into binary data.
Starting from x0, generate bits by applying logistic mapping iteratively.
The bit length generated from one initial value x0 is called block length and is expressed in units of 8 bits = 1 byte.
That is, if the initial value x0 is determined, a bit string of an arbitrary length can be obtained by logistic mapping.
This operation is very fast.

# Determination of initial value x0

In order to resolve (encode) x0 from the bit string contrary to bit generation, it takes time because the bit string can only be calculated from the initial value x0 round robin.
Also, in theory, this block length can be extended infinitely, but there is no guarantee that x0 will always be found within the computer's initial value accuracy.
Therefore, in this program, 2 bytes (16 bits) is specified as the default.

In this condition, the initial value of x0 is represented by 8 bytes, and the data length that obtained from x0 is 2 bytes.
That is, unfortunately four times the original data length.
If the initial value x0 can be expressed with a size smaller than the block size, the data is compressed, but it is not realistic on current computers.

In the future, if an environment in which the search for x0 can be performed at a high speed with the quantum computer etc. is achieved, it is considered that a huge bit string can be represented by the initial value x0 of a small data volume.
It is noted here, data compression may be possible using this method.

## Supplement
There is a special solution

	x[n] = 1/2 (1-Cos[2^n ArcCos[1-2 x0]])

for a = 4 as you know.
However, if you try to plot this function, you can see immediately that x oscillates rapidly between 0 and 1 for large n.
Therefore, it is expected that it is difficult to solve x0 numerically while maintaining valid accuracy for large n.

# Application to something like cryptography

Although the bit string generated from the initial value x0 is uniquely determined, there are a plurality of initial values x0 that can generate a bit string.
Also, the initial bit sensitivity makes the original bit string and the initial value x0 irrelevant information.
This means hiding the structure of the data in a chaos map.
However, if the data is known to be the initial value of the logistic map, the attacker can quickly decode it. In other words, it has not been encrypted at this point.

# More safely, random XOR

Therefore, this problem is solved by XOR operation with random data

	bin = x0 ^ random.

Here, x0 is a bit string representing x0 which can generate bits obtained by encoding, and random is a bit string of the same length as x0 obtained from the random number generator.
If you want to restore x0, you can use the identity

	x0 = bin ^ random

that holds simultaneously because of the nature of XOR operation.
x0 can be restored if there is one pair of bin and random.
This operation converts the original data into a seemingly random set of data. This means that we can not restore without two files.
This method is fundamental to cryptography but is a very effective key generation method. But it's a huge key.
Even with the shortcomings of pseudorandom number generators, the sensitivity of the underlying chaotic map makes estimating the underlying data more difficult.
In summary, it is almost impossible to estimate the original data by hiding the chaotic map with random data.

# Build
```
make
```

# Install
```
sudo make install
```