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
$SetupName = "$PackageName-Setup"
$SetupPath = Join-Path $DistRoot "$SetupName.exe"

if (!(Test-Path $Exe)) {
    throw "Missing $Exe. Build first with: .\tools\stone-stew-dev.ps1 check"
}

if (Test-Path $PackageDir) {
    Remove-Item -LiteralPath $PackageDir -Recurse -Force
}
if (Test-Path $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}
if (Test-Path $SetupPath) {
    Remove-Item -LiteralPath $SetupPath -Force
}

New-Item -ItemType Directory -Force -Path $PackageDir | Out-Null

Copy-Item -LiteralPath $Exe -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $SourceDir "dat") -Destination $PackageDir -Recurse
Copy-Item -LiteralPath (Join-Path $RepoRoot "crawl-ref/settings") -Destination $PackageDir -Recurse
Copy-Item -LiteralPath (Join-Path $RepoRoot "crawl-ref/docs") -Destination $PackageDir -Recurse
New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "contrib/fonts") | Out-Null
Copy-Item -LiteralPath (Join-Path $SourceDir "contrib/fonts/DejaVuSans.ttf") -Destination (Join-Path $PackageDir "contrib/fonts")
Copy-Item -LiteralPath (Join-Path $SourceDir "contrib/fonts/DejaVuSansMono.ttf") -Destination (Join-Path $PackageDir "contrib/fonts")
Copy-Item -LiteralPath (Join-Path $RepoRoot "README.md") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "LICENSE") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "docs/stone-stew-design.md") -Destination $PackageDir
Copy-Item -LiteralPath (Join-Path $RepoRoot "docs/testing-stone-stew.md") -Destination $PackageDir

@"
@echo off
cd /d "%~dp0"
echo Starting Stone Stew from %CD%
crawl.exe %*
if errorlevel 1 pause
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

$Iscc = Get-Command ISCC.exe -ErrorAction SilentlyContinue
if (!$Iscc) {
    $CommonPaths = @(
        "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
    )
    foreach ($Path in $CommonPaths) {
        if ($Path -and (Test-Path $Path)) {
            $Iscc = Get-Item $Path
            break
        }
    }
}

if ($Iscc) {
    $IssPath = Join-Path $DistRoot "$PackageName.iss"
    $PackageDirForInno = $PackageDir -replace "\\", "\\"
    $DistRootForInno = $DistRoot -replace "\\", "\\"

@"
[Setup]
AppId={{8E7E091B-7A83-4F65-97D9-5F4F0E560B1B}
AppName=Stone Stew
AppVersion=$Version
AppPublisher=Lake-Sam
DefaultDirName={localappdata}\Programs\Stone Stew
DefaultGroupName=Stone Stew
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir=$DistRootForInno
OutputBaseFilename=$SetupName
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=Stone Stew

[Files]
Source: "$PackageDirForInno\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Stone Stew"; Filename: "{app}\Start Stone Stew.bat"; WorkingDir: "{app}"
Name: "{group}\Stone Stew Direct"; Filename: "{app}\crawl.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\Stone Stew"; Filename: "{app}\Start Stone Stew.bat"; WorkingDir: "{app}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Run]
Filename: "{app}\Start Stone Stew.bat"; WorkingDir: "{app}"; Description: "Launch Stone Stew"; Flags: postinstall skipifsilent
"@ | Set-Content -LiteralPath $IssPath -Encoding ASCII

    & $Iscc.FullName $IssPath | Out-Host
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
    Write-Host "Created $SetupPath"
}
else {
    Write-Host "Inno Setup compiler not found; skipped installer .exe."
}
