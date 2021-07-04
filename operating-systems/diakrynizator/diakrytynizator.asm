global _start

section .rodata
        sys_read equ 0
        sys_write equ 1
        sys_exit equ 60
        stdin equ 0
        stdout equ 1
        mod equ 0x10FF80            ; modulo value for polynomial
        unicode_max equ 0x10FFFF    ; max unicode value that symbols can have
        line_feed equ 0x0A          ; unicode LF value
        zero equ 0x30               ; unicode 0 value
        nine equ 0x39               ; unicode 9 value
        upper_bound equ 0x7F
        constant equ 0x80           ; constant used with polynomial
        buf_read_size equ 2048
        buf_write_limit equ 2044    ; buffer has size 1024, we want to flush before it overflows

section .data
        iter dq 2047                ; variable for iteration over read buffer
        read_bytes_num dq 2048      ; variable for storing number of bytes read by sys function
        char db 0                   ; variable for passing one char of input from buffer
        write_count dq 0            ; counter for putting chars into write buffer
                                    ; it's value is equal to first free position on buffer as
                                    ; well as current length of output

section .bss
        buffer_read resb 2048       ; buffer for reading
        buffer_write resb 2048      ; buffer for writing

section .text
_start:
        call    parse_args
        call    manage_symbols
        jmp     exit_success


parse_args:
        pop     r11             ; pop return address
        pop	    r8              ; popping number of arguments
        dec     r8              ; decrease count of args
        pop	    rsi             ; popping first arg - program name
        push    r11             ; push back return address

        cmp     r8, 0           ; check if number of remaining args equals 0
        jz      exit_err        ; if yes then exit_err because we need at least parameter a0 for polynomial

        mov     r9, 0           ; set counter for iterating over arguments (counting up)

_parse_args_loop1:
        mov     rdi, [rsp + 8 * r9 + 8]; get argument's start address
        mov     rdx, rdi        ; save argument's start because rdi will be modified in calculations
        call    calculate_length; calculate length (result in ecx)
        mov     rdi, rdx        ; retrieve argument's start

        mov     rsi, rcx        ; set iterator for string (counting down from the end of argument)
        xor     rdx, rdx        ; prepare register for result
        mov     rcx, 1          ; prepare register that will store current power of 10

