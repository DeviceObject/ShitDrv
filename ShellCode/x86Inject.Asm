USE32

x86InjectShellCode_Start:
pushad
call GetCurrentAddress 
GetCurrentAddress:
BASE_OFFSET_CURRENT_ADDRESS equ (GetCurrentAddress - x86InjectShellCode_Start)
pop ebp
sub ebp,BASE_OFFSET_CURRENT_ADDRESS

mov eax,0x11111111
lea ecx,[ebp + x86_InjectDll_Path]
push ecx
call eax
popad
;jmp ds:0x40404040
db 0xEA
dd 0x40404040
db 0x1B
db 0x00
retn
x86_InjectDll_Path:
dd 0x80808080
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
dd 0x00000000
x86InjectShellCode_End: