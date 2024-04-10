#!/bin/bash

# Define the format_file function
format_file() {
    local input_file="$1"
    local output_file="$2"
    # Ensure the output file's directory exists
    mkdir -p "$(dirname "${output_file}")"
    while IFS= read -r line; do
        echo "\"${line}\\n\"" >> "${output_file}"
    done < "${input_file}"
}

# Base directory containing the files and subdirectories to format
base_directory="certs"

# Directory to store formatted files, mirroring the structure of base_directory
output_base_directory="formatted_certs"

# Use find to iterate over each file within base_directory and its subdirectories
while IFS= read -r input_file; do
    # Construct the corresponding output directory structure
    relative_path="${input_file#${base_directory}/}" # Extract the relative path from the base_directory
    output_file="${output_base_directory}/${relative_path}_formatted"
    
    # Call the format_file function with the current file
    format_file "${input_file}" "${output_file}"
done < <(find "${base_directory}" -type f)

echo "Formatting of all files and replication of directory structure completed."