; registers r8 (number of args), r9 (counter for loop over args), rsi (iter of arg string),
; rcx (current power of 10), rdi (argument's start) are occupied and rdx is set to store current result
_parse_args_loop2:          ; result integer in rdx
        dec     rsi             ; decrement iterator at the start of every loop

        xor     rax, rax        ; clear rax before any operation on its subregisters
        mov     al, [rdi + rsi] ; get next char of argument

        cmp     al, zero        ; check if symbol is smaller than '0' (we accept leading zeros)
        jb      exit_err        ; if smaller then symbol is incorrect (exit error) else continue

        cmp     al, nine        ; check if symbol is greater than '9'
        ja      exit_err        ; if greater then symbol is incorrect (exit error) else contionue

        sub     al, zero        ; change register value to correspond stored symbol
        call    safe_mul        ; multiply rax by current power of 10
        jo      exit_err        ; check overflow flag and exit if given argument is greater than register range
        add     rdx, rax        ; add value to result register
        jo      exit_err        ; check overflow flag and exit if given argument is greater than register range

        mov     rax, rcx        ; move current power of 10 to acumulator
        mov     rcx, 10         ; init rcx with 10
        call    safe_mul        ; calculate next power of 10 in rax
        mov     rcx, rax        ; move multiplication result to right register

        cmp     rsi, 0          ; check if we reached beginning of argument string
        jnz     _parse_args_loop2; jump to the beginning of the loop if not
_parse_args_loop2_end:          ; still in loop1

        mov     [rsp + 8 * r9 + 8], rdx; put polynomial coefficient on stack

        inc     r9                  ; increment counter after every loop
        cmp     r9, r8              ; check if counter equals number of args
        jnz     _parse_args_loop1   ; if not then continue loop
_parse_args_loop1_end:

        ret                         ; return from parse_args

; in advance we calculate all arguments modulo 0x10ff80
; modifies r11, should get first argument for multiplication in rax, second in rcx
safe_mul:                   ; result like in standard mul - in rax
        mov     r11, rdx        ; move current result to other register because mul will modify rdx
        mul     rcx             ; multiply by value in rcx
        push    rdi             ; preserve rdi
        call    modulo          ; calculate argument mod modulo_value, pass arg in rax, result in rdx
        pop     rdi             ; retrieve rdi
        mov     rax, rdx        ; put calcualted value in return register
        mov     rdx, r11        ; restore result value
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


; number of bytes read is returned in read_bytes_num
; reads one byte of input
getchar:                        ; byte of input is returned in [char]
        push    rax                 ; save registers
        push    rcx
        push    rdx

        inc     qword[iter]
        mov     rcx, [iter]         ; set register for comparison
        cmp     rcx, [read_bytes_num]; if not end of the buffer then just return next char
        jnz     ret_next_char       ; else read new input

        mov     rax, sys_read       ; we call sys_read
        mov     rdi, stdin          ; read from stdin
        mov     rsi, buffer_read    ; store input in buffer
        mov     rdx, buf_read_size  ; of buf_size
        syscall
        mov     [read_bytes_num], rax; save number of bytes read
        mov     qword[iter], 0      ; set iterator for buffer

ret_next_char:
        mov     rcx, [iter]         ; get iter
        mov     dl, [buffer_read + rcx]
        mov     [char], dl          ; put next value for return

        pop     rdx                 ; restore registers from stack
        pop     rcx
        pop     rax
        ret


convert_utf8_to_unicode:        ; result in rcx
        xor     rcx, rcx            ; register for final unicode result
        xor     rax, rax
        xor     rdx, rdx            ; two registers to store initial utf8 encoding
        xor     rsi, rsi            ; one more register for boundaries check
        mov     ah, [char]          ; move first utf8 value

_utf8_uni_1byte:
        cmp     ah, 0x80
        jae     _utf8_uni_2byte
        mov     cl, ah
        ret

_utf8_uni_2byte:
        cmp     ah, 0xe0
        jae     _utf8_uni_3byte

        call    getchar         ; get second utf8 value
        mov     al, [char]

        cmp     ax, 0xc1bf      ; check utf8 boundaries
        jbe     exit_err        ; symbol x must satisfy: 0xc1bf < x <= 0xdfbf to be valid unicode encoding
        cmp     ax, 0xdfbf
        ja      exit_err

        sub     ah, 0xc0        ; first 8 bits - we take last 5 bits - 000xxxxx
        sub     al, 0x80        ; second 8 bits - we take last 6 bits - 00xxxxxx

        shl     al, 2           ; fit to left - xxxxxx00
        mov     cx, ax          ; now we have 000xxxxx xxxxxx00
        shr     ecx, 2          ; shift back - now 00000xxx xxxxxxxx
        ret

_utf8_uni_3byte:
        cmp     ah, 0xf0
        jae     _utf8_uni_4byte

        call    getchar         ; get second utf8 value
        mov     al, [char]
        call    getchar         ; get second utf8 value
        mov     dh, [char]

        mov     si, ax          ; put values in one register and check utf8 boundaries
        shl     esi, 16         ; symbol x must satisfy: 0xe09fbf < x <= 0xefbfbf to be valid unicode encoding
        mov     si, dx
        shr     esi, 8
        cmp     esi, 0xe09fbf
        jbe     exit_err
        cmp     esi, 0xefbfbf
        ja      exit_err

        sub     ah, 0xe0        ; we take last 4 bits - 0000xxxx
        sub     al, 0x80        ; second 8 bits - we take last 6 bits - 00xxxxxx
        sub     dh, 0x80        ; third 8 bits - we take last 6 bits - 00xxxxxx

        shl     al, 2           ; fit to left - xxxxxx00
                                ; and in 16bit - 0000xxxx xxxxxx00

        mov     cx, ax          ; now we have 0000xxxx xxxxxx00
        shl     ecx, 6          ; now 00000000 000000xx xxxxxxxx 00000000

        shl     dh, 2           ; in dh - xxxxxx00

        mov     cl, dh          ; now 00000000 0000xxxx xxxxxxxx xxxxxx00
        shr     ecx, 2          ; final shift - we have 00000000 00000000 xxxxxxxx xxxxxxxx

        ret

_utf8_uni_4byte:
        cmp     ah, 0xf8
        jae     exit_err

        call    getchar        ; get second utf8 value
        mov     al, [char]
        call    getchar        ; get second utf8 value
        mov     dh, [char]
        call    getchar        ; get second utf8 value
        mov     dl, [char]

        mov     si, ax          ; put values in one register and check utf8 boundaries
        shl     esi, 16         ; symbol x must satisfy: 0xf08fbfbf < x <= 0xf8bfbfbf to be valid unicode encoding
        mov     si, dx
        cmp     esi, 0xf08fbfbf
        jbe     exit_err
        cmp     esi, 0xf8bfbfbf
        ja      exit_err

        sub     ah, 0xf0        ; we take last 3 bits - 00000xxx
        sub     al, 0x80        ; second 8 bits - we take last 6 bits - 00xxxxxx
        sub     dh, 0x80        ; third 8 bits - we take last 6 bits - 00xxxxxx
        sub     dl, 0x80        ; fourth 8 bits - we take last 6 bits - 00xxxxxx

        shl     al, 2           ; to left - xxxxxx00 and 16bit - 00000xxx xxxxxx00
        shl     dl, 2           ; to left - xxxxxx00 and 16bit - 00xxxxxx xxxxxx00
        shl     dx, 2           ; in dx - xxxxxxxx xxxx0000

        mov     cx, ax          ; now we have 00000000 00000000 00000xxx xxxxxx00
        shl     ecx, 14         ; now 0000000x xxxxxxxx 00000000 00000000

        mov     cx, dx          ; now 0000000x xxxxxxxx xxxxxxxx xxxx0000
        shr     ecx, 4          ; final shift - we have 00000000 000xxxxx xxxxxxxx xxxxxxxx

        ret


; transforms one unicode symbol - we get unicode value in rcx
transform_symbol:           ; result in rsi
        cmp     rcx, unicode_max; compare value with given max
        ja      exit_err        ; if value is greater then symbol is incorrect so exit error

        mov     rsi, rcx        ; put result in case value below 0x80
        cmp     rcx, upper_bound; compare with upper bound
        jbe     _transform_symbol_end   ; if value is smaller then omit calculations

        mov     r9, 0           ; set counter for looping over args in polynomial calcualation
        sub     rcx, constant   ; subtract constant to get x - 0x80
        mov     r10, 1          ; prepare x^n counter, start with x^0
        xor     rsi, rsi        ; prepare register for result

_transform_loop:
        mov     rax, r10        ; move x^n to accumulator
        mul     qword[rsp + 8 * r9 + 16] ; multiply by argument
        ;(+16 because top is transform_symbol ret and top + 8 is manage_symbols ret)
        call    modulo          ; now in rdx we have (an * x^n) mod mod_value

        add     rdx, rsi        ; add to current result - we have in rdx an * x^n + ... + a0)
        mov     rax, rdx        ; prepare paremeter for module
        call    modulo          ; calculate modulo - in rdx (rdx an * x^n + ... + a0) mod mod_value
        mov     rsi, rdx        ; move to result register

        mov     rax, r10        ; move x^n to accumulator
        mul     rcx             ; multiply to get x^(n+1) for next loop
        call    modulo          ; calculate modulo
        mov     r10, rdx        ; copy calculated x^(n+1)

        inc     r9              ; increment counter
        cmp     r9, r8          ; compare with number of arguments
        jnz     _transform_loop ; if not equal then continue polynomial calcualation
