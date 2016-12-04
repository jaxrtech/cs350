
            .ORIG   0x3000

            LEA     R0, Prompt
            PUTS

            JSR ReadLine
            HALT


; ~~ data ~~
Prompt .STRINGZ "> "


; ReadLine:
;   reads input into a buffer until a new-line ('\n') is hit
;
; out R0: address of the start of the stringz buffer for the input
; 
ReadLine    ST      R1, RL_R1   ; save R1
            ST      R2, RL_R2   ; save R2
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
            
            BR      RL_loop     ; get next char
                                
RL_done     LEA     R0, RL_buf  ; set return to addr of buffer

            LD      R1, RL_R1   ; restore R1
            LD      R2, RL_R2   ; restore R2
            LD      R6, RL_R6   ; restore R6
            
            LD      R7, RL_R7   ; restore R7
            RET                 ; return

RL_R1       .BLKW   1
RL_R2       .BLKW   1
RL_R6       .BLKW   1
RL_R7       .BLKW   1
            

EOL         .FILL   10   ; ASCII '\n'
NEG_EOL     .FILL   -10  ; ASCII '\n' except negative; used for comparison test
RL_buf      .BLKW   80
            
            .END
            