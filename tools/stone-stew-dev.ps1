param(
    [ValidateSet("check", "launch", "solution")]
    [string]$Action = "check"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$SourceDir = Join-Path $RepoRoot "crawl-ref/source"
$Solution = Join-Path $SourceDir "MSVC/crawl-ref.sln"
$Exe = Join-Path $SourceDir "crawl.exe"
$TownMap = "stone_stew_arrival_gate_town"

function Require-CrawlExe {
    if (!(Test-Path $Exe)) {
        Write-Host "No crawl.exe found at $Exe"
        Write-Host "Build local tiles first with Visual Studio:"
        Write-Host "  $Solution"
        Write-Host "Configuration: Release Tiles | x64"
        exit 1
    }
}

switch ($Action) {
    "solution" {
        if (!(Test-Path $Solution)) {
            throw "Could not find Visual Studio solution at $Solution"
        }
        Start-Process $Solution
    }

    "check" {
        Require-CrawlExe
        Push-Location $SourceDir
        try {
            Write-Host "Rebuilding .des database..."
            & $Exe -builddb
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

            Write-Host "Validating Stone Stew D:1 town map..."
            & $Exe -mapstat D:1 -iters 1 -force-map $TownMap
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
        finally {
            Pop-Location
        }
    }

    "launch" {
        Require-CrawlExe
        Push-Location $SourceDir
        try {
            & $Exe
        }
        finally {
            Pop-Location
        }
    }
}
