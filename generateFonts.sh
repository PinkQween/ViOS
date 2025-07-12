#!/bin/bash
set -e  # Exit script if any command fails

# Create virtual environment
python3 -m venv .venv

# Activate virtual environment
source .venv/bin/activate

# Upgrade pip first (optional but recommended)
pip install --upgrade pip

# Install freetype-py
pip install --upgrade freetype-py

# Run the Python script
./utilities/generateFonts.py