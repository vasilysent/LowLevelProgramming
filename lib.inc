section .data
newline_char: db 10
for_read_char: db 0

section .text
 
; Gets exit code in rdi and exit 
exit: ;OK
    mov rax, 60
    syscall
ret 

; Gets a pointer to a null-terminated string in rdi and returns its length in rax
string_length: ;OK
    xor rax, rax
    .loop:
        cmp byte [rdi+rax], 0
        je .end
        inc rax 
        jmp .loop
    .end:
ret

; ; Gets a pointer to a null-terminated string in rdi and writes it in stdout
print_string: ;OK
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
print_char: ;OK
    mov rax, 1            ; 'write' syscall number
    push rdi
    mov rsi, rsp          ; string address
    mov rdi, 1            ; stdout descriptor
    mov rdx, 1            ; string length in bytes
    syscall               ; 'write' syscall
ret

; New line (writes 0xA (10) simbol)
print_newline: ;OK
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
    pop r10
    pop rax
    pop rdi
    pop rdx
ret

; Prints 8-byte unsigned integer in decimal format from rdi
print_int: ;NOT YET
    cmp rdi, 0
    jge print_uint
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
    jmp print_uint
ret

; Принимает два указателя (rdi and rsi) на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе 
string_equals: ;OK
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
    jne no
    xor rcx, rcx 
    .loop:
        mov al, [rsi + rcx]
        mov dl, [rdi + rcx]
        cmp al, dl
        jne no
        inc rcx
        cmp rcx, r8
        jl .loop 
    xor rax, rax
    inc rax
ret
    no: 
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
    je end
    cmp rax, 10
    je end
    mov al, [for_read_char]
ret
    end:
        mov rax, 0
    ret
 


; Принимает: адрес начала буфера, размер буфера
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
    ret
 

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
    xor rax, rax
    ret




; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
    xor rax, rax
    ret 

; Принимает указатель на строку rdi, указатель на буфер rsi и длину буфера rdx
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0 in rax
string_copy:    ;OK
    push rdi
    push rsi
    push rdx
    call string_length ; string length in rax
    pop rdx
    pop rsi
    pop rdi
    cmp rax, rdx 
    jge notfit
    ; if fits:
    xor rcx, rcx
    .loop:
        mov al, [rdi + rcx]
        mov  [rsi + rcx], al
        inc rcx 
        cmp rcx, rdx 
    jl  .loop
    mov byte[rsi + rdx + 1], 0 ; adding null-terminator to buffer
ret
notfit:
    xor rax, rax
ret
