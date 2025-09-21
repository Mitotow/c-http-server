#!/bin/bash

script_dir="$(dirname "$(realpath "$0")")"
public_path="$script_dir/public"
bin_path="$script_dir/bin"

if [ ! -d "$public_path" ]; then
    echo "Creating public directory"
    mkdir -p $public_path
fi

if [ ! -d "$bin_path" ]; then
    echo "Creating bin directory"
    mkdir -p $bin_path
fi

echo "Building project into bin"
gcc *.c -o bin/server
