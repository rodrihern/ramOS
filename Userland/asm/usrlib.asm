global generate_invalid_opcode
global printf
global scanf
extern printf_aux
extern scanf_aux

generate_invalid_opcode:
    ud2         ; Genera excepción de opcode inválido
    ret

printf:
    push rbp
    mov  rbp, rsp

    ; Guarda los 5 registros variables en un array local args[5] que creamos arriba de la pila
    sub  rsp, 104               ; 13 × 8 bytes (5 punteros/enteros, etc y 8 para flotantes)
    mov  [rsp+8*0], rsi        ; args[0] = 2.º parámetro real
    mov  [rsp+8*1], rdx
    mov  [rsp+8*2], rcx
    mov  [rsp+8*3], r8
    mov  [rsp+8*4], r9
    
    ; Guardar argumentos floats/double (primeros 2 registros XMM)
    movsd [rsp+8*5], xmm0      ; float_args[0]
    movsd [rsp+8*6], xmm1      ; float_args[1]
    movsd [rsp+8*7], xmm2      ; float_args[2]
    movsd [rsp+8*8], xmm3      ; float_args[3]
    movsd [rsp+8*9], xmm4      ; float_args[4]
    movsd [rsp+8*10], xmm5      ; float_args[5]
    movsd [rsp+8*11], xmm6      ; float_args[6]
    movsd [rsp+8*12], xmm7      ; float_args[7]

    ; Argumentos para printf_aux
    ; 1er argumento de printf_aux: rdi ya tenía *fmt desde el principio
    lea  rsi, [rsp]            ; 2do argumento de printf_aux: rsi = &args[0] (dirección del primer elemento del arreglo que se creo en el stack con los registros) 
    lea  rdx, [rbp+16]         ; 3er argumento de printf_aux: rdx = puntero al 1er arg en stack (los argumentos que no entraron en los registros para pasarse a printf)
    lea  rcx, [rsp+8*5]        ; 4to argumento: rcx = &float_args[0]
    call printf_aux         ; kvprintf_core(fmt, regArgs, stackPtr, floatArgs)

    leave
    ret

scanf:
    push rbp
    mov  rbp, rsp

    ; Guarda los 5 registros variables en un array local args[5] que creamos arriba de la pila
    sub  rsp, 40               ; 5 × 8 bytes 
    mov  [rsp+8*0], rsi        ; args[0] = 2.º parámetro real
    mov  [rsp+8*1], rdx
    mov  [rsp+8*2], rcx
    mov  [rsp+8*3], r8
    mov  [rsp+8*4], r9

    ; Argumentos para printf_aux
    ; 1er argumento de printf_aux: rdi ya tenía *fmt desde el principio
    lea  rsi, [rsp]            ; 2do argumento de printf_aux: rsi = &args[0] (dirección del primer elemento del arreglo que se creo en el stack con los registros) 
    lea  rdx, [rbp+16]         ; 3er argumento de printf_aux: rdx = puntero al 1er arg en stack (los argumentos que no entraron en los registros para pasarse a printf)
    call scanf_aux         ; kvprintf_core(fmt, regArgs, stackPtr, floatArgs)

    leave
    ret
