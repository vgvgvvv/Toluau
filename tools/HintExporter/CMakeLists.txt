

set(TargetName "ToLuauHintExporter")
ReMake_AddTarget(
        TARGET_NAME ${TargetName}
        MODE EXE
        LIB ToLuau
        DEFINE_PRIVATE ToLuauTool
)