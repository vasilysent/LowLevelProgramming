section .data
newline_char: db 10
for_read_char: db 0

section .text
 
; Gets exit code in rdi and exit 
exit: 
    mov rax, 60
    syscall
ret 

; Gets a pointer to a null-terminated string in rdi and returns its length in rax
string_length: 
    xor rax, rax
    .loop:
        cmp byte [rdi+rax], 0
        je .end
        inc rax 
        jmp .loop
    .end:
ret

; Gets a pointer to a null-terminated string in rdi and writes it in stdout
print_string: 
    push rax
    push rdi
    call string_length   
    pop rdi 
    mov rsi, rdi         ; string address
    mov rdi, 1           ; stdout descriptor  
    mov rdx, rax         ; string length in bytes
    mov rax, 1           ; 'write' syscall number
    push rcx
    syscall
    pop rcx
    pop rax
ret

; Gets symbol code in rdi and writes it in stdout
print_char: 
    mov rax, 1            ; 'write' syscall number
    push rdi
    mov rsi, rsp          ; string address
    mov rdi, 1            ; stdout descriptor
    mov rdx, 1            ; string length in bytes
    syscall               ; 'write' syscall
    pop rdi
ret

; New line (writes 0xA (10) simbol)
print_newline: 
    mov rax, 1            
    mov rdi, 1            
    mov rsi, newline_char 
    mov rdx, 1            
    syscall 
ret

; Prints unsigned integer in decimal format from rdi
print_uint: ;OK
    push rdx
    push rdi
    push rax
    push r10
    mov r8, rsp
    mov rax, rdi
    mov r10, 10
    dec rsp
    mov byte[rsp], 0    ; adding null-terminator to print the number as a string
    .loop:
        xor rdx, rdx
        div r10         ; divides rax by 10, writes remainder in rdx
        dec  rsp
        add  dl, '0'    ; adding '0' ASCII code to the number                   ; rdx -- dl 
        mov  [rsp], dl                                                          ; rdx -- dl
        test rax, rax
    jnz  .loop
    mov rdi, rsp
    call print_string
    mov rsp, r8
    pop r10
    pop rax
    pop rdi
    pop rdx
ret

; Prints 8-byte unsigned integer in decimal format from rdi
print_int: 
    cmp rdi, 0
    jge print_uint
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
    jmp print_uint
ret

; Gets two pointers (in rdi and rsi) to null-terminated strings, returns 1 if they are equal and 0 otherwise 
string_equals: 
    push rdi 
    push rsi
    call string_length ; puts 1st string length in r8
    mov r8, rax
    pop rsi
    mov rdi, rsi
    push rsi
    push r8
    call string_length ; puts 2nd string length in rax
    pop r8
    pop rsi
    pop rdi
    cmp r8, rax
    jne .no
    xor rcx, rcx 
    .loop:
        mov al, [rsi + rcx]
        mov dl, [rdi + rcx]
        cmp al, dl
        jne .no
        inc rcx
        cmp rcx, r8
        jl .loop 
    xor rax, rax
    inc rax
ret
    .no: 
        xor rax, rax 
    ret

; Reads one symbol from stdin and returns it. Returns 0 if the stream has ended
read_char: 
    mov rax, 0 ; read
    mov rdi, 0 ; from stdin
    mov rdx, 1 ; one symbol
    mov rsi, for_read_char
    syscall
    cmp rax, -1
    je .end
    cmp rax, 10
    je .end
    cmp rax, 0
    je .end
    mov al, [for_read_char]
ret
    .end:
        xor rax, rax
    ret
 
; Gets: buffer's address in rdi, and its size rsi
; Reads in buffer a word from stdin skipping blank symbols in the begining
; Space - 0x20, tabs - 0x9 and periods - 0xA
; Sops and returns 0 if the word is too big for the buffer
; If succeed returns buffer's address in rax, and word length in rdx.
; Otherwise returns 0 in rax
; The function should add a null-terminator to the word

read_word: 
    xor rcx, rcx ;counter
    dec rsi 
    
    .init:
    push rcx
    push rdi
    push rsi
    call read_char
    pop rsi
    pop rdi
    pop rcx
    cmp al, 0x20
    je .init
    cmp al, 0x9
    je .init
    cmp al, 0xA
    je .init
    cmp al, 0
    je .wordend
    mov [rdi], al
    inc rcx
    jmp .inword

    .inword:
    cmp rcx, rsi
    jg .err
    push rcx
    push rdi
    push rsi
    call read_char
    pop rsi
    pop rdi
    pop rcx
    cmp al, 0x20
    je .wordend
    cmp al, 0x9
    je .wordend
    cmp al, 0xA
    je .wordend
    cmp al, 0
    je .wordend
    lea rdx, [rdi + rcx]
    mov [rdx], al
    inc rcx
    jmp .inword

    .wordend:
    lea rdx, [rdi + rcx]
    mov  al, 0
    mov [rdx], al
    mov rax, rdi
    mov rdx, rcx
    ret

    .err:
    xor rax, rax
    ret

 ; Gets a pointer to a null-terminated string in rdi and tries to read
; an unsigned integer from its beginning.
; Returns the number in rax and its length in rdx
; rdx = 0 in case of error
parse_uint: 
    xor rdx, rdx ;counter
    xor rax, rax
    xor rcx, rcx
    mov r10, 10
    mov cl, [rdi]
    cmp cl, '0'
    jl .err
    cmp cl, '9'
    jg .err
    sub cl, '0'
    add rax, rcx
    inc rdx
    jmp .innumber

    .innumber:
    mov cl, [rdi + rdx]
    cmp cl, '0'
    jl .end
    cmp cl, '9'
    jg .end
    push rdx
    mul r10  
    pop rdx
    sub cl, '0'
    add rax, rcx
    inc rdx
    jmp .innumber

    .err:
    xor rdx, rdx
    ret

    .end:
    ret

; Gets a pointer to a null-terminated string in rdi and tries to read
; an signed integer from its beginning.
; If there is a sign, spaces between it and the number are not allowed.
; Returns the number in rax and its length (including the sign if it is) in rdx 
; rdx = 0 in case of error
parse_int: 
    xor rcx, rcx
    mov cl, [rdi]
    cmp cl, '-'
    je .sign
    jmp parse_uint

    .sign:
    inc rdi
    push rdi
    call parse_uint
    pop rdi
    cmp rdx, 0
    je .err
    inc rdx
    neg rax
    ret

    .err:
    xor rdx, rdx
    ret

; Gets a pointer to a null-terminated string in rdi, buffer's address in rsi and its size rdx
; Copies the string to the buffer.
; Returns string's length in rax or 0 if is is too big 
string_copy:    
    push rdi
    push rsi
    push rdx
    call string_length ; string length in rax
    pop rdx
    pop rsi
    pop rdi
    cmp rax, rdx 
    jge .notfit
    xor rcx, rcx
    .loop:
        mov al, [rdi + rcx]
        mov  [rsi + rcx], al
        inc rcx 
        cmp rcx, rdx 
    jl  .loop
    mov byte[rsi + rdx + 1], 0 ; adding null-terminator to buffer
ret
.notfit:
    xor rax, rax
ret
