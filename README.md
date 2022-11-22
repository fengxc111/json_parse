# json_parse
--参考[从零开始的JSON库教程](https://github.com/miloyip/json-tutorial/)

## json语法
  > JSON-text = ws value ws
  > 
  >   These are the six structural characters:
  > 
  >     begin-array     = ws %x5B ws  ; [ left square bracket
  > 
  >     begin-object    = ws %x7B ws  ; { left curly bracket
  >
  >     end-array       = ws %x5D ws  ; ] right square bracket
  >
  >     end-object      = ws %x7D ws  ; } right curly bracket
  >
  >     name-separator  = ws %x3A ws  ; : colon
  >
  >     value-separator = ws %x2C ws  ; , comma
  >> Insignificant whitespace is allowed before or after any of the six structural characters.
  >>
  >>      ws = *(
  >>
  >>               %x20 /              ; Space
  >>
  >>               %x09 /              ; Horizontal tab
  >>
  >>               %x0A /              ; Line feed or New line
  >>
  >>               %x0D )              ; Carriage return

## 值
> A JSON value MUST be an object, array, number, or string, or one of
>   the following three literal names:
>
>      false null true
>
>   The literal names MUST be lowercase.  No other literal names are
>   allowed.
>
>      value = false / null / true / object / array / number / string
>
>      false = %x66.61.6c.73.65   ; false
>
>      null  = %x6e.75.6c.6c      ; null
>
>      true  = %x74.72.75.65      ; true

## 对象
> An object structure is represented as a pair of curly brackets
>   surrounding zero or more name/value pairs (or members).  A name is a
>
>   string.  A single colon comes after each name, separating the name
>   from the value.  A single comma separates a value from a following
>
>   name. *The names within an object SHOULD be **unique**.*
>
>      object = begin-object [ member *( value-separator member ) ]
>               end-object
>
>      member = string name-separator value

## 数组
> An array structure is represented as square brackets surrounding zero
>   or more values (or elements).  Elements are separated by commas.
>
>       array = begin-array [ value *( value-separator value ) ] end-array
>
>   *There is no requirement that the values in an array be of the same
>   type.*

## 数字
> The representation of numbers is similar to that used in most programming languages.
> A number is represented in base 10 using decimal digits.  It contains an integer component that may be
>   prefixed with an **optional minus sign**, which may be followed by a
>   **fraction part** and/or an **exponent part**.  Leading zeros are not allowed.
>
>   A fraction part is a decimal point followed by one or more digits.
>
>   An exponent part begins with the letter E in upper or lower case,
>   which may be followed by a plus or minus sign.  The E and optional
>   sign are followed by one or more digits.
>
>   Numeric values that cannot be represented in the grammar below (such as Infinity and NaN) are not permitted.
>
>      number = [ minus ] int [ frac ] [ exp ]
>
>      decimal-point = %x2E       ; .
>
>      digit1-9 = %x31-39         ; 1-9
>
>      e = %x65 / %x45            ; e E
>
>      exp = e [ minus / plus ] 1*DIGIT
>
>      frac = decimal-point 1*DIGIT
>
>      int = zero / ( digit1-9 *DIGIT )
>
>      minus = %x2D               ; -
>
>      plus = %x2B                ; +
>
>      zero = %x30                ; 0

## 字符串
