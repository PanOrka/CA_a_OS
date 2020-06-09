global _start

section .text
_start:
        ; starting msg
        mov    eax, 4d
        mov    ebx, 1d
        mov    ecx, st_msg
        mov    edx, len_msg
        int    0x80

        ; read
        mov    eax, 3d
        mov    ebx, 0d
        mov    ecx, num
        mov    edx, 4d
        int    0x80

        ; convert to DEC
        mov    eax, 0d
        mov    ebx, 0d
        jmp    decc

loop:   mul    byte [multiplier]     ; multiplying al by 10
decc:   sub    byte [num + ebx], '0' ; taking num[loop_count] - '0'
        add    al, [num + ebx]
        inc    ebx                   ; incrementing loop_count
        mov    ah, [num + ebx]
        cmp    ah, byte [eol]        ; comparing ah to eol
        jne    loop

        mov    byte [tri_height], al

        ; triangle loop, convert to ASCII
        mov    byte [counter], 1d    ; setting our line elements counter to 1, used for line count elements
M_loop: mov    cl, 0d                ; setting ECX to 0, used for S_loop, in line
S_loop: mov    eax, 0d               ; set eax to 0
        mov    al, cl                ; taking ecx+1 to eax for counting number in Line
        add    al, 1d
        mov    byte [divider], 100d  ; setting divider to max sized number (3 digits)
        mov    byte [size], 3d       ; setting size to max sized number (3 digits)
        push   eax
        jmp    divis

A_loop: sub    byte [size], 1d       ; This is procedure that counts digits in number
        mov    eax, 0d
        mov    al, byte [divider]    ; divider / 10 to look up if size is smaller
        div    byte [multiplier]
        mov    byte [divider], al
        pop    eax                   ; taking eax back from stack
        push   eax
divis:  div    byte [divider]        ; here is division to know the size of number
        cmp    al, 0d                ; we are doing this until we get 0 from division
        je     A_loop

        mov    ebx, 0d
        mov    bl, byte [size]       ; here we put size to index register EBX
        mov    byte [num + ebx], ' ' ; space add
        pop    eax                   ; we take old eax value from stack

R_loop: sub    ebx, 1d               ; This procedure inserts ASCII char for digits in number
        div    byte [multiplier]
        mov    byte [num + ebx], ah  ; here we take %10 from ax
        add    byte [num + ebx], '0' ; inserting equal digit in ASCII to write on console
        mov    ah, 0d                ; we dont need modulo, just divided so we set it to 0 in register
        cmp    ebx, 0d               ; ebx has to reach 0, ebx is index for num table
        jne    R_loop

        push   eax                   ; IDK what could be on the registers, maybe something useful in next
        push   ebx                   ; increments in-line
        push   ecx
        push   edx

        add    byte [size], 1d       ; increasing size for space print

        mov    eax, 4d                     ; printing the number
        mov    ebx, 1d
        mov    ecx, num
        mov    edx, [size]
        int    0x80

        pop    edx
        pop    ecx
        pop    ebx
        pop    eax

        add    cl, 1d
        cmp    byte [counter], cl
        jne    S_loop

        sub    byte [tri_height], 1d      ; coming down in triangle
        add    byte [counter], 1d         ; increasing max counter in one particular line

        mov    eax, 4d                    ; EOL add
        mov    ebx, 1d
        mov    ecx, eol
        mov    edx, 1d
        int    0x80

        cmp    byte [tri_height], 0d      ; checking whether we are at the base of triangle
        jne    M_loop

end:    mov    ebx, 0d
        mov    eax, 1d
        int    0x80

section .data
st_msg:        db 'Enter number: '
len_msg:       equ $-st_msg
test_msg:      db '1 '
len_test:      equ $-test_msg
multiplier:    db 10d
eol:           db 0x0a

section .bss
num:           times 4 resb 1 ; reserving 4 bytes
tri_height:    resb 1         ; reserving 1 byte for triangle height
counter:       resb 1         ; reserving 1 byte for actual max_counter
divider:       resb 1
size:          resb 1
