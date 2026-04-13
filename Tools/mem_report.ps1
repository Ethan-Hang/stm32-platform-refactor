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
    if ($fillCount -lt 0)       { $fillCount = 0 }
    if ($fillCount -gt $Length) { $fillCount = $Length }

    return ("$Fill" * $fillCount) + ("$Empty" * ($Length - $fillCount))
}

function Get-PreviousRegionSnapshot {
    param(
        [Parameter(Mandatory = $false)]
        [object]$State,

        [Parameter(Mandatory = $true)]
        [string]$RegionName
    )

    if ($null -eq $State -or $null -eq $State.regions) { return $null }
    $prop = $State.regions.PSObject.Properties[$RegionName]
    if ($null -eq $prop) { return $null }
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

    $deltaPct   = [Math]::Round($CurrentPct - [double]$PreviousRegion.pct, 1)
    $deltaBytes = [int64]($CurrentUsed - [int64]$PreviousRegion.used)

    if ([Math]::Abs($deltaPct) -lt 0.05 -and $deltaBytes -eq 0) {
        return " (no change)"
    }

    if ($deltaPct -gt 0.0 -or (($deltaPct -eq 0.0) -and ($deltaBytes -gt 0))) {
        return (" (+{0:N1}%, +{1:N0} bytes)" -f [Math]::Abs($deltaPct), [Math]::Abs($deltaBytes))
    }

    return (" (-{0:N1}%, -{1:N0} bytes)" -f [Math]::Abs($deltaPct), [Math]::Abs($deltaBytes))
}

if (-not (Test-Path -LiteralPath $ElfPath)) { throw "ELF file not found: $ElfPath" }
if (-not (Test-Path -LiteralPath $LdPath))  { throw "Linker script not found: $LdPath" }

# ---------------------------------------------------------------------------
# 1. Parse all MEMORY regions from the linker script
# ---------------------------------------------------------------------------
$ldLines = Get-Content -LiteralPath $LdPath
$memories = [System.Collections.Generic.List[PSCustomObject]]::new()

foreach ($line in $ldLines) {
    if ($line -match "^\s*([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*:\s*ORIGIN\s*=\s*([^,]+),\s*LENGTH\s*=\s*([^\s/]+)") {
        $memories.Add([PSCustomObject]@{
            Name   = $matches[1]
            Attr   = $matches[2]
            Origin = (Convert-ToUInt64 -Value $matches[3].Trim())
            Length = (Convert-ToUInt64 -Value $matches[4].Trim())
            Used   = [int64]0
        })
    }
}

if ($memories.Count -eq 0) {
    throw "No MEMORY regions parsed from linker script: $LdPath"
}

# ---------------------------------------------------------------------------
# 2. Attribute each ELF section to its memory region by VMA
#    (arm-none-eabi-size -A reports VMA + size for every allocated section,
#     including NOLOAD/SHT_NOBITS sections such as .bss and .RW_RTT)
# ---------------------------------------------------------------------------
$sizeALines = & $SizeTool -A $ElfPath 2>&1

foreach ($line in $sizeALines) {
    # Expected format: "  .section_name   <size_decimal>   <addr_decimal>"
    if ($line -match "^\s*(\.\S*)\s+(\d+)\s+(\d+)\s*$") {
        $secSize = [int64]$matches[2]
        $secAddr = [uint64]$matches[3]
        if ($secSize -le 0) { continue }

        foreach ($mem in $memories) {
            if ($secAddr -ge $mem.Origin -and $secAddr -lt ($mem.Origin + $mem.Length)) {
                $mem.Used += $secSize
                break
            }
        }
    }
}

# ---------------------------------------------------------------------------
# 3. Memory Usage Summary  (top-level, uses first rx region + first w region)
# ---------------------------------------------------------------------------
$sizeSummaryLine = & $SizeTool $ElfPath | Select-Object -Last 1
$summaryParts = $sizeSummaryLine -split "\s+" | Where-Object { $_ -ne "" }
if ($summaryParts.Count -lt 3) { throw "Unexpected size output: $sizeSummaryLine" }

$text      = [int64]$summaryParts[0]
$data      = [int64]$summaryParts[1]
$bss       = [int64]$summaryParts[2]
$flashUsed = $text + $data

# Main RAM = first writable region only (region-attributed, excludes RTT_RAM etc.)
$loadMemory = $memories | Where-Object { $_.Attr -match "r" -and $_.Attr -match "x" -and $_.Attr -notmatch "w" } | Select-Object -First 1
if (-not $loadMemory) { $loadMemory = $memories | Where-Object { $_.Attr -match "r" -and $_.Attr -match "x" } | Select-Object -First 1 }
if (-not $loadMemory) { $loadMemory = $memories | Select-Object -First 1 }

$rwMemory = $memories | Where-Object { $_.Attr -match "w" } | Select-Object -First 1
if (-not $rwMemory) { $rwMemory = $memories | Select-Object -First 1 }

$flashTotal = [int64]$loadMemory.Length
$ramTotal   = [int64]$rwMemory.Length
$ramUsed    = [int64]$rwMemory.Used   # region-attributed (excludes RTT_RAM etc.)
if ($flashTotal -le 0) { $flashTotal = 1 }
if ($ramTotal   -le 0) { $ramTotal   = 1 }

