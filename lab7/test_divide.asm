            .ORIG 0x3000

            LD      R0, X
            LD      R1, Y            
            JSR     f_div

            HALT


; ~~ data ~~
X          .FILL   5
Y          .FILL   2


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
