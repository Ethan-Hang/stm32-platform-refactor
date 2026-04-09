param(
    [Parameter(Mandatory = $true)]
    [string]$ElfPath,

    [Parameter(Mandatory = $true)]
    [string]$LdPath,

    [Parameter(Mandatory = $true)]
    [string]$SizeTool
)

function Convert-ToUInt64 {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    $raw = $Value.Trim()
    $multiplier = 1

    if ($raw.EndsWith("K", [System.StringComparison]::OrdinalIgnoreCase)) {
        $multiplier = 1024
        $raw = $raw.Substring(0, $raw.Length - 1)
    }
    elseif ($raw.EndsWith("M", [System.StringComparison]::OrdinalIgnoreCase)) {
        $multiplier = 1024 * 1024
        $raw = $raw.Substring(0, $raw.Length - 1)
    }

    if ($raw.StartsWith("0x", [System.StringComparison]::OrdinalIgnoreCase)) {
        $baseValue = [Convert]::ToUInt64($raw.Substring(2), 16)
    }
    else {
        $baseValue = [Convert]::ToUInt64($raw)
    }

    return [uint64]($baseValue * $multiplier)
}

function New-Bar {
    param(
        [Parameter(Mandatory = $true)]
        [double]$Percent,

        [Parameter(Mandatory = $true)]
        [int]$Length,

        [Parameter(Mandatory = $true)]
        [char]$Fill,

        [Parameter(Mandatory = $true)]
        [char]$Empty
    )

    $fillCount = [Math]::Round(($Percent / 100.0) * $Length)
    if ($fillCount -lt 0) {
        $fillCount = 0
    }
    if ($fillCount -gt $Length) {
        $fillCount = $Length
    }

    return ("$Fill" * $fillCount) + ("$Empty" * ($Length - $fillCount))
}

function Get-PreviousRegionSnapshot {
    param(
        [Parameter(Mandatory = $false)]
        [object]$State,

        [Parameter(Mandatory = $true)]
        [string]$RegionName
    )

    if ($null -eq $State -or $null -eq $State.regions) {
        return $null
    }

    $prop = $State.regions.PSObject.Properties[$RegionName]
    if ($null -eq $prop) {
        return $null
    }

    return $prop.Value
}

function Format-DeltaSuffix {
    param(
        [Parameter(Mandatory = $true)]
        [double]$CurrentPct,

        [Parameter(Mandatory = $true)]
        [int64]$CurrentUsed,

        [Parameter(Mandatory = $false)]
        [object]$PreviousRegion
    )

    if ($null -eq $PreviousRegion -or $null -eq $PreviousRegion.pct -or $null -eq $PreviousRegion.used) {
        return " (first run)"
    }

    $deltaPct = [Math]::Round($CurrentPct - [double]$PreviousRegion.pct, 1)
    $deltaBytes = [int64]($CurrentUsed - [int64]$PreviousRegion.used)

    if ([Math]::Abs($deltaPct) -lt 0.05 -and $deltaBytes -eq 0) {
        return " (no change)"
    }

    if ($deltaPct -gt 0.0 -or (($deltaPct -eq 0.0) -and ($deltaBytes -gt 0))) {
        return (" (+{0:N1}%, +{1:N0} bytes)" -f [Math]::Abs($deltaPct), [Math]::Abs($deltaBytes))
    }

    return (" (-{0:N1}%, -{1:N0} bytes)" -f [Math]::Abs($deltaPct), [Math]::Abs($deltaBytes))
}

if (-not (Test-Path -LiteralPath $ElfPath)) {
    throw "ELF file not found: $ElfPath"
}
if (-not (Test-Path -LiteralPath $LdPath)) {
    throw "Linker script not found: $LdPath"
}

$sizeSummaryLine = & $SizeTool $ElfPath | Select-Object -Last 1
$summaryParts = $sizeSummaryLine -split "\s+" | Where-Object { $_ -ne "" }
if ($summaryParts.Count -lt 6) {
    throw "Unexpected size output for ELF: $sizeSummaryLine"
}

