.code

public _ExitBootServices

extern RetExitBootServices : qword
extern HookedExitBootServices : proc

_ExitBootServices proc
    mov rax, [rsp]
    mov RetExitBootServices, rax
    jmp HookedExitBootServices
_ExitBootServices endp

END
