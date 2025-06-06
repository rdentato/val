Below is the IEEE 754 double‐precision (“double”) representation

---

A double is stored in 64 bits, partitioned as follows:

* **Sign bit**:

  $$
    s \;=\; b_{63} \quad\bigl(\text{0 for +, 1 for –}\bigr)
  $$
* **Exponent field** (11 bits):

  $$
    e \;=\; \sum_{i=0}^{10} b_{52+i}\,2^{\,i} \quad\text{(unsigned integer between 0 and 2047)}
  $$
* **Fraction (mantissa) field** (52 bits):

  $$
    f \;=\; \sum_{i=1}^{52} b_{52-i}\,2^{-\,i} \quad\bigl(\text{interpreted as a fraction in }[0,1)\bigr)
  $$

Here, $b_{63}$ is the most‐significant bit (the sign bit), bits $b_{62}$ down to $b_{52}$ form the exponent, and bits $b_{51}$ down to $b_{0}$ form the fraction.

The **bias** for the 11‐bit exponent is

$$
  \text{Bias} \;=\; 2^{10} - 1 \;=\; 1023.
$$

---

## 1. Normalized values ($1 \le e \le 2046$)

When $1 \le e \le 2046$, the encoded value is

$$
  \text{value} \;=\; (-1)^{s} \times \Bigl(1 + f\Bigr) \times 2^{\,e - 1023}.
$$

Equivalently, plugging in the definitions of $e$ and $f$:

$$
  \text{value}
  \;=\; (-1)^{b_{63}}
  \;\times\;
  \Biggl(
    1 \;+\; \sum_{i=1}^{52} b_{52-i}\,2^{-\,i}
  \Biggr)
  \;\times\;
  2^{\Bigl(\sum_{i=0}^{10} b_{52+i}\,2^{i}\Bigr)\;-\;1023}.
$$

---

## 2. Subnormal (denormal) values ($e = 0,\ f \neq 0$)

When $e = 0$ and $f \neq 0$, the exponent is treated as $1 - \text{Bias} = 1 - 1023 = -1022$, but there is **no implicit leading 1** in the significand. Thus:

$$
  \text{value}
  \;=\; (-1)^{s} \times f \times 2^{-1022}
  \;=\;
  (-1)^{b_{63}}
  \;\times\;
  \Bigl(\sum_{i=1}^{52} b_{52-i}\,2^{-\,i}\Bigr)
  \times
  2^{-1022}.
$$

---

## 3. Zero ($e = 0,\ f = 0$)

When $e = 0$ and $f = 0$, the value is

$$
  (+0)\quad\text{if }s = 0,\qquad
  (-0)\quad\text{if }s = 1.
$$

That is,

$$
  \text{value} = (-1)^{s} \times 0.
$$

---

## 4. Infinities ($e = 2047,\ f = 0$)

When $e = 2047$ (all exponent bits = 1) and $f = 0$, the value is

$$
  +\infty \quad\text{if }s = 0,\qquad
  -\infty \quad\text{if }s = 1.
$$

Equivalently:
$\text{value} = (-1)^{s} \times \infty.$

---

## 5. NaNs (“Not a Number”) ($e = 2047,\ f \neq 0$)

When $e = 2047$ and $f \neq 0$, the encoding represents a NaN. There are two flavors:

* **Quiet NaN (qNaN)** if the most‐significant fraction bit ($b_{51}$) is 1.
* **Signaling NaN (sNaN)** if $b_{51} = 0$.

But in both cases, the value is simply “NaN” and does not correspond to a real number.

---

### Summary Table

| Exponent field $e$ |     Fraction $f$    |                       Interpretation                       |                   Value formula                  |
| :----------------: | :-----------------: | :--------------------------------------------------------: | :----------------------------------------------: |
| $1 \le e \le 2046$ | any ($0 \le f < 1$) |                       **Normalized**                       | $\displaystyle (-1)^s \,(1 + f)\,2^{\,e - 1023}$ |
|       $e = 0$      |      $f \neq 0$     |                  **Subnormal (denormal)**                  |       $\displaystyle (-1)^s \,f\,2^{-1022}$      |
|       $e = 0$      |       $f = 0$       |                          **Zero**                          |          $\displaystyle (-1)^s \times 0$         |
|     $e = 2047$     |       $f = 0$       |                        **Infinity**                        |       $\displaystyle (-1)^s \times \infty$       |
|     $e = 2047$     |      $f \neq 0$     | **NaN** (quiet or signaling, depending on MSB of fraction) |                  $\mathrm{NaN}$                  |

---

#### Notation recap

* $s = b_{63}$ is the sign bit (0 = +, 1 = –).
* $e = \sum_{i=0}^{10} b_{52+i}\,2^{i}$ is the unsigned integer value of the 11 exponent bits.
* $f = \sum_{i=1}^{52} b_{52-i}\,2^{-\,i}$ is the fractional part (mantissa) in $[0,1)$.
* Bias = 1023.

Putting it all together, the core formula for a normalized double-precision value is:

$$
  \boxed{
    \text{(for }1 \le e \le 2046\text{)} 
    \quad
    \text{value} \;=\; 
    (-1)^{s} 
    \;\times\; 
    \bigl(1 + f\bigr) 
    \;\times\; 
    2^{\,e - 1023}
  }
$$
