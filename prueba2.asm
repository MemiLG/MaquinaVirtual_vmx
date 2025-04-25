xor efx, efx
mov [4], 0
otro: mov edx, ds
mov ch, 4
mov cl, 1
mov al, 0x01
sys 1
cmp [edx], 0
jn
add [4], [edx]
add efx, 1
jmp otro
sigue: cmp efx, 0
jz
div [4], efx
fin: mov edx, ds
add edx, 4
mov ch, 1
mov cl, 4
mov al, 0x01
sys 2
stop