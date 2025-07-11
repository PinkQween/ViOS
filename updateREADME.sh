#!/bin/bash

TARGET_DIR="/tmp/ViOS"

rm -rf "$TARGET_DIR"

echo "🔁 Updating files into $TARGET_DIR, excluding .git/ and files ignored by .gitignore except .env"

# Copy files excluding .git and gitignored files except .env
find . -type f | while read -r file; do
    rel_path="${file#./}"

    # Explicitly skip .git directory contents
    if [[ "$rel_path" == .git/* ]]; then
        echo "🚫 Skipping .git directory file $rel_path"
        continue
    fi

    # Always include .env even if ignored
    if [[ "$rel_path" == ".env" ]]; then
        echo "✅ Copying $rel_path (forced include)"
        mkdir -p "$TARGET_DIR/$(dirname "$rel_path")"
        cp "$rel_path" "$TARGET_DIR/$rel_path"
        continue
    fi

    # Check if file is ignored by git
    if git check-ignore -q "$rel_path"; then
        echo "🚫 Skipping ignored $rel_path"
        continue
    fi

    echo "✅ Copying $rel_path"
    mkdir -p "$TARGET_DIR/$(dirname "$rel_path")"
    cp "$rel_path" "$TARGET_DIR/$rel_path"
done

# Generate tree output from $TARGET_DIR
RAW_TREE=$(tree -a "$TARGET_DIR" | grep -v '\.env\.example$')

# Replace first line "/tmp/ViOS" with "."
TREE_OUTPUT=$(echo "$RAW_TREE" | sed "1s|^$TARGET_DIR$|.|")

# Update real README.md (in current directory)
README="README.md"
START_LINE=$(grep -n "🗂️ Project Structure" "$README" | cut -d: -f1)
if [ -z "$START_LINE" ]; then
  echo "Error: Project Structure section not found in $README"
  exit 1
fi

CODE_START_LINE=$(tail -n +"$START_LINE" "$README" | grep -n '^```' | head -1 | cut -d: -f1)
CODE_START_LINE=$((START_LINE + CODE_START_LINE - 1))

CODE_END_LINE=$(tail -n +"$((CODE_START_LINE + 1))" "$README" | grep -n '^```' | head -1 | cut -d: -f1)
CODE_END_LINE=$((CODE_START_LINE + CODE_END_LINE))

TEMP_README=$(mktemp)

head -n $((CODE_START_LINE)) "$README" > "$TEMP_README"

{
  echo "$TREE_OUTPUT"
  echo '```'
} >> "$TEMP_README"

tail -n +$((CODE_END_LINE + 1)) "$README" >> "$TEMP_README"

mv "$TEMP_README" "$README"

echo "✅ README.md 'Project Structure' section updated with current tree output."