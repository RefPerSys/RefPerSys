# Refpersys Persistent Format

A key goal of Refpersys is persistence, and to this end, a suitable format for
persisting Refpersys objects is required. The design goals of this persistent
format are: 
  * it should be machine-friendly, i.e., conceptually simple to parse
  * it should be git-friendly
  * it should have a suitable trade-off between readability and space
    performance

Given these considerations, a format has been designed largely inspired by the
JSON format.

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


## Example

An example of an object persisted on to file would be as follows (with optional
whitespace):

```
oid : class @ mtime (
        name = ...,
        space = ...,
        comp = (
                oid,
                oid,
                ...
        ),
        pload = (
                oid,
                oid,
                ...
        ),
        attr = (
                oid = "for string",
                oid = 1234.5678,
                oid = {                 # for sets
                        oid,
                        oid,
                        ...
                },
                oid = [                 # for tuples
                        oid,
                        oid,
                        ...
                ]
        )
)
```

