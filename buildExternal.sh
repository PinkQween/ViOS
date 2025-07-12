#!/bin/bash

set -e

# Load .env
if [ ! -f .env ]; then
  echo "❌ .env file not found!"
  exit 1
fi

export $(grep -v '^#' .env | xargs)

# Check required vars
for var in USER HOST REMOTE_PATH; do
  if [ -z "${!var}" ]; then
    echo "❌ Missing $var in .env"
    exit 1
  fi
done

if [ -z "$PASSWORD" ] && [ -z "$IDENTITY_FILE" ]; then
  echo "❌ Must provide either PASSWORD or IDENTITY_FILE in .env"
  exit 1
fi

LOCAL_PATH="$(pwd)"

# Define rsync base options (exclude .git and local build/bin folders, but include fonts and font sources)
RSYNC_OPTS="-avz --no-owner --no-group --no-times --exclude=.git --exclude=*/build/ --exclude=*/bin/"

# Include your source font files (update this path if your font sources are somewhere else)
RSYNC_INCLUDE_FONTS="--include='utilities/fonts/***' --include='utilities/fonts/'"

echo "[*] Ensuring src/fonts exists on remote server..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" ssh -o StrictHostKeyChecking=no "$USER@$HOST" \
    "mkdir -p '$REMOTE_PATH/src/fonts' && sudo chmod 755 '$REMOTE_PATH/src/fonts'"
else
  ssh -i "$IDENTITY_FILE" -o StrictHostKeyChecking=no "$USER@$HOST" \
    "mkdir -p '$REMOTE_PATH/src/fonts' && sudo chmod 755 '$REMOTE_PATH/src/fonts'"
fi

# Ensure proper permissions on the entire remote directory
echo "[*] Setting proper permissions on remote directory..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" ssh -o StrictHostKeyChecking=no "$USER@$HOST" \
    "sudo chmod -R 755 '$REMOTE_PATH'"
else
  ssh -i "$IDENTITY_FILE" -o StrictHostKeyChecking=no "$USER@$HOST" \
    "sudo chmod -R 755 '$REMOTE_PATH'"
fi

# Change ownership to the user
echo "[*] Setting ownership of remote directory..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" ssh -o StrictHostKeyChecking=no "$USER@$HOST" \
    "sudo chown -R $USER:$USER '$REMOTE_PATH'"
else
  ssh -i "$IDENTITY_FILE" -o StrictHostKeyChecking=no "$USER@$HOST" \
    "sudo chown -R $USER:$USER '$REMOTE_PATH'"
fi

# Upload (only modified files), including the font source directory explicitly
echo "[*] Syncing modified files to $USER@$HOST:$REMOTE_PATH..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" rsync $RSYNC_OPTS $RSYNC_INCLUDE_FONTS \
    -e "ssh -o StrictHostKeyChecking=no" \
    "$LOCAL_PATH/" "$USER@$HOST:$REMOTE_PATH/"
else
  rsync $RSYNC_OPTS $RSYNC_INCLUDE_FONTS \
    -e "ssh -i $IDENTITY_FILE -o StrictHostKeyChecking=no" \
    "$LOCAL_PATH/" "$USER@$HOST:$REMOTE_PATH/"
fi

# Build step on remote server
echo "[*] Running ./build.sh on remote server..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" ssh -o StrictHostKeyChecking=no "$USER@$HOST" \
    "cd '$REMOTE_PATH' && sudo chmod +x ./build.sh && sudo ./build.sh"
else
  ssh -i "$IDENTITY_FILE" -o StrictHostKeyChecking=no "$USER@$HOST" \
    "cd '$REMOTE_PATH' && sudo chmod +x ./build.sh && sudo ./build.sh"
fi

# Download all /bin and /build folders recursively
echo "[*] Downloading all /bin and /build folders recursively..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" rsync -avz \
    -e "ssh -o StrictHostKeyChecking=no" \
    --include="/bin/***" \
    --include="/build/***" \
    --include="*/" \
    --include="*/bin/***" \
    --include="*/build/***" \
    --exclude="*" \
    "$USER@$HOST:$REMOTE_PATH/" "$LOCAL_PATH/"
else
  rsync -avz \
    -e "ssh -i $IDENTITY_FILE -o StrictHostKeyChecking=no" \
    --include="/bin/***" \
    --include="/build/***" \
    --include="*/" \
    --include="*/bin/***" \
    --include="*/build/***" \
    --exclude="*" \
    "$USER@$HOST:$REMOTE_PATH/" "$LOCAL_PATH/"
fi

echo "[✓] Done."
