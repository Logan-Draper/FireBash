#!/bin/bash

dir_search() {
  local dir="$1"
  local outfile="${dir}_output.txt"

  >"$outfile"

  for file in "$dir"/*; do
    if [[ -d "$file" ]]; then
      dir_search "$file"
    else
      size=$(wc -c <"$file")
      echo "$file,$size" >>"$outfile"
    fi
  done
}
>cwd_output.txt
for file in *; do
  if [[ -d "$file" ]]; then
    dir_search "$file"
    continue
  fi
  if [[ "$file" =~ _output\.txt$ ]]; then
    continue
  fi
  size=$(wc -c <"$file")
  echo "$file,$size" >>cwd_output.txt
done
