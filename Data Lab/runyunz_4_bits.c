/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here> RUNYUN ZHANG - runyunz
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* implement bitAnd with ~ and | */
  //return 2;
  return ~(~x | ~y);
}
/* 
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int copyLSB(int x) {
  /* left shift then right shift */
  //return 2;
  return ((x << 31) >> 31);
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
int leastBitPos(int x) {
  /* bitAnd x and its 2's compliment */
  //return 2;
  return (x & (~x + 1));
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /* generate a mask using << and >>, then bitAnd */
  //return 2;
  return ~(((1 << 31) >> n) << 1) & (x >> n);
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /* create 5 masks in words and count bits using divide and conquer */
  int mask_1, mask_2, mask_3, mask_4, mask_5;
  /* mask_1 = 0x55555555 = 0b0101...0101 */
  mask_1 = 0x55 + (0x55 << 8);
  mask_1 = mask_1 + (mask_1 << 16);
  /* mask_2 = 0x33333333 = 0b0011...0011 */
  mask_2 = 0x33 + (0x33 << 8);
  mask_2 = mask_2 + (mask_2 << 16);
  /* mask_3 = 0x0F0F0F0F = 0b00001111...00001111 */
  mask_3 = 0xF + (0xF << 8);
  mask_3 = mask_3 + (mask_3 << 16);
  /* mask_4 = 0x00FF00FF = 0b0000000011111111... */
  mask_4 = 0xFF + (0xFF << 16);
  /* mask_5 = 0x0000FFFF */
  mask_5 = 0xFF + (0xFF << 8);

  /* count bits using divide and conquer */
  x = (x & mask_1) + ((x >> 1) & mask_1);
  x = (x & mask_2) + ((x >> 2) & mask_2);
  x = (x & mask_3) + ((x >> 4) & mask_3);
  x = (x & mask_4) + ((x >> 8) & mask_4);
  x = (x & mask_5) + ((x >> 16) & mask_5);
  return x;
  //return 2;
}
/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  /* return 0x7FFFFFFF */
  //return 2;
  return ~(1 << 31);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  /* 
   * negative: (x + (2^k-1)) / 2^k 
   * positive: x / 2^k
   */
  return (x + (((1 << n) + ~0) & (x >> 31))) >> n;  
  //return 2;
}
/* 
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNonNegative(int x) {
  /* x >> 31, get -1 when negative and 0 otherwise */
  //return ((x >> 31) + 1);
  return !(x >> 31);
  //return 2;
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  /*
   * same sign: compare x & y via (y + ~x + 1)
   * different sign: negative sign is smaller
   */
  int xSign = x >> 31; /* 0 if x nonnegative or -1 if x negative */
  int ySign = y >> 31; /* 0 if y nonnegative or -1 if y negative */
  int xorRes = xSign ^ ySign; /* 0 if same sign; -1 if different*/
  int valRes = (y + ~x + 1) >> 31; /* -1 if x > y or 0 if x <= y */
  /* res = -1 if x > y or 0 if otherwise */
  /* res = valRes if xorRes = 0 (same sign) or ySign if xorRes = -1 (different)*/
  int res = (valRes & ~xorRes) + (ySign & xorRes);
  return (~res+1);  
  //return 2;
}
/* 
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
  /* use mask and XOR */
  //return 2;
  int xSign = x >> 31;
  return (x + xSign) ^ xSign; //((x ^ xSign) + (xSign & 1));
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) {
  /*
   * if x is power of 2 then (x & (x-1)) = 0
   */

  /* cond_1 = 0 if x is power of 2 including x = 0 and x = 1<<31 */
  int cond_1 = x & (x + ~0); 
  /* cond_2 = 0 if x = 0 or x = 1<<31 */
  int cond_2 = x << 1; 

  return !(cond_1 | (!cond_2));
} 
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  /* 
   * connect sign, exp and frac bits together, consider round issues 
   */
  const int BIAS = 127;
  const int FRAC_BITS = 23;
  const int OTHER_BITS = 9; /* 32 - 23 = 9 */
  const int INT_MIN = 0x80000000; 
  
  const unsigned MASK_FRAC = 0x7FFFFF; /* (1 << FRAC_BITS) - 1; format:000[9]111[23] */
  const unsigned MASK_TRUNC = 0x1FF; /* (1 << OTHER_BITS) - 1; format:000[23]111[9] */
  const unsigned TRESHOLD = 0x100; /* 1 << (OTHER_BITS - 1); format:000[23]1000[8] */

  unsigned res = 0;
  unsigned exp = x ? 158 : 0; /* when x is 1<<31 or 0; 158 = BIAS + 31 */
  unsigned frac = 0;
  int countE = 0;
  unsigned trunc, temp;

  /* set sign bit s */
  res = x & INT_MIN; /* 1 << 31 */

  /* deal with x when x is not 0 or 1<<31 */
  if (x << 1) {
      /* deal with negative */
      //x = x & 0x7FFFFFFF;
      if (x < 0) {
      	  x = -x;
      }
      /* find most significant 1 bit */ 
      temp = x;
      while (temp >>= 1) {
	    countE += 1;
      }

      /* compute exp and frac */
      exp = countE + BIAS;
      x <<= (32 - countE); /* leave all frac from the very left */
      frac = MASK_FRAC & (x >> OTHER_BITS); /* leave 23 bits frac */
      trunc = MASK_TRUNC & x;
      
      /* pay attention to round issues for frac */
      if ((trunc > TRESHOLD) || ((trunc == TRESHOLD) && (frac & 0x1))) {
           frac += 1;
	   /* pay attention to round issues for exp */
	   if (frac == 0x800000) { /* 0x800000 = 1 << FRAC_BITS */
	       exp++;
	       frac = 0;
	   }
       }

  }

  return (res | exp << FRAC_BITS | frac);
  //return 0;
}
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  /*
   * nan: exp = 0xff and frac > 0
   */
  unsigned ans = uf & 0x7FFFFFFF; /* 0x7FFFFFFF = (1U << 31) - 1 */
  return (ans > 0x7F800000 ? uf : ans); /* 0x7F800000 = +inf */
  //return 2;
}
