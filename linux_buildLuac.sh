#!/bin/bash

INPUT_DIR="./lua"
OUTPUT_DIR="./build/lua"

# Create base output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Find all .lua files in INPUT_DIR recursively
find "$INPUT_DIR" -type f -name "*.lua" | while read -r file; do
    # Compute relative path (remove ./scripts/ prefix)
    relative_path="${file#$INPUT_DIR/}"
    
    # Compute output path (change .lua to .luac)
    output_file="$OUTPUT_DIR/${relative_path%.lua}.luac"

    # Create directory if it doesn't exist
    mkdir -p "$(dirname "$output_file")"

    # Compile to luac
    echo "Compiling $file to $output_file"
    if luac -o "$output_file" "$file"; then
        echo "Successfully compiled $file"
    else
        echo "Failed to compile $file"
    fi
done
