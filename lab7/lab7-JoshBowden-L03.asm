; CS 350, Fall 2016
;
; Lab 7 - Section L03
;
; by Josh Bowden and Taylor Berg
;

            .ORIG   x3000
    
            ; print(str_prompt)
            ;
            LEA     R0, str_prompt
            PUTS
            
            ; str = readln()
            ;
            JSR     f_readln    ; out R0 = start of stringz

            ; n = str_to_int(str)
                                 ; in  R0 = ptr to str
            JSR     f_str_to_int ; out R0 = n
            AND     R1, R0, R0  ; R1 = R0; R1 = n (save a copy)

            ; print("2 * ")
            LEA     R0, str_two_times
            PUTS

            ; str_n = str_to_int(n)
            AND     R0, R1, R1   ; in  R0 = n
            JSR     f_int_to_str ; out R0 = ptr to start of stringz

            ; print(str_n)
            PUTS                ; in R0 = already ptr to stringz

            ; print(" = ")
            LEA     R0, str_equ
            PUTS

            ; (n *= n) <=> (n = n + n)
            AND     R0, R1, R1  ; R0 = n
            ADD     R0, R0, R0  ; R0 += R0; n = n + n

            ; str_2n = str_to_int(2n)
                                 ; in  R0 = 2 * n
            JSR     f_int_to_str ; out R0 = ptr to start of stringz

            ; print(str_2n)
            PUTS                ; in R0 = already ptr to stringz

            ; exit
            HALT


; ~~ data ~~
str_prompt  .STRINGZ "Enter a number: "
str_two_times .STRINGZ "2 * "
str_equ     .STRINGz " = "

; ~~ constants ~~
N10         .FILL   10
ASCII_0     .FILL   48  ; '0' = 48
NEG_ASCII_0 .FILL   -48 ; -'0' = -48


; ~~ subroutines ~~

; f_int_to_str:
;   converts a integer to a string
;
; in  R0: the integer value
; out R0: address of the start of the stringz
;
; Notes:
;   R0 -> `x` as the integer value to convert
;   R1 -> used as:
;           * `N10` as constant of 10 for `f_div` call
;           * `i` from modulus of `f_div` call
;           * `c` as ASCII representation of `i`
;   R2 -> `ptr` as the current address in the result stringz
;   R3 -> '0' as the ASCII 0 (zero)
;
f_int_to_str
            ST      R1, IS_S1   ; save R1
            ST      R2, IS_S2   ; save R2
            ST      R3, IS_S3   ; save R3
            ST      R7, IS_S7   ; save R7

            LD      R3, ASCII_0 ; R3 = '0'

            ; set R2 (ptr) to the ending null-terminator of the buffer
            ; since we are working from rightmost digit to leftmost.
            ;
            ; we'll deincrement the ptr before we set the char so that
            ; the ptr will end on the last char added to the buffer
            ;
            LEA     R2, IS_buf_endz ; ptr = buf_endz

            ; always run the loop at least once since otherwise
            ; we don't output anything when x == 0
            ;
            BR      IS_loop_0

IS_loop     AND     R0, R0, R0  ; set CC to R0 (x)
            BRZ     IS_done     ; if R0 (x) == 0 then break

IS_loop_0                       ; in R0 = x
            LD      R1, N10     ; in R1 = 10
            JSR     f_div       ; out R0 = x / 10 (x)
                                ; out R1 = x % 10 (i)

            ADD     R1, R1, R3  ; R1 += R3; c = i + '0'
            ADD     R2, R2, -1  ; R2 -= 1; ptr -= 1
            STR     R1, R2, 0   ; M[R2+0] <- R1; *(ptr+0) = c     

            BR      IS_loop     ; continue loop

IS_done     AND     R0, R2, R2  ; out R0 = address of start of stringz

            LD      R1, IS_S1   ; restore R1
            LD      R2, IS_S2   ; restore R2
            LD      R3, IS_S3   ; restore R3
            LD      R7, IS_S7   ; restore R7

            RET

                        ; INT16_MAX = 32,767 (5 chars + '\0')
IS_buf      .BLKW   5   ;   5 chars
IS_buf_endz .BLKW   1   ; + 1 char for '\0'

IS_S1       .BLKW   1
IS_S2       .BLKW   1
IS_S3       .BLKW   1
IS_S7       .BLKW   1


; f_str_to_int:
;   converts a string to an integer
;
; in  R0: address to start of stringz
; out R0: integer value of string
;
; Notes:
;   R0 -> `acc` as accumulator
;   R1 -> `N10` as constant of 10 for call to `f_mul`
;   R2 ->  used as:
;          * `c` as current char
;          * `i` as integer value of current char
;   R3 -> `ptr` as address to the *last* character in string
;   R4 -> `-'0'` used for getting integer value of ASCII digit
;
f_str_to_int
            ST      R1, SI_S1   ; save R1
            ST      R2, SI_S2   ; save R2
            ST      R3, SI_S3   ; save R3
            ST      R4, SI_S4   ; save R4
            ST      R7, SI_S7   ; save R7
            
            AND     R3, R0, R0  ; move R0 (in ptr) to R3 (ptr)
            AND     R0, R0, 0   ; R0 = 0; acc = 0

            LD      R1, N10     ; const R1 = 10
            LD      R4, NEG_ASCII_0 ; R4 = -'0'

