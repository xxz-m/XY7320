$ErrorActionPreference = "Stop"

$projectRoot = $PSScriptRoot
$templatePath  = Join-Path $projectRoot "CMakeLists_template.txt"
$generatedPath = Join-Path $projectRoot "CMakeLists.txt"

$layerMarkers = @(
    "BSP/uart","BSP/adc","BSP/tim","BSP/dma","BSP/gpio",
    "Services/uart_service","Services/adc_service","Services/report",
    "Domain/filter","Domain/measurement","Domain/model",
    "App/mode_manager","App/command","App/usecase",
    "System/scheduler","System/event","System/queue",
    "Common/types","Common/config","Common/utils"
)

$sourceGlobs = @(
    '"BSP/*.*"','"Middleware/*.*"','"Services/*.*"',
    '"Domain/*.*"','"App/*.*"','"System/*.*"','"Common/*.*"'
)

Write-Host "============================================"
Write-Host " CMakeLists Sync Check"
Write-Host "============================================"
Write-Host ""

if (-not (Test-Path $templatePath)) {
    Write-Host "[ERROR] Template not found: $templatePath" -ForegroundColor Red
    exit 1
}
if (-not (Test-Path $generatedPath)) {
    Write-Host "[ERROR] Generated file not found: $generatedPath" -ForegroundColor Red
    exit 1
}

$templateContent  = Get-Content $templatePath -Raw
$generatedContent = Get-Content $generatedPath -Raw

$missingInTemplate  = @()
$missingInGenerated = @()

foreach ($marker in $layerMarkers) {
    if ($templateContent -notmatch [regex]::Escape($marker)) {
        $missingInTemplate += $marker
    }
    if ($generatedContent -notmatch [regex]::Escape($marker)) {
        $missingInGenerated += $marker
    }
}

foreach ($glob in $sourceGlobs) {
    $escaped = [regex]::Escape($glob)
    if ($templateContent -notmatch $escaped) {
        $missingInTemplate += "SOURCE: $glob"
    }
    if ($generatedContent -notmatch $escaped) {
        $missingInGenerated += "SOURCE: $glob"
    }
}

$hasError = $false

if ($missingInTemplate.Count -gt 0) {
    Write-Host "[WARN] Template missing layer config:" -ForegroundColor Yellow
    foreach ($m in $missingInTemplate) {
        Write-Host "  - $m" -ForegroundColor Yellow
    }
    Write-Host ""
}

if ($missingInGenerated.Count -gt 0) {
    $hasError = $true
    Write-Host "[ERROR] CMakeLists.txt missing layer config (CubeMX overwrite?):" -ForegroundColor Red
    foreach ($m in $missingInGenerated) {
        Write-Host "  - $m" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "[FIX] Re-sync from CMakeLists_template.txt or regenerate via CubeMX" -ForegroundColor Yellow
    Write-Host ""
}

if (-not $hasError -and $missingInTemplate.Count -eq 0) {
    Write-Host "[OK] CMakeLists.txt and template are in sync. All layer configs present." -ForegroundColor Green
}

if ($hasError) { exit 2 } else { exit 0 }
