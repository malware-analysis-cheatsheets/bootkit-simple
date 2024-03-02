.code

PUBLIC _ExitBootServices

EXTERN RetExitBootServices : QWORD
EXTERN HookedExitBootServices : PROC

_ExitBootServices PROC
    mov rax, [rsp]
    mov RetExitBootServices, rax
    jmp HookedExitBootServices
_ExitBootServices ENDP

END
