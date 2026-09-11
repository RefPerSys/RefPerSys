# Refpersys Persistent Format

A key goal of Refpersys is persistence, and to this end, a suitable format for
persisting Refpersys objects is required. The design goals of this persistent
format are: 
  * it should be machine-friendly, i.e., conceptually simple to parse
  * it should be git-friendly
  * it should have a suitable trade-off between readability and space
    performance

Given these considerations, the textual format for RefPerSys store is
an enhanced version of [JSON](https://json.org/). A space file is a
sequence of JSON objects but can (and usually do) contain comments as
lines starting with two slashes ie `//`.


# OBSOLETE DOCUMENTATION BELOW

## EBNF Grammar

  * Each store document must contain at least *one* **object**.  
  * Each **object** must be defined with an **oid**, a **class**, an **mtime**; 
    its properties are enclosed in parentheses; each **object** must have a **name**, 
    and optionally the **space**, **component**, **payload**, and **attribute** 
    properties.  
  * An **oid** comprises of a random number expressed in base 62 and prefixed by
    an underscore.  
  * a **base 62 digit** consists of the integers 0-9, the lowercase letters a-z, 
    and the uppercase letters A-Z.  
  * a **class** is an **oid**.  
  * an **mtime** is a floating point number.  
  * a **name** is a string of Unicode characters.  
  * a **component** consists of **oid**s separated by commas, except the last
    one, and enclosed within parentheses.  
  * a **payload** consists of **oid**s separated by commas, except the last one,
    and enclosed within parentheses.


**document** = **object**, { **object** } ;

**object** = **oid**, ":", **class**, "@", **mtime**, "(", **name**, [ "," ],
           [ space ], [ "," ], [ **component** ], [ "," ], [ **payload** ], [ "," ], 
           [ **attribute** ], ")" ;

**oid** = "_", **base 62 digit**, { **base 62 digit** } ;

**base 62 digit** = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
                  | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j"
                  | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t"
                  | "u" | "v" | "w" | "x" | "y" | "z"
                  | "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J"
                  | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T"
                  | "U" | "V" | "W" | "X" | "Y" | "Z" ;

**class** = **oid** ;

**mtime** = [ floating point number ] ;

**name** = { unicode character } ;

**component** = "comp", "=", "(", **oid**, [ "," ], { (**oid**, [ "," ]) }, ")" ;

**payload** = "pload", "=", "(", **oid**, [ "," ], { (**oid**, [ "," ]) }, ")" ;

**attribute** = "attr", "=", { **string** }, [ "," ], { **double** }, [ "," ],
              { **set** }, [ "," ], { **tuple** } ;

**string** = **oid**, "=", unicode character, { unicode character } ;

**double** = **oid**, "=", floating point number ;

**set** = **oid**, "=", "{", **oid**, [ "," ], { **oid** }, "}" ;

**tuple** = **oid**, "=", "[", **oid**, [ "," ], { **oid** }, "]" ;


## Example

An example of an object persisted on to file would be as given below.
Whitespaces are optional, and are included only for improving readability.
Whitespace characters include spaces, tabs, and carriage returns. In our format,
sets are enclosed in braces, and tuples in brackets. `object` would be the
starting symbol.

```
object = (
        id = _07YGv5jImHw03JSziA,
        class = _0OE0PJ2Nihp00j9HvR,
        mtime = _87QUZDREppL01GjRJx,
        name = "sample_object_for_example",
        comp = (_04YQEJtbxJ704mmY6j, _8WQ1IHwxDOf02U1G9I),
        load = (_90WQpRgSoN8018gLbJ, _3EMmRTwppRq00YOpJZ),
        attr = (
                _4cG7qyo0038046cBsf = "string", # for string
                _1OlebU0NNqG03H0h1E = 1234.567, # for double
                #for set
                _2EXYXvXKfNL04gTxus = {_4eK0IPW6HT604zz8s6, _0A5YBlxJgIM03Hs6f9},
                #for tuple
                _0j2JzHrzA0604avLAv = [_5JSE68ZYK4003UVdVc, _73UWhRTzDSw0373KVm],
        )
)
```

