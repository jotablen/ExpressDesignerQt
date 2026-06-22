param(
    [string]$ExePath,
    [string]$MingwBin = 'C:\msys64\mingw64\bin',
    [string]$BinDir
)

$systemDlls = @(
    'kernel32.dll','user32.dll','advapi32.dll','shell32.dll','msvcrt.dll',
    'ole32.dll','oleaut32.dll','gdi32.dll','ws2_32.dll','wsock32.dll',
    'crypt32.dll','comctl32.dll','comdlg32.dll','winmm.dll','shlwapi.dll',
    'rpcrt4.dll','setupapi.dll','oleacc.dll','version.dll','iphlpapi.dll',
    'netapi32.dll','imm32.dll','uxtheme.dll','dwmapi.dll','d3d9.dll',
    'opengl32.dll','glu32.dll','winspool.drv','secur32.dll',
    'ntdll.dll','kernelbase.dll','mscoree.dll','bcrypt.dll','cryptbase.dll',
    'sspicli.dll','cfgmgr32.dll','ucrtbase.dll','powrprof.dll','shcore.dll',
    'windows.storage.dll','wldp.dll','profapi.dll','gdi32full.dll',
    'msvcp_win.dll','win32u.dll','sechost.dll'
)

$seen = @{}
$queue = [System.Collections.ArrayList]::new()
$scannedFiles = @{}

[void]$queue.Add($ExePath)
Get-ChildItem "$MingwBin\libTK*.dll" | ForEach-Object { [void]$queue.Add($_.FullName) }

$round = 0
while ($queue.Count -gt 0 -and $round -lt 8) {
    $nextQueue = [System.Collections.ArrayList]::new()
    foreach ($file in $queue) {
        if ($scannedFiles.ContainsKey($file)) { continue }
        $scannedFiles[$file] = $true
        $deps = & "$MingwBin\objdump.exe" -p $file 2>$null |
            Select-String 'DLL Name:\s+(\S+\.dll)' |
            ForEach-Object { $_.Matches.Groups[1].Value.ToLower() }
        foreach ($d in $deps) {
            if ($systemDlls -contains $d) { continue }
            $srcPath = Join-Path $MingwBin $d
            if (-not (Test-Path $srcPath)) { continue }
            if (-not $seen.ContainsKey($d)) {
                $destPath = Join-Path $BinDir $d
                if (-not (Test-Path $destPath)) {
                    Copy-Item $srcPath $BinDir -Force
                    Write-Host "[OK] $d"
                }
                $seen[$d] = $true
                [void]$nextQueue.Add($srcPath)
            }
        }
    }
    $queue = $nextQueue
    $round++
}
Write-Host "Transitive scan complete ($round round(s), $($seen.Count) DLLs resolved)."