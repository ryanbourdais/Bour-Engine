src_files=$(find src -type f \
  ! -path 'src/external/*' \
  ! -path '*/build/*' \
  \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \))

printf "%s\n" "$src_files" |
xargs wc -l -m |
awk '
  END {
    printf "source files: %d\n", NR - 1
    printf "lines: %s\n", $1
    printf "characters: %s\n", $2
  }
'

printf "\nlines by subsystem:\n"

for dir in src/*; do
  [ -d "$dir" ] || continue
  [ "$(basename "$dir")" = "external" ] && continue
  [ "$(basename "$dir")" = "build" ] && continue

  find "$dir" -type f \
    ! -path '*/build/*' \
    \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \) \
    -exec wc -l {} + |
  awk -v name="$(basename "$dir")" '
    END {
      if (NR > 0) {
        printf "  %-12s %s\n", name ":", $1
      }
    }
  '
done