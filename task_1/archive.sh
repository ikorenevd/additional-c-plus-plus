#!/usr/bin/env bash
set -euo pipefail

# Работаем в каталоге скрипта независимо от места запуска.
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

if ! command -v zip >/dev/null 2>&1; then
    printf 'Ошибка: установите утилиту zip.\n' >&2
    exit 1
fi

files=(
    block.h block.cpp
    date_type.h date_type.cpp
    rational.h rational.cpp
)

for file in "${files[@]}"; do
    if [[ ! -f "$file" ]]; then
        printf 'Ошибка: файл не найден: %s\n' "$file" >&2
        exit 1
    fi
done

# Создаём архив заново, чтобы в нём не оставались удалённые файлы.
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT
zip -q "$temp_dir/Korenev_ID.zip" "${files[@]}"
mv -f -- "$temp_dir/Korenev_ID.zip" Korenev_ID.zip
printf 'Создан архив: %s/Korenev_ID.zip\n' "$PWD"
