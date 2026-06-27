param(
    [string]$RuntimeVersion = "latest",
    [string]$ModelRepo = "ggml-org/Qwen3-1.7B-GGUF",
    [string]$ModelFile = "Qwen3-1.7B-Q4_K_M.gguf",
    [string]$VcRuntimePackage = "ThinkGeo.Dependency.MicrosoftVisualCRuntime140",
    [string]$VcRuntimeVersion = "14.5.2"
)

$ErrorActionPreference = "Stop"

$RepoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$BundleRoot = Join-Path $RepoRoot "third_party/stone-stew-llm"
$RuntimeDir = Join-Path $BundleRoot "llama"
$ModelDir = Join-Path $BundleRoot "models"
$CacheDir = Join-Path $BundleRoot "_cache"

New-Item -ItemType Directory -Force -Path $RuntimeDir | Out-Null
New-Item -ItemType Directory -Force -Path $ModelDir | Out-Null
New-Item -ItemType Directory -Force -Path $CacheDir | Out-Null

function Download-File {
    param(
        [Parameter(Mandatory=$true)][string]$Uri,
        [Parameter(Mandatory=$true)][string]$OutFile
    )

    if (Test-Path $OutFile) {
        Write-Host "Already downloaded: $OutFile"
        return
    }

    Write-Host "Downloading $Uri"
    Invoke-WebRequest -Uri $Uri -OutFile $OutFile -Headers @{ "User-Agent" = "StoneStewDev" }
}

if ($RuntimeVersion -eq "latest") {
    $Release = Invoke-RestMethod -Uri "https://api.github.com/repos/ggml-org/llama.cpp/releases/latest" -Headers @{ "User-Agent" = "StoneStewDev" }
}
else {
    $Release = Invoke-RestMethod -Uri "https://api.github.com/repos/ggml-org/llama.cpp/releases/tags/$RuntimeVersion" -Headers @{ "User-Agent" = "StoneStewDev" }
}

$RuntimeAsset = $Release.assets |
    Where-Object { $_.name -match '^llama-.+-bin-win-cpu-x64\.zip$' } |
    Select-Object -First 1

if (!$RuntimeAsset) {
    throw "Could not find a Windows CPU x64 llama.cpp release asset."
}

$RuntimeZip = Join-Path $CacheDir $RuntimeAsset.name
Download-File -Uri $RuntimeAsset.browser_download_url -OutFile $RuntimeZip

Write-Host "Expanding llama.cpp runtime..."
Remove-Item -LiteralPath $RuntimeDir -Recurse -Force
New-Item -ItemType Directory -Force -Path $RuntimeDir | Out-Null
Expand-Archive -LiteralPath $RuntimeZip -DestinationPath $RuntimeDir -Force

$Server = Get-ChildItem -LiteralPath $RuntimeDir -Recurse -Filter "llama-server.exe" | Select-Object -First 1
if (!$Server) {
    throw "Downloaded llama.cpp runtime did not contain llama-server.exe."
}

if ($Server.DirectoryName -ne $RuntimeDir) {
    Copy-Item -LiteralPath (Join-Path $Server.DirectoryName "*") -Destination $RuntimeDir -Recurse -Force
}

$ModelUri = "https://huggingface.co/$ModelRepo/resolve/main/${ModelFile}?download=true"
$ModelPath = Join-Path $ModelDir $ModelFile
Download-File -Uri $ModelUri -OutFile $ModelPath

$VcPackagePath = Join-Path $CacheDir "$VcRuntimePackage.$VcRuntimeVersion.nupkg"
$VcPackageZip = Join-Path $CacheDir "$VcRuntimePackage.$VcRuntimeVersion.zip"
$VcExtractDir = Join-Path $CacheDir "vcruntime"
$VcNativeDir = Join-Path $VcExtractDir "runtimes/win-x64/native"

Download-File -Uri "https://www.nuget.org/api/v2/package/$VcRuntimePackage/$VcRuntimeVersion" -OutFile $VcPackagePath
Copy-Item -LiteralPath $VcPackagePath -Destination $VcPackageZip -Force
Remove-Item -LiteralPath $VcExtractDir -Recurse -Force -ErrorAction SilentlyContinue
Expand-Archive -LiteralPath $VcPackageZip -DestinationPath $VcExtractDir -Force

if (!(Test-Path $VcNativeDir)) {
    throw "Could not find x64 VC runtime files in $VcPackagePath."
}

Copy-Item -Path (Join-Path $VcNativeDir "*.dll") -Destination $RuntimeDir -Force

@"
Stone Stew bundled LLM assets

Runtime:
  llama.cpp $($Release.tag_name)
  $($RuntimeAsset.browser_download_url)

Model:
  $ModelRepo
  $ModelFile
  https://huggingface.co/$ModelRepo

App-local VC runtime:
  $VcRuntimePackage $VcRuntimeVersion
  https://www.nuget.org/packages/$VcRuntimePackage/$VcRuntimeVersion

Packaging:
  tools/package-stone-stew.ps1 automatically includes this folder when
  llama/llama-server.exe and models/$ModelFile are present.
"@ | Set-Content -LiteralPath (Join-Path $BundleRoot "README.txt") -Encoding ASCII

Write-Host "Bundled LLM assets are ready in $BundleRoot"
