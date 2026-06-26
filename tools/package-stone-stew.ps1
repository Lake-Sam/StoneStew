param(
    [string]$Version = "0.34.1-dev1",
    [string]$Configuration = "win64"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$SourceDir = Join-Path $RepoRoot "crawl-ref/source"
$Exe = Join-Path $SourceDir "crawl.exe"
$DistRoot = Join-Path $RepoRoot "dist"
$PackageName = "StoneStew-$Version-$Configuration"
$PackageDir = Join-Path $DistRoot $PackageName
$ZipPath = Join-Path $DistRoot "$PackageName.zip"

if (!(Test-Path $Exe)) {
    throw "Missing $Exe. Build first with: .\tools\stone-stew-dev.ps1 check"
}

if (Test-Path $PackageDir) {
    Remove-Item -LiteralPath $PackageDir -Recurse -Force
}
if (Test-Path $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}

New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

Copy-Item -LiteralPath $Exe -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $SourceDir "dat") -Destination $PackageDir -Recurse
Copy-Item -LiteralPath (Join-Path $RepoRoot "README.md") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "LICENSE") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "docs/stone-stew-design.md") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "docs/testing-stone-stew.md") -Destination $PackageDir

@"
@echo off
cd /d "%~dp0"
start "" crawl.exe
"@ | Set-Content -LiteralPath (Join-Path $PackageDir "Start Stone Stew.bat") -Encoding ASCII

@"
Stone Stew $Version ($Configuration)

Run:
  crawl.exe

Or double-click:
  Start Stone Stew.bat

This is an early development build based on Dungeon Crawl Stone Soup 0.34.1.
"@ | Set-Content -LiteralPath (Join-Path $PackageDir "RUN-ME.txt") -Encoding ASCII

if (Get-Command 7z -ErrorAction SilentlyContinue) {
    Push-Location $DistRoot
    try {
        & 7z a -tzip "$PackageName.zip" $PackageName | Out-Host
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
    finally {
        Pop-Location
    }
}
else {
    Compress-Archive -LiteralPath $PackageDir -DestinationPath $ZipPath -CompressionLevel Optimal
}

Write-Host "Created $ZipPath"
