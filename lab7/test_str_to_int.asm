            
            .ORIG   x3000

            LEA     R0, str_45   ; in  R0 = ptr to stringz
            JSR     f_str_to_int ; out R0 = 45

            HALT

; ~~ constants ~~
;
str_45      .STRINGZ "45"
N10         .FILL   10
ASCII_0     .FILL   48  ; '0' = 48
NEG_ASCII_0 .FILL   -48 ; -'0' = -48


; ~~ subroutines ~~
;

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


            .END
