mov edx, ds
mov [edx], 5
mov eax, 10
div eax, [edx]
mov [edx], eax
mov ch, 4
mov cl, 1
mov al, 0x01
sys 2
stop