_transform_loop_end:

        add     rsi, constant   ; add 0x80 to result register
_transform_symbol_end:
        ret                     ; return from transform_symbol


; value must be in rax before call, rdi is modified
modulo:                     ; result in rdx
        xor     rdx, rdx        ; clear remainder register
        mov     rdi, mod        ; prepare divisor
        div     rdi             ; divide
        ret


convert_unicode_to_utf8:
        mov     rcx, rsi        ; we need access to both 8bit registers
        xor     rdx, rdx        ; additional register

_uni_utf8_1byte:
        cmp     rsi, 0x7F       ; for ascii characters conversion is straightforward
        ja      _uni_utf8_2byte

        mov     eax, ecx
        mov     rdx, 1          ; move number of bytes to rdx for syscall
        ret

_uni_utf8_2byte:
        cmp     rsi, 0x7FF
        ja      _uni_utf8_3byte

        shl     cx, 2               ; 00000xxx xxxxxxxx -> 000xxxxx xxxxxx00
        shr     cl, 2               ; -> 000xxxxx 00xxxxxx
        mov     dh, cl
        mov     dl, ch              ; in dx - 00xxxxxx 000xxxxx (little endian)

        add     dx, 0x80c0          ; 10xxxxxx 110xxxxx
        mov     eax, edx ; move to buffer
        mov     rdx, 2              ; move number of bytes to rdx for syscall
        ret