$text = [int64]$summaryParts[0]
$data = [int64]$summaryParts[1]
$bss = [int64]$summaryParts[2]
$flashUsed = $text + $data
$ramUsed = $data + $bss

$ldLines = Get-Content -LiteralPath $LdPath
$memories = @()
foreach ($line in $ldLines) {
    if ($line -match "^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*:\s*ORIGIN\s*=\s*([^,]+),\s*LENGTH\s*=\s*([^\s/]+)") {
        $memories += [PSCustomObject]@{
            Name   = $matches[1]
            Attr   = $matches[2]
            Origin = (Convert-ToUInt64 -Value $matches[3])
            Length = (Convert-ToUInt64 -Value $matches[4])
        }
    }
}

if ($memories.Count -eq 0) {
    throw "No MEMORY regions parsed from linker script: $LdPath"
}

$loadMemory = $memories | Where-Object { $_.Attr -match "r" -and $_.Attr -match "x" -and $_.Attr -notmatch "w" } | Select-Object -First 1
if (-not $loadMemory) {
    $loadMemory = $memories | Where-Object { $_.Attr -match "r" -and $_.Attr -match "x" } | Select-Object -First 1
}
if (-not $loadMemory) {
    $loadMemory = $memories | Select-Object -First 1
}

$rwMemory = $memories | Where-Object { $_.Attr -match "w" } | Select-Object -First 1
if (-not $rwMemory) {
    $rwMemory = $memories | Select-Object -First 1
}

$flashTotal = [int64]$loadMemory.Length
$ramTotal = [int64]$rwMemory.Length
if ($flashTotal -le 0) {
    $flashTotal = 1
}
if ($ramTotal -le 0) {
    $ramTotal = 1
}

$flashPct = [Math]::Round(($flashUsed * 100.0) / $flashTotal, 1)
$ramPct = [Math]::Round(($ramUsed * 100.0) / $ramTotal, 1)

Write-Host "=== Memory Usage Summary ==="
Write-Host ("RAM used  : {0} / {1} bytes ({2} %)" -f $ramUsed, $ramTotal, $ramPct)
Write-Host ("FLASH used: {0} / {1} bytes ({2} %)" -f $flashUsed, $flashTotal, $flashPct)

$barLen = 48
$ramBar = New-Bar -Percent $ramPct -Length $barLen -Fill 'O' -Empty '_'
$flashBar = New-Bar -Percent $flashPct -Length $barLen -Fill '#' -Empty '_'

$lrName = "LR_{0}" -f $loadMemory.Name
$rwName = "RW_{0}" -f $rwMemory.Name
$erName = "ER_{0}" -f $loadMemory.Name

$elfAbsPath = (Resolve-Path -LiteralPath $ElfPath).Path
$buildDir = Split-Path -Path $elfAbsPath -Parent
$projectDir = Split-Path -Path $buildDir -Parent
if ([string]::IsNullOrWhiteSpace($projectDir)) {
    $projectDir = (Get-Location).Path
}

$targetName = [System.IO.Path]::GetFileNameWithoutExtension($elfAbsPath)
$stateDir = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($stateDir) -or -not (Test-Path -LiteralPath $stateDir)) {
    $stateDir = Split-Path -Path $LdPath -Parent
}

$stateFile = Join-Path $stateDir (".mem_report_last_{0}.json" -f $targetName)
$legacyRootStateFile = Join-Path $projectDir (".mem_report_last_{0}.json" -f $targetName)
$legacyBuildStateFile = Join-Path $buildDir ".mem_report_last.json"

$previousState = $null
if (Test-Path -LiteralPath $stateFile) {
    try {
        $previousState = Get-Content -LiteralPath $stateFile -Raw | ConvertFrom-Json
    }
    catch {
        $previousState = $null
    }
}
elseif (Test-Path -LiteralPath $legacyRootStateFile) {
    try {
        $previousState = Get-Content -LiteralPath $legacyRootStateFile -Raw | ConvertFrom-Json
    }
    catch {
        $previousState = $null
    }
}
elseif (Test-Path -LiteralPath $legacyBuildStateFile) {
    try {
        $previousState = Get-Content -LiteralPath $legacyBuildStateFile -Raw | ConvertFrom-Json
    }
    catch {
        $previousState = $null
    }
}

