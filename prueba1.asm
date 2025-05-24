\\EXTRA 1024
null    equ     -1
size_es equ     1024


; inicializar HEAP
; sin parámetros
; asume que todo el extra segment es para uso de memoria dinámica    
heap_init:  mov [es], es
            add [es], 4
            ret
; invocacion: 
; call heap_init



; solicita memoria dinámica 
; parámetros: +8 cantidad de bytes a solicitar
; devuelve en EAX dirección de memoria o NULL si no hay memoria suficiente
alloc:      push    bp
            mov     bp, sp
            push    bx
            mov     eax, null
            mov     bx, [es]
            add     bx, [bp+8]
            cmp     bx, size_es
            jp      allocfin
            mov     eax, [es]
            add     [es], [bp+8]
allocfin:   pop     bx
            mov     sp, bp  
            pop     bp
            ret
; invocacion: 
; push <cantidad de bytes a solicitar>
; call alloc
; add  sp, 4
; << eax dirección de memoria

; Binary Trees

; Estructura del nodo:
;    +------------+
;  0 |   entero   |  (4 bytes)
;    +------------+
; +4 |   *left    |  (4 bytes)
;    +------------+
; +8 |   *right   |  (4 bytes)
;    +------------+
btn_size    equ     12 ; Binary Tree Node
val         equ     0
left        equ     4
right       equ     8


;----------------------------------------
; crea un nuevo nodo de árbol binario
; parámetros: +8 valor entero
;----------------------------------------
; invocación:
; push  <valor entero>
; call  btn_new
; add   sp, 4
; << eax puntero al nuevo nodo
;----------------------------------------
btn_new:        push    bp
                mov     bp, sp

                push    btn_size
                call    alloc
                add     sp, 4      

                cmp     eax, null
                jz      btn_new_fin

                mov     [eax+val], [bp+8]
                mov     [eax+left], null
                mov     [eax+right], null

btn_new_fin:    mov     sp, bp
                pop     bp
                ret 

;----------------------------------------
; agrega nodo a árbol binario de búsqueda (BST)
; parámetros: 
;  +8 doble puntero a root
; +12 puntero a nodo de arbol a insertar
;----------------------------------------
; invocación:
; push  <*bnt>
; push  <**root>
; call  bst_add
; add   sp,8
; (no devuelve nada)
;----------------------------------------
bst_add:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    edx            

                mov     edx, [bp+8]     ; **root
                mov     ebx, [edx]      ; *root
                mov     eax, [bp+12]    ; *bnt a insertar

                cmp     eax, null
                jz      bst_add_end

                cmp     ebx, null
                jz      bst_append

                cmp     [ebx+val],[eax+val]
                jz      bst_add_end

                jp      bst_add_left 
                jn      bst_add_right

bst_add_left:   add     ebx, left
                push    eax
                push    ebx
                call    bst_add
                add     sp, 8
                jmp     bst_add_end

bst_add_right:  add     ebx, right
                push    eax
                push    ebx
                call    bst_add
                add     sp, 8
                jmp     bst_add_end

bst_append:     mov     [edx], eax
bst_add_end:    pop     edx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret


;----------------------------------------
; imprime en in-order árbol binario de búsqueda (BST)
; parámetros: 
;  +8 puntero simple a root
;----------------------------------------
; invocación:
; push  <*root>
; call  inorder
; add   sp,4
; (no devuelve nada)
;----------------------------------------
inorder:        push    bp
                mov     bp, sp
                push    eax
                push    ebx
                push    ecx
                push    edx

                mov     ebx, [bp+8]     ; *root
                
                cmp     ebx, null
                jz      inorder_end

                ; llamo por izquierda
                push    [ebx+left]
                call    inorder
                add     sp, 4

                ; preparo en edx la dirección de la variable aux
                mov     edx, ebx
                add     edx, val    
                mov     eax, 0x0001
                mov     ecx, 0x0401
                sys     0x0002

                ; llamo por derecha
                push    [ebx+right]
                call    inorder
                add     sp, 4

inorder_end:    pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                mov     sp, bp
                pop     bp
                ret
root  equ    4
MAIN:   push bp
        mov bp, sp
        sub sp, 4 ; root de ABB
        push eax
        push edx
        
        call    heap_init    ; inicializa memoria dinámica

        mov edx, bp
        sub edx, root        ; edx = &root
        mov [edx], null      ; inicializo root = null

        push 20
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 20
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 10
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 15
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 30
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push 25
        call btn_new
        add sp, 4

        push eax
        push edx
        call bst_add
        add sp, 8

        push [edx]
        call inorder
        add sp, 4

main_end:   pop edx
        pop eax
        mov sp,bp
        pop bp
        stop 
