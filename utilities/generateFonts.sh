#!/bin/bash
set -e  # Exit script if any command fails

# Check if freetype-py is already available
if python3 -c "import freetype" 2>/dev/null; then
    echo "freetype-py is already available, using existing environment"
    python3 ./utilities/generateFonts.py
else
    echo "freetype-py not found, setting up virtual environment..."
    
    # Create virtual environment if it doesn't exist
    if [ ! -d ".venv" ]; then
        python3 -m venv .venv
    fi

    # Activate virtual environment
    source .venv/bin/activate

    # Upgrade pip first (optional but recommended)
    pip install --upgrade pip

    # Remove any broken freetype installs
    rm -rf .venv/lib/python3.12/site-packages/freetype*
    pip install --force-reinstall --no-deps freetype-py==2.5.1

    # Run the Python script
    python3 ./utilities/generateFonts.py
fi