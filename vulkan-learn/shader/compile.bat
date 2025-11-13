@echo off
for %%f in (*.frag *.vert *.comp) do (
    echo 正在处理文件: "%%f"
    glslc "%%f" -o "%%f.spv"
    if errorlevel 1 (
        echo 错误: 处理文件 "%%f" 时失败
    ) else (
        echo 成功: 生成 "%%f.spv"
    )
)
echo 所有文件处理完成。
pause
