
set(TargetName Test_ToLuau)
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE EXE
        LIB ToLuau
            StandaloneLuau
)

