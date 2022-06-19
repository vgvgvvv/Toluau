

set(TargetName ToLuau)
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE SHARED
        LIB Luau.VM
            Luau.Compiler
)

ReMake_StartUse()
UseLuau(${TargetName})
ReMake_EndUse()