SI_loop     LDR     R2, R3, 0   ; R2 = *(R3+0); c = *(ptr+0)
            BRZ     SI_done     ; if R0 (c) == '\0' then break

            ADD     R2, R2, R4  ; R2 += R4; i = c + (-'0')
            
            ; shift the acc left over a decimal place (acc *= 10)
            ;
                                ; in  R0 = acc
                                ; in  R1 = 10
            JSR     f_mul       ; out R0 = acc * 10

            ; then add current value of char (i in R2)
            ;
            ADD     R0, R0, R2  ; R0 += R2; acc += i

            ADD     R3, R3, 1   ; incr ptr to move right in stringz
            BR      SI_loop     ; continue loop
            

SI_done                         ; out R0 = acc

            LD      R1, SI_S1   ; restore R1
            LD      R2, SI_S2   ; restore R2
            LD      R3, SI_S3   ; restore R3
            LD      R4, SI_S4   ; restore R4
            LD      R7, SI_S7   ; restore R7

            RET                 ; return

SI_S1       .BLKW   1
SI_S2       .BLKW   1
SI_S3       .BLKW   1
SI_S4       .BLKW   1
SI_S7       .BLKW   1


; f_readln:
;   reads input into a buffer until a new-line ('\n') is hit
;
; out R0: address of the start of the stringz buffer from  the input
; 
f_readln    ST      R2, RL_R2   ; save R2
            ST      R6, RL_R6   ; save R6
            ST      R7, RL_R7   ; save R7

            LEA     R6, RL_buf  ; R6 = addr of buffer
                                    
RL_loop     GETC                ; R0 = getc()
            OUT                 ; putchar(R0)
            AND     R1, R0, R0  ; R1 = R0 (save copy of getc())
            
            ; check if we hit end of line
            ;
            LD      R2, NEG_EOL ; R2 = -'\n'
            ADD     R0, R0, R2  ; R0 += R2 (input - '\n')
            BRZ     RL_done     ; if R0 == 0 (hit EOL) then break
            
            ; since getc() is not EOL, save it to the buffer
            ; original input is still in R0
            ;
            STR     R1, R6, 0   ; *(buffer addr) = input
            ADD     R6, R6, 1   ; incr buffer addr
            BR      RL_loop     ; continue loop to get next char
                                
RL_done     LEA     R0, RL_buf  ; set R0 (ret start) to addr of buffer

            LD      R2, RL_R2   ; restore R2
            LD      R6, RL_R6   ; restore R6
            
            LD      R7, RL_R7   ; restore R7
            RET                 ; return

RL_R2       .BLKW   1
RL_R6       .BLKW   1
RL_R7       .BLKW   1
            

EOL         .FILL   10   ; ASCII '\n'
NEG_EOL     .FILL   -10  ; ASCII '\n' except negative; used for comparison test
RL_buf      .BLKW   80


; f_mul:
;   implements software multiplication (a = a * b)
;
; in  R0: a
; in  R1: b
; out R0: a * b
;
; Notes:
;   R0 -> `a` as the mulitpler (since we branch on R0)
;   R1 -> `b` as the multiplicand that we deincrement
;   R2 -> accumulator
;
f_mul       ST      R2, MU_R2   ; save R2
            AND     R2, R2, 0   ; R2 = 0 (reset accumulator)
            
MU_loop     AND     R0, R0, R0  ; set CC from R0 (mulitpler)
            BRZ     MU_done     ; if R0 (multipler) == 0 then break
            ADD     R2, R2, R1  ;   R2 (acc) += R1 (multiplicand)
            ADD     R0, R0, -1  ;   R0-- (multipler)
            
            BR      MU_loop     ; continue loop

MU_done     AND     R0, R2, R2  ; set result to accumulator
            LD      R2, MU_R2   ; restore R2
            RET                 ; return

MU_R2       .BLKW   1


; f_div:
;   implements software integer division with modulus (q = x / y and r = x % y)
;   only supports positive numbers (by lab specification)
;
; in  R0: x (dividend) for x >= 0
; in  R1: y (divisor) for y > 0 
; out R0: q = x / y (quotient)
; out R1: r = x % y (remainder)
;
; Notes:
;   R0 -> `x` (dividend/remainder)
;   R1 -> -`y` (divisor) as constant that we use to subtract from `x`
;   R2 -> `q` (quotient) as counter for number of divisions
;   R3 -> temp for comparison
;
f_div       ST      R2, DV_R2   ; save R2
            ST      R3, DV_R3   ; save R3

            AND     R2, R2, 0   ; R2 (q) = 0
            
            ; negate `y` (~R1 + 1)
            NOT     R1, R1      ; R1 = ~R1
            ADD     R1, R1, 1   ; R1 += 1

DV_loop     AND     R3, R3, 0   ; R3 (comparison) = 0
            ADD     R3, R0, R1  ; compare if rem < divisor aka rem + (-y)
            BRN     DV_done     ; if rem <= 0 then break
            ADD     R0, R0, R1  ; R0 (rem) += R1 (-y)
            ADD     R2, R2, 1   ; R2 (q) += 1
            BR      DV_loop     ; continue loop

DV_done     AND     R1, R0, R0  ; R1 (ret r) = R1 (rem)
            AND     R0, R2, R2  ; R0 (ret q) = R2 (q)
            
            LD      R2, DV_R2   ; restore R2
            LD      R3, DV_R3   ; restore R3
                
            RET

DV_R2       .BLKW 1
DV_R3       .BLKW 1


            .END

