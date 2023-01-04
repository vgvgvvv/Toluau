
set(TargetName "luausocket")
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE SHARED
        LIB Luau.VM
            Luau.Compiler
            ws2_32
        DEFINE LUA_VERSION_NUM=501
)

ReMake_StartUse()
UseLuau(${TargetName})
ReMake_EndUse()