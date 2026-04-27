param(
    [string]$MakeExe = "make",
    [string]$BuildDir = "build",
    [string]$BuildLog = "build/build.log",
    [string]$Target = "build-core",
    [int]$Jobs = $env:NUMBER_OF_PROCESSORS
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Ensure a fresh log for each build.
New-Item -ItemType File -Path $BuildLog -Force | Out-Null

$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

$makeCommand = ('"{0}" --no-print-directory -j{1} {2}' -f $MakeExe.Replace('"', '""'), $Jobs, $Target)
& cmd /d /c "$makeCommand 2>&1" | Tee-Object -FilePath $BuildLog -Append
$makeExitCode = $LASTEXITCODE

$stopwatch.Stop()

$logText = ""
if (Test-Path -Path $BuildLog) {
    $logText = Get-Content -Path $BuildLog -Raw
}

$warningCount = ([regex]::Matches($logText, '(?im)(^|[^A-Za-z0-9_])warning:')).Count
$errorCount = ([regex]::Matches($logText, '(?im)(^|[^A-Za-z0-9_])error:')).Count

$elapsedTotalSeconds = [int][Math]::Round($stopwatch.Elapsed.TotalSeconds)
$elapsedMinutes = [int]($elapsedTotalSeconds / 60)
$elapsedSeconds = [int]($elapsedTotalSeconds % 60)

Write-Host ""
Write-Host "=== Build Summary ==="
Write-Host ("Compile time: {0:D2}:{1:D2} ({2}s)" -f $elapsedMinutes, $elapsedSeconds, $elapsedTotalSeconds)
Write-Host ("Warnings: {0}" -f $warningCount)
Write-Host ("Errors  : {0}" -f $errorCount)

exit $makeExitCode
