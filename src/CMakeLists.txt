

set(TargetName "ToLuau")
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE SHARED
        LIB StandaloneLuau
)