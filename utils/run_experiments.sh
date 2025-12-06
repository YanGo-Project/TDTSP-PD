#!/bin/bash

FILES=(
    0.json
    10.json
    12.json
    14.json
    16.json
    18.json
    2.json
    21.json
    23.json
    25.json
    4.json 
    6.json 
    8.json
    1.json 
    11.json
    13.json
    15.json
    17.json 
    19.json 
    20.json 
    22.json 
    24.json # проверить что тут может вызывать бесконечный цикл
    3.json  
    5.json  
    7.json  
    9.json
)


APP_PATH="${1:-./build/app}"
OUTPUT_CSV="${2:-results.csv}"
RUNS_PER_FILE="${3:-5}"
PATH_TO_JSONS="${4:-./jsons}"
TIME_LIMIT=50

if [ ! -f "$APP_PATH" ]; then
    echo "Ошибка: приложение не найдено по пути $APP_PATH"
    echo "Убедитесь, что приложение скомпилировано в директории build/"
    exit 1
fi

TOTAL_FILES=${#FILES[@]}

echo "file_name,score,time,distance,timestamp" >> "$OUTPUT_CSV"

CURRENT_FILE=0
PROCESSED_FILES=0

for file in "${FILES[@]}"; do
    CURRENT_FILE=$((CURRENT_FILE + 1))
    
    if [ ! -f "$PATH_TO_JSONS/$file" ]; then
        echo "Предупреждение: файл $file не найден, пропускаем..."
        continue
    fi
    
    PROCESSED_FILES=$((PROCESSED_FILES + 1))
    echo "[$CURRENT_FILE/$TOTAL_FILES] Обработка файла: $file (запусков: $RUNS_PER_FILE)"
    
    for run in $(seq 1 $RUNS_PER_FILE); do
        echo "  Запуск $run/$RUNS_PER_FILE"
        ./"$APP_PATH" -p "$PATH_TO_JSONS/$file" -t "$TIME_LIMIT" -c "$OUTPUT_CSV"
        
        if [ $? -ne 0 ]; then
            echo "  Ошибка при выполнении запуска $run для файла $PATH_TO_JSONS/$file"
        fi
    done
done

echo ""
echo "Готово! Результаты сохранены в $OUTPUT_CSV"
echo "Файлов в списке: $TOTAL_FILES"
echo "Обработано файлов: $PROCESSED_FILES"
echo "Запусков на файл: $RUNS_PER_FILE"

