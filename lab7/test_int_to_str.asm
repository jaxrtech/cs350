            
            .ORIG   x3000

            LD      R0, N45      ; in  R0 = n
            JSR     f_int_to_str ; out R0 = ptr to start of stringz
            PUTS

            HALT

; ~~ constants ~~
N45         .FILL   45
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
            ST     R1, IS_S1   ; save R1
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
            BRNZ    DV_done     ; if rem <= 0 then break
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
