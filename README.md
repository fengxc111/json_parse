# json_parse
--参考

[从零开始的JSON库教程](https://github.com/miloyip/json-tutorial/)  
[ECMA-404](https://www.ecma-international.org/wp-content/uploads/ECMA-404_2nd_edition_december_2017.pdf)  
[RFC 7159](https://www.rfc-editor.org/rfc/rfc7159#section-7)  

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
> surrounding zero or more name/value pairs (or members).  A name is a
> string.  A single colon comes after each name, separating the name
> from the value.  A single comma separates a value from a following
> name. *The names within an object SHOULD be **unique**.*
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
>   总结一下： 必须是10进制数，不能有前导0或是'+'(正号)；浮点数必须有整数和小数部分；可用科学计数法，其幂部分不能有前导0，但是能有'+'(正号)；不能有表示除数为0(nan)或是溢出(inf)的符号
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
>   A string begins and ends with quotation marks.
>   1. If the character is in the Basic Multilingual Plane (U+0000 through U+FFFF), then it may be
>   represented as a six-character sequence: a reverse solidus, followed by the lowercase letter u, followed by four 
>   hexadecimal digits that encode the character's code point.
>   2. Alternatively, there are two-character sequence escape representations of some popular characters.
>   3. To escape an extended character that is not in the Basic Multilingual Plane, the character is represented as a 12-
>   character sequence, encoding the UTF-16 surrogate pair.
>
>   总结一下：字符串必须以*双引号*(")开头和结尾；  
>   1.转义utf-8字符，格式为6字节序列：1个\(反斜杠)，1个u(小写的字母u)，4位十六进制数字，其中的字母可大写可小写  
>       &emsp;eg："\u005C"  
>   2.转义特殊含义字符，格式为2字节序列：1个\(反斜杠)，1个需要被转义的字符  
>       &emsp;eg："\\"， "\""  
>   3.转义宽字节字符(例如utf-16四字节模式)，格式为12字节序列：1个\(反斜杠)，1个u(小写的字母u)，4位十六进制数字，
>   1个\(反斜杠)，1个u(小写的字母u)，4位十六进制数字，十六进制的字母可大写可小写  
>       &emsp;eg： "\uD834\uDD1E"  
>
>   string = quotation-mark *char quotation-mark
>
>      char = unescaped /  
>           escape (  
>                %x22 /          ; "    quotation mark  U+0022  
>                %x5C /          ; \    reverse solidus U+005C  
>                %x2F /          ; /    solidus         U+002F  
>                %x62 /          ; b    backspace       U+0008  
>                %x66 /          ; f    form feed       U+000C  
>                %x6E /          ; n    line feed       U+000A  
>                %x72 /          ; r    carriage return U+000D  
>                %x74 /          ; t    tab             U+0009  
>                %x75 4HEXDIG )  ; uXXXX                U+XXXX  
>
>      escape = %x5C              ; \  
>
>      quotation-mark = %x22      ; "  
>
>      unescaped = %x20-21 / %x23-5B / %x5D-10FFFF  