$flashPct = [Math]::Round(($flashUsed * 100.0) / $flashTotal, 1)
$ramPct   = [Math]::Round(($ramUsed   * 100.0) / $ramTotal,   1)

Write-Host "=== Memory Usage Summary ==="
Write-Host ("RAM used  : {0} / {1} bytes ({2} %)" -f $ramUsed, $ramTotal, $ramPct)
Write-Host ("FLASH used: {0} / {1} bytes ({2} %)" -f $flashUsed, $flashTotal, $flashPct)

# ---------------------------------------------------------------------------
# 4. Per-file object table
# ---------------------------------------------------------------------------
$elfAbsPath = (Resolve-Path -LiteralPath $ElfPath).Path
$buildDir   = Split-Path -Path $elfAbsPath -Parent
$projectDir = Split-Path -Path $buildDir   -Parent
if ([string]::IsNullOrWhiteSpace($projectDir)) { $projectDir = (Get-Location).Path }
$targetName = [System.IO.Path]::GetFileNameWithoutExtension($elfAbsPath)

$stateDir = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($stateDir) -or -not (Test-Path -LiteralPath $stateDir)) {
    $stateDir = Split-Path -Path $LdPath -Parent
}

$stateFile            = Join-Path $stateDir (".mem_report_last_{0}.json" -f $targetName)
$legacyRootStateFile  = Join-Path $projectDir (".mem_report_last_{0}.json" -f $targetName)
$legacyBuildStateFile = Join-Path $buildDir ".mem_report_last.json"

$previousState = $null
foreach ($f in @($stateFile, $legacyRootStateFile, $legacyBuildStateFile)) {
    if (Test-Path -LiteralPath $f) {
        try { $previousState = Get-Content -LiteralPath $f -Raw | ConvertFrom-Json; break } catch {}
    }
}

Write-Host ""
Write-Host "=== Per-File RAM/FLASH (object-based) ==="
$objs = Get-ChildItem -Path $buildDir -Filter "*.o" | Sort-Object Name
$rows = @()
foreach ($o in $objs) {
    $line = & $SizeTool $o.FullName | Select-Object -Last 1
    if (-not $line) { continue }
    $parts = $line -split "\s+" | Where-Object { $_ -ne "" }
    if ($parts.Count -lt 6) { continue }
    $rows += [PSCustomObject]@{
        File  = $o.Name
        RAM   = [int64]$parts[1] + [int64]$parts[2]
        FLASH = [int64]$parts[0] + [int64]$parts[1]
    }
}

if ($rows.Count -gt 0) {
    $fileWidth = [Math]::Max(36, ($rows.File | Measure-Object -Property Length -Maximum).Maximum + 2)
    $sep = "".PadLeft($fileWidth + 38, '-')
    Write-Host $sep
    Write-Host (("{0} | {1,15} | {2,15}" -f "FILE(s)".PadRight($fileWidth), "RAM (byte)", "FLASH (byte)"))
    Write-Host $sep
    foreach ($r in $rows) {
        Write-Host (("{0} | {1,15} | {2,15}" -f $r.File.PadRight($fileWidth), $r.RAM, $r.FLASH))
    }
    Write-Host $sep
}

# ---------------------------------------------------------------------------
# 5. Link Region View — one entry per LD MEMORY region, auto-discovered
# ---------------------------------------------------------------------------
$barLen = 48
$currentState = [ordered]@{
    timestamp = (Get-Date).ToString("yyyy-MM-ddTHH:mm:ss")
    regions   = [ordered]@{}
}

Write-Host ""
Write-Host "=== Link Region View (From LD) ==="

foreach ($mem in $memories) {
    $usedBytes  = $mem.Used
    $totalBytes = [int64]$mem.Length
    if ($totalBytes -le 0) { $totalBytes = 1 }

    $pct     = [Math]::Round(($usedBytes * 100.0) / $totalBytes, 1)
    $usedKB  = $usedBytes  / 1024.0
    $totalKB = $totalBytes / 1024.0

    $fillChar      = if ($mem.Attr -match "w") { [char]'O' } else { [char]'#' }
    $bar           = New-Bar -Percent $pct -Length $barLen -Fill $fillChar -Empty '_'
    $sectionPrefix = if ($mem.Attr -match "w") { "RW" } else { "ER" }
    $sectionLabel  = "{0}_{1}" -f $sectionPrefix, $mem.Name

    $prevSnap = Get-PreviousRegionSnapshot -State $previousState -RegionName $sectionLabel
    $delta    = Format-DeltaSuffix -CurrentPct $pct -CurrentUsed $usedBytes -PreviousRegion $prevSnap

    Write-Host ""
    Write-Host ("        {0,-18}[0x{1:X8} | 0x{2:X8} ({2})]" -f $mem.Name, $mem.Origin, $mem.Length)
    Write-Host ("                {0,-10}[0x{1:X8}]|{2}| ( {3,6:N1} KB / {4,6:N1} KB )  {5,5:N1}%{6}" -f `
        $sectionLabel, $mem.Origin, $bar, $usedKB, $totalKB, $pct, $delta)

    $currentState.regions[$sectionLabel] = [ordered]@{
        used  = [int64]$usedBytes
        total = [int64]$totalBytes
        pct   = [double]$pct
    }
}

try {
    $currentState | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $stateFile -Encoding UTF8
}
catch {}
