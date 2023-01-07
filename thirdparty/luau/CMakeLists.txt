
set(TargetName "StandaloneLuau")
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE SHARED
        INC "Luau/VM/include"
            "Luau/Common/include"
            "Luau/Compiler/include"
            "Luau/Ast/include"
            "."
)
