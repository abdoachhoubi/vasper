#!/bin/bash

word_to_replace="root"
input_file="$(pwd)/config/default.conf"
new_root="root $(pwd)"

# Use awk to replace lines starting with the specified word
awk -v word="$word_to_replace" -v new_root="$new_root" '{ if ($1 == word) printf "\t%s;\n", new_root; else print $0 }' "$input_file" > tmpfile && mv tmpfile "$input_file"

echo "Updated default.conf"