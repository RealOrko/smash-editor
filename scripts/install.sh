#!/bin/bash
#
# SmashEdit Installer for Linux and macOS
# Downloads the latest release and installs to ~/.smash
#

set -e

REPO="RealOrko/smash-editor"
INSTALL_DIR="$HOME/.smash"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Detect platform
detect_platform() {
    local os=$(uname -s | tr '[:upper:]' '[:lower:]')
    case "$os" in
        linux*)  echo "linux" ;;
        darwin*) echo "macos" ;;
        *)       error "Unsupported platform: $os" ;;
    esac
}

# Get latest release version from GitHub API
get_latest_version() {
    local version
    if command -v curl &> /dev/null; then
        version=$(curl -s "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name"' | sed -E 's/.*"([^"]+)".*/\1/')
    elif command -v wget &> /dev/null; then
        version=$(wget -qO- "https://api.github.com/repos/$REPO/releases/latest" | grep '"tag_name"' | sed -E 's/.*"([^"]+)".*/\1/')
    else
        error "Neither curl nor wget found. Please install one of them."
    fi

    if [ -z "$version" ]; then
        error "Failed to get latest version. Check your internet connection."
    fi

    echo "$version"
}

# Download file
download() {
    local url="$1"
    local output="$2"

    info "Downloading from $url"

    if command -v curl &> /dev/null; then
        curl -fsSL "$url" -o "$output"
    elif command -v wget &> /dev/null; then
        wget -q "$url" -O "$output"
    fi
}

# Add to PATH in shell config
update_path() {
    local shell_config=""
    local path_line="export PATH=\"\$HOME/.smash:\$PATH\""

    # Detect shell config file
    if [ -n "$ZSH_VERSION" ] || [ -f "$HOME/.zshrc" ]; then
        shell_config="$HOME/.zshrc"
    elif [ -n "$BASH_VERSION" ] || [ -f "$HOME/.bashrc" ]; then
        shell_config="$HOME/.bashrc"
    elif [ -f "$HOME/.profile" ]; then
        shell_config="$HOME/.profile"
    fi

    if [ -z "$shell_config" ]; then
        warn "Could not detect shell config file. Please add $INSTALL_DIR to your PATH manually."
        return
    fi

    # Check if already in PATH config
    if grep -q "\.smash" "$shell_config" 2>/dev/null; then
        info "PATH already configured in $shell_config"
    else
        echo "" >> "$shell_config"
        echo "# SmashEdit" >> "$shell_config"
        echo "$path_line" >> "$shell_config"
        success "Added $INSTALL_DIR to PATH in $shell_config"
        warn "Run 'source $shell_config' or restart your terminal to use smashedit"
    fi
}

main() {
    echo ""
    echo "  ╔═══════════════════════════════════════╗"
    echo "  ║       SmashEdit Installer             ║"
    echo "  ╚═══════════════════════════════════════╝"
    echo ""

    # Detect platform
    local platform=$(detect_platform)
    info "Detected platform: $platform"

    # Get latest version
    local version=$(get_latest_version)
    info "Latest version: $version"

    # Create install directory
    mkdir -p "$INSTALL_DIR"
    info "Install directory: $INSTALL_DIR"

    # Download URL
    local archive_name="smashedit-${platform}-${version}.tar.gz"
    local download_url="https://github.com/$REPO/releases/download/${version}/${archive_name}"

    # Create temp directory
    local tmp_dir=$(mktemp -d)
    trap "rm -rf $tmp_dir" EXIT

    # Download archive
    local archive_path="$tmp_dir/$archive_name"
    download "$download_url" "$archive_path"

    # Extract archive
    info "Extracting archive..."
    tar -xzf "$archive_path" -C "$INSTALL_DIR"

    # Make executable
    chmod +x "$INSTALL_DIR/smashedit"

    success "SmashEdit $version installed to $INSTALL_DIR/smashedit"

    # Update PATH
    update_path

    echo ""
    success "Installation complete!"
    echo ""
    echo "  Run 'smashedit' to start editing (after restarting your terminal)"
    echo "  Or run directly: $INSTALL_DIR/smashedit"
    echo ""
}

main "$@"
