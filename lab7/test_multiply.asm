			.ORIG 0x3000

			AND		R0, R0, 0
			ADD		R0, R0, 5

			AND		R1, R1, 0
			ADD		R1, R1, 9
			
			JSR		Multiply

			HALT

; Multiply:
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
Multiply    ST      R2, MU_R2   ; save R2
            AND     R2, R2, 0   ; R2 = 0 (reset accumulator)
            
MU_loop     AND     R0, R0, R0  ; set CC from R0 (mulitpler)
            BRZ     MU_done     ; if R0 (multipler) == 0 then break
            ADD     R2, R2, R1  ;   R2 (acc) += R1 (multiplicand)
            ADD     R0, R0, -1  ;   R0-- (multipler)
            
            BR      MU_loop     ; continue loop

MU_done     AND     R0, R2, R2  ; set result to accumulator
            LD      R2, MU_R2   ; restore R2
	        RET

MU_R2       .BLKW   1

			.END