$previousRw = Get-PreviousRegionSnapshot -State $previousState -RegionName $rwName
$previousEr = Get-PreviousRegionSnapshot -State $previousState -RegionName $erName
$rwDeltaSuffix = Format-DeltaSuffix -CurrentPct $ramPct -CurrentUsed $ramUsed -PreviousRegion $previousRw
$erDeltaSuffix = Format-DeltaSuffix -CurrentPct $flashPct -CurrentUsed $flashUsed -PreviousRegion $previousEr

Write-Host ""
Write-Host "=== Per-File RAM/FLASH (object-based) ==="
$objs = Get-ChildItem -Path $buildDir -Filter "*.o" | Sort-Object Name
$rows = @()
foreach ($o in $objs) {
    $line = & $SizeTool $o.FullName | Select-Object -Last 1
    if (-not $line) {
        continue
    }

    $parts = $line -split "\s+" | Where-Object { $_ -ne "" }
    if ($parts.Count -lt 6) {
        continue
    }

    $otext = [int64]$parts[0]
    $odata = [int64]$parts[1]
    $obss = [int64]$parts[2]
    $rows += [PSCustomObject]@{
        File  = $o.Name
        RAM   = $odata + $obss
        FLASH = $otext + $odata
    }
}

if ($rows.Count -eq 0) {
    Write-Host "No object files found."
    exit 0
}

$fileWidth = (($rows.File | Measure-Object -Property Length -Maximum).Maximum + 2)
if ($fileWidth -lt 36) {
    $fileWidth = 36
}
$sep = "".PadLeft($fileWidth + 38, '-')

Write-Host $sep
Write-Host (("{0} | {1,15} | {2,15}" -f ("FILE(s)".PadRight($fileWidth)), "RAM (byte)", "FLASH (byte)"))
Write-Host $sep
foreach ($r in $rows) {
    Write-Host (("{0} | {1,15} | {2,15}" -f ($r.File.PadRight($fileWidth)), $r.RAM, $r.FLASH))
}
Write-Host $sep

Write-Host ""
Write-Host "=== Link Region View (From LD) ==="
Write-Host $lrName
Write-Host ("        {0,-18}[0x{1:X8} | 0x{2:X8} ({2})]" -f $rwMemory.Name, $rwMemory.Origin, $rwMemory.Length)
Write-Host ("                {0,-10}[0x{1:X8}]|{2}| ( {3,6:N1} KB / {4,6:N1} KB )  {5,5:N1}%{6}" -f $rwName, $rwMemory.Origin, $ramBar, ($ramUsed / 1024.0), ($ramTotal / 1024.0), $ramPct, $rwDeltaSuffix)
Write-Host ""
Write-Host ("        {0,-18}[0x{1:X8} | 0x{2:X8} ({2})]" -f $loadMemory.Name, $loadMemory.Origin, $loadMemory.Length)
Write-Host ("                {0,-10}[0x{1:X8}]|{2}| ( {3,6:N1} KB / {4,6:N1} KB )  {5,5:N1}%{6}" -f $erName, $loadMemory.Origin, $flashBar, ($flashUsed / 1024.0), ($flashTotal / 1024.0), $flashPct, $erDeltaSuffix)

$currentState = [ordered]@{
    timestamp = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
    regions = [ordered]@{}
}
$currentState.regions[$rwName] = [ordered]@{
    used = [int64]$ramUsed
    total = [int64]$ramTotal
    pct = [double]$ramPct
}
$currentState.regions[$erName] = [ordered]@{
    used = [int64]$flashUsed
    total = [int64]$flashTotal
    pct = [double]$flashPct
}

try {
    $currentState | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $stateFile -Encoding UTF8
}
catch {
}