_uni_utf8_3byte:
        cmp     rsi, 0xFFFF
        ja      _uni_utf8_4byte

        shl     ecx, 4              ; 00000000 00000000 xxxxxxxx xxxxxxxx -> 00000000 0000xxxx xxxxxxxx xxxx0000
        shr     cx, 2               ; -> 00000000 0000xxxx 00xxxxxx xxxxxx00
        shr     cl, 2               ; -> 00000000 0000xxxx 00xxxxxx 00xxxxxx
        mov     dh, cl
        mov     dl, ch              ; in dx - 00xxxxxx 00xxxxxx (little endian)

        shl     edx, 8              ; in edx - 00000000 00xxxxxx 00xxxxxx 00000000
        shr     ecx, 8              ; in ecx - 00000000 00000000 0000xxxx 00xxxxxx
        mov     dl, ch              ; in edx - 00000000 00xxxxxx 00xxxxxx 0000xxxx

        add     edx, 0x8080e0       ; 00000000 10xxxxxx 10xxxxxx 1110xxxx
        mov     eax, edx ; move to buffer
        mov     rdx, 3              ; move number of bytes to rdx for syscall
        ret

_uni_utf8_4byte:
        cmp     rsi, 0x10FFFF

        shl     ecx, 4              ; 00000000 000xxxxx xxxxxxxx xxxxxxxx -> 0000000x xxxxxxxx xxxxxxxx xxxx0000
        shr     cx, 2               ; -> 0000000x xxxxxxxx 00xxxxxx xxxxxx00
        shr     cl, 2               ; -> 0000000x xxxxxxxx 00xxxxxx 00xxxxxx
        mov     dh, cl
        mov     dl, ch              ; in dx - 00xxxxxx 00xxxxxx (little endian)

        shl     edx, 16             ; in edx - 00xxxxxx 00xxxxxx 00000000 00000000
        shr     ecx, 14             ; in ecx - 00000000 00000000 00000xxx xxxxxx00
        shr     cl, 2               ; in ecx - 00000000 00000000 00000xxx 00xxxxxx
        mov     dh, cl
        mov     dl, ch              ; in edx - 00xxxxxx 00xxxxxx 00xxxxxx 00000xxx (little endian)

        add     edx, 0x808080f0     ; 10xxxxxx 10xxxxxx 10xxxxxx 11110xxx
        mov     eax, edx ; move to buffer
        mov     rdx, 4              ; move number of bytes to rdx for syscall
        ret

; before call rdx should contain number of characters to display
putchar_32bit:
        mov     rdi, [write_count]  ; save current length/first free byte in buffer
        add     [write_count], rdx  ; add to count length of new char
        mov     [buffer_write + rdi], eax ; put new char into buffer using value saved in rdi to calcualte position
        cmp     qword[write_count], buf_write_limit ; check if we are above limit for output length was reached
        jbe     return              ; if not then return else flush buffer
flush:  ; rdx should contain number of chars to display
        mov     rax, sys_write      ; writing symbols which were transformed with polynomial
        mov     rdi, stdout         ; write to stdout
        mov     rdx, [write_count]  ; move length of output
        mov     rsi, buffer_write   ; from same buffer
        syscall
        mov     qword[write_count], 0; reset counter
return:
        ret


manage_symbols:
_manage_symbols_loop:
        call    getchar                 ; get char every loop cycle
        cmp     qword[read_bytes_num], 0; if sys_read returned 0 bytes - end
        jz      _manage_symbols_loop_end

        call    convert_utf8_to_unicode ; returned value in rcx
        call    transform_symbol        ; returned value in rsi
        call    convert_unicode_to_utf8 ; returned value in [buffer_write]
        call    putchar_32bit

        jmp     _manage_symbols_loop
_manage_symbols_loop_end:

        ret


exit_success:
        call    flush               ; flush before exit
        mov     rax, sys_exit       ; exit 0 when program is successful
        mov     rdi, 0
        syscall


exit_err:
        call    flush               ; flush before exit
        mov     rax, sys_exit       ; exit 1 if some errors occured
        mov     rdi, 1
        syscall
