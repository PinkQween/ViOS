#!/bin/bash

echo "=================================="
echo "  ViOS Remote Build Setup"
echo "=================================="
echo

# Check if .env already exists
if [ -f .env ]; then
    echo "⚠️  .env file already exists!"
    read -p "Do you want to overwrite it? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Setup cancelled."
        exit 0
    fi
fi

echo "Please provide the following information for remote building:"
echo

# Get remote server details
read -p "Remote username: " USER
read -p "Remote host (IP or hostname): " HOST
read -p "Remote project path: " REMOTE_PATH

echo
echo "Choose authentication method:"
echo "1. Password authentication"
echo "2. SSH key authentication"
read -p "Enter choice (1 or 2): " AUTH_CHOICE

if [ "$AUTH_CHOICE" = "1" ]; then
    read -s -p "Password: " PASSWORD
    echo
    IDENTITY_FILE=""
elif [ "$AUTH_CHOICE" = "2" ]; then
    read -p "SSH key path (e.g., ~/.ssh/id_rsa): " IDENTITY_FILE
    PASSWORD=""
else
    echo "Invalid choice. Exiting."
    exit 1
fi

# Create .env file
cat > .env << EOF
# Remote build configuration for buildExternal.sh
USER=$USER
HOST=$HOST
REMOTE_PATH=$REMOTE_PATH
PASSWORD=$PASSWORD
IDENTITY_FILE=$IDENTITY_FILE
EOF

echo
echo "✅ .env file created successfully!"
echo
echo "Next steps:"
echo "1. Make sure your remote server has the necessary build tools"
echo "2. Ensure the remote user has sudo privileges"
echo "3. Run './buildExternal.sh' to build on the remote server"
echo
echo "Note: The remote server should have:"
echo "  - Linux (for mtools and other dependencies)"
echo "  - i686-elf cross-compiler toolchain"
echo "  - nasm, make, gcc, python3, and other build tools" 