global notec
extern debug
default rel

align 8
section .data
        spinlock1 dq 0          ; spinlock for entering init section, only one thread inits bss array
        spinlock2 dq 1          ; spinlock for other threads waiting for end of initialization

align 8
section .bss
        waiting_for resq N      ; number of the partner that notec is waiting for
        swap_values resq N      ; each notec under its index puts value for its partner
                                ; each notec has its value and partner number under the index equal to its number
                                ; default value N in waiting_for means that only owner can access its fields
                                ; changing waiting_for to other value 'x' means notec with number x
                                ; can now access fields under this index

; Because .bss and .data variables are global we store thread local variables on stack at following addresses:
; [r12] - n (our notec number)
; [r12 - 8] - address of the start of the calc string
; [r12 - 16] - length of calc
; [r12 - 24] - determines mode, 1 - number entering mode, 0 - mode inactive
; [r12 - 32] - place to store calc string counter during call to debug function

section .text
notec:
        push r12                ; save r12
        push rdi                ; save our number
        mov r12, rsp            ; save address of the first stack variable
        push rsi                ; and calculation string start address
        mov rdi, rsi            ; put start for length calculations
        call calculate_length   ; result in rcx
        cmp rcx, 0              ; check for empty string
        jz loop_end
        push rcx                ; save length
        push qword 0            ; number entering mode = false
        push qword 0            ; place for counter
        call init_bss
        xor rcx, rcx            ; set counter for iteration over string

loop_over_calc:
        xor rdx, rdx
        mov rsi, [r12 - 8]      ; get string start address
        mov dl, [rsi + rcx]     ; prepare character value for comparisons

        mov r10, '0'            ; prepare range for testing
        mov r11, '9'
        call if_in_range
        jz parse_num

        mov r10, 'a'            ; prepare range for testing
        mov r11, 'f'
        call if_in_range
        jz parse_hex

        mov r10, 'A'            ; prepare range for testing
        mov r11, 'F'
        call if_in_range
        jz parse_hex_capital

        cmp dl, '='             ; compare with consecutive values and jump to appropriate code if true
        jz num_mode_end
        cmp dl, '+'
        jz sum
        cmp dl, '*'
        jz product
        cmp dl, '-'
        jz neg
        cmp dl, '&'
        jz and
        cmp dl, '|'
        jz or
        cmp dl, '^'
        jz xor
        cmp dl, '~'
        jz not
        cmp dl, 'Z'
        jz delete
        cmp dl, 'Y'
        jz duplicate
        cmp dl, 'X'
        jz swap
        cmp dl, 'N'
        jz push_N
        cmp dl, 'n'
        jz push_n
        cmp dl, 'g'
        jz call_debug
        cmp dl, 'W'
        jz multithread_swap

num_mode_end:
        mov qword[r12 - 24], 0  ; set num_mode variable to false - that is the end of number
loop_condition:
        inc rcx
        cmp rcx, [r12 - 16]
        jnz loop_over_calc      ; if counter does not equal length then continue loop else end
loop_end:
        mov rax, [rsp]          ; return value from the stack's top
        lea rsp, [r12 + 8]      ; shift top of the stack to return address of notec function
        pop r12                 ; restore r12 from before notec call
        ret

; modifies registers rax, rcx, rdi
init_bss:
        mov rax, 1
        xchg rax, [spinlock1]
        cmp rax, 0                  ; 0 means lock open, 1 means lock closed
        jnz init_wait               ; only first thread will init bss section array
        xor rcx, rcx                ; set counter for iteration over [waiting_for]
        lea rdi, [waiting_for]      ; get array start
loop_bss_array:
        mov qword[rdi + 8 * rcx], N ; setting field in array to N
        inc rcx
        cmp rcx, N
        jnz loop_bss_array
        mov qword[spinlock2], 0     ; releasing lock
init_wait:
        mov rax, [spinlock2]        ; other threads have to wait for init completion
        cmp rax, 0                  ; threads are released when first thread ends loop
        jnz init_wait
        ret

parse_num:
        sub rdx, '0'
        jmp new_number
parse_hex:
        sub rdx, 'a' - 10
        jmp new_number
parse_hex_capital:
        sub rdx, 'A' - 10
