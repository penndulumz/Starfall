param(
    [string]$OutputDirectory = 'releases',
    [string]$Revision = 'HEAD'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $repoRoot
try {
    $outputRoot = [IO.Path]::GetFullPath($OutputDirectory)
    $sourceZip = Join-Path $outputRoot 'Starfall-Source.zip'
    $historyBundle = Join-Path $outputRoot 'Starfall-History.bundle'
    foreach ($destination in @($sourceZip, "$sourceZip.sha256", $historyBundle, "$historyBundle.sha256")) {
        if (Test-Path -LiteralPath $destination) { throw "Existing artifact preserved: $destination. Choose a fresh output directory." }
    }
    $commit = & git -C $repoRoot rev-parse --verify "$Revision^{commit}"
    if ($LASTEXITCODE -ne 0) { throw 'A committed Git revision is required for the source/history handoff.' }
    $trackedFiles = & git -C $repoRoot ls-tree -r --name-only $commit
    if ($LASTEXITCODE -ne 0) { throw 'Could not inspect the source revision.' }
    if ($trackedFiles | Where-Object { $_ -match '^(dist|build|releases|\.git)/' }) {
        throw 'The revision still tracks generated outputs. Untrack them before creating the source archive.'
    }
    New-Item -ItemType Directory -Path $outputRoot -Force | Out-Null
    & git -C $repoRoot archive --format=zip --prefix=Starfall-Source/ --output=$sourceZip $commit
    if ($LASTEXITCODE -ne 0) { throw 'Source archive creation failed.' }
    Add-Type -AssemblyName System.IO.Compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::Open($sourceZip, [IO.Compression.ZipArchiveMode]::Update)
    try {
        $entry = $zip.CreateEntry('Starfall-Source/SOURCE-COMMIT.txt')
        $writer = [IO.StreamWriter]::new($entry.Open(), [Text.UTF8Encoding]::new($false))
        try { $writer.WriteLine($commit) } finally { $writer.Dispose() }
    } finally { $zip.Dispose() }
    # HEAD retains an unmerged candidate too; main and the integration branch
    # preserve the original baseline and normal local implementation history.
    & git -C $repoRoot bundle create $historyBundle main integration/starfall-overhaul $Revision
    if ($LASTEXITCODE -ne 0) { throw 'History bundle creation failed.' }
    & git -C $repoRoot bundle verify $historyBundle
    if ($LASTEXITCODE -ne 0) { throw 'History bundle verification failed.' }
    foreach ($artifact in @($sourceZip, $historyBundle)) {
        $stream = [IO.File]::OpenRead($artifact)
        $sha = [Security.Cryptography.SHA256]::Create()
        try { $digest = ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-', '').ToLowerInvariant() }
        finally { $stream.Dispose(); $sha.Dispose() }
        [IO.File]::WriteAllText("$artifact.sha256", "$digest  $([IO.Path]::GetFileName($artifact))`n", (New-Object Text.UTF8Encoding($false)))
        Write-Host "$digest  $artifact"
    }
    Write-Host "Source-only ZIP and private history bundle record $commit. Playing without a compiler uses the separate portable ZIP."
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
} finally { Pop-Location }
