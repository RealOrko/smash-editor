#Requires -Version 5.1
<#
.SYNOPSIS
    SmashEdit Installer for Windows
.DESCRIPTION
    Downloads the latest release of SmashEdit and installs it to ~/.smash
    Also adds the installation directory to the user's PATH
#>

$ErrorActionPreference = "Stop"

$Repo = "RealOrko/smash-editor"
$InstallDir = Join-Path $env:USERPROFILE ".smash"

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] " -ForegroundColor Blue -NoNewline
    Write-Host $Message
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] " -ForegroundColor Green -NoNewline
    Write-Host $Message
}

function Write-Warn {
    param([string]$Message)
    Write-Host "[WARN] " -ForegroundColor Yellow -NoNewline
    Write-Host $Message
}

function Write-Error2 {
    param([string]$Message)
    Write-Host "[ERROR] " -ForegroundColor Red -NoNewline
    Write-Host $Message
    exit 1
}

function Get-LatestVersion {
    try {
        $response = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest" -UseBasicParsing
        return $response.tag_name
    }
    catch {
        Write-Error2 "Failed to get latest version. Check your internet connection. $_"
    }
}

function Update-UserPath {
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")

    if ($currentPath -like "*$InstallDir*") {
        Write-Info "PATH already contains $InstallDir"
        return
    }

    $newPath = "$InstallDir;$currentPath"
    [Environment]::SetEnvironmentVariable("Path", $newPath, "User")

    # Also update current session
    $env:Path = "$InstallDir;$env:Path"

    Write-Success "Added $InstallDir to user PATH"
    Write-Warn "You may need to restart your terminal for PATH changes to take effect"
}

function Main {
    Write-Host ""
    Write-Host "  +===========================================+" -ForegroundColor Cyan
    Write-Host "  |         SmashEdit Installer               |" -ForegroundColor Cyan
    Write-Host "  +===========================================+" -ForegroundColor Cyan
    Write-Host ""

    # Get latest version
    $version = Get-LatestVersion
    Write-Info "Latest version: $version"

    # Create install directory
    if (-not (Test-Path $InstallDir)) {
        New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    }
    Write-Info "Install directory: $InstallDir"

    # Download URL
    $archiveName = "smashedit-windows-$version.zip"
    $downloadUrl = "https://github.com/$Repo/releases/download/$version/$archiveName"

    # Create temp directory
    $tempDir = Join-Path $env:TEMP "smashedit-install-$(Get-Random)"
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

    try {
        $archivePath = Join-Path $tempDir $archiveName

        # Download archive
        Write-Info "Downloading from $downloadUrl"
        Invoke-WebRequest -Uri $downloadUrl -OutFile $archivePath -UseBasicParsing

        # Extract archive
        Write-Info "Extracting archive..."
        Expand-Archive -Path $archivePath -DestinationPath $InstallDir -Force

        Write-Success "SmashEdit $version installed to $InstallDir\smashedit.exe"

        # Update PATH
        Update-UserPath

        Write-Host ""
        Write-Success "Installation complete!"
        Write-Host ""
        Write-Host "  Run 'smashedit' to start editing (after restarting your terminal)"
        Write-Host "  Or run directly: $InstallDir\smashedit.exe"
        Write-Host ""
    }
    finally {
        # Cleanup temp directory
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

Main