new_number:
        cmp qword[r12 - 24], 0  ; check if we are already in number entering mode
        jnz already_num_mode
        mov qword[r12 - 24], 1  ; set number entering mode to true
        push rdx                ; push new number if we are not in number entering mode
        jmp loop_condition
already_num_mode:
        shl qword[rsp], 4       ; move already present number 4 bits left
        add [rsp], rdx
        jmp loop_condition

sum:
        call pop_twice
        add r10, r11
        push r10
        jmp num_mode_end

product:
        call pop_twice
        imul r10, r11
        push r10
        jmp num_mode_end

neg:
        neg qword[rsp]
        jmp num_mode_end

and:
        call pop_twice
        and r10, r11
        push r10
        jmp num_mode_end

or:
        call pop_twice
        or r10, r11
        push r10
        jmp num_mode_end

xor:
        call pop_twice
        xor r10, r11
        push r10
        jmp num_mode_end

not:
        not qword[rsp]
        jmp num_mode_end

delete:
        pop r11
        jmp num_mode_end

duplicate:
        push qword[rsp]
        jmp num_mode_end

swap:
        call pop_twice
        push r10
        push r11
        jmp num_mode_end

push_N:
        push N
        jmp num_mode_end

push_n:
        push qword[r12]     ; push this notec number
        jmp num_mode_end

call_debug:
        mov rdi, [r12]      ; prepare first call argument - our number
        mov rsi, rsp        ; and second call argument - stack pointer
        mov [r12 - 32], rcx ; save rcx because function debug is allowed to modify it
        test spl, 0x0f      ; check if last 4 bits of rsp are empty
        jz already_aligned  ; if not then align stack top to 16
        push qword 0
        call debug
        pop rcx             ; pop alignement
        jmp after_call
already_aligned:
        call debug
after_call:
        mov rcx, 8          ; multiply stack top shift by number of bytes in 64bit
        mul rcx
        add rsp, rax        ; move top of stack by number returned by debug
        mov rcx, [r12 - 32] ; restore counter
        jmp num_mode_end


multithread_swap:
        mov rsi, [r12]      ; get our notec number
        pop rdx             ; get second notec number

        lea rdi, [waiting_for]
busy_wait_self:
        mov rax, [rdi + 8 * rsi]    ; acquire OUR notec lock
        cmp rax, N                  ; check if lock is unobtained (N means that is free)
        jnz busy_wait_self
        lea rdi, [swap_values]      ; get swap values array start
        pop qword[rdi + 8 * rsi]    ; set our value for swap
        lea rdi, [waiting_for]
        mov [rdi + 8 * rsi], rdx    ; set notec number that we are waiting for and release lock at the same time

        lea rdi, [waiting_for]
busy_wait_second:
        mov rax, [rdi + 8 * rdx]    ; acquire SECOND notec lock
        cmp rax, rsi                ; check if second notec is waiting for us
        jnz busy_wait_second
        lea rdi, [swap_values]      ; get swap values array start
        push qword[rdi + 8 * rdx]   ; make swap - put second notec value to its field in array
        lea rdi, [waiting_for]
        mov qword[rdi + 8 * rdx], N ; release SECOND notec lock (N means it is free)
        jmp num_mode_end


; pops two values from stack, puts first in r10, second in r11, modifies rdx
pop_twice:
        pop rdx                 ; pop our return address
        pop r10                 ; pop first value
        pop r11                 ; second value
        push rdx                ; restore return address
        ret

; receives arguments in: rdx - value to be tested, r10 - range beginning(inclusive), r11 - range end(inclusive)
; r10 is modified, rdx is NOT modified
if_in_range:                ; result in ZF, if ZF = 1 then value is in range else value out of range
        cmp rdx, r10            ; compare lower end
        jb return_false
        cmp rdx, r11            ; compare higher end
        ja return_false
return_true:
        xor r10, r10            ; sets ZF to 1
        ret
return_false:
        add r10, 1              ; sets ZF to 0
        ret

; string start should be passed in rdi, register rax will be modified
calculate_length:           ; result in ecx
        xor     rcx, rcx        ; zero repne output register
        not     ecx             ; and set to max value
        xor     al, al          ; set al to match NULL character in the end of string

        cld                     ; clear flag
        repne scasb             ; find end of string, inc edi, dec ecx by the string length
        not     ecx             ; negate by bits because str_len + 1 = max_value - ecx (before this instruction)
        dec     ecx             ; and decrement to get string length
        ret
