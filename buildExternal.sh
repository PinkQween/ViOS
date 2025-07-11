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

# Define rsync base options (exclude .git and local build/bin folders)
RSYNC_OPTS="-avz --no-owner --no-group --exclude=.git --exclude=*/build/ --exclude=*/bin/"

# Upload (only modified files) — run rsync remotely as sudo
echo "[*] Syncing modified files to $USER@$HOST:$REMOTE_PATH..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" rsync $RSYNC_OPTS \
    --rsync-path="sudo rsync" \
    -e "ssh -o StrictHostKeyChecking=no" \
    "$LOCAL_PATH/" "$USER@$HOST:$REMOTE_PATH/"
else
  rsync $RSYNC_OPTS \
    --rsync-path="sudo rsync" \
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

# Download all /bin and /build folders recursively (rsync remotely as sudo)
echo "[*] Downloading all /bin and /build folders recursively..."
if [ -n "$PASSWORD" ]; then
  sshpass -p "$PASSWORD" rsync -avz \
    --rsync-path="sudo rsync" \
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
    --rsync-path="sudo rsync" \
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
