# DemoProject

## 1. Библиотека (де)сереализации JSON формата 
В проекте используется поплуряное решение 'nlohmann': https://github.com/nlohmann/json

Для его установки надо  выполнить команду:
1. ```sudo apt install nlohmann-json3-dev``` на Linux
2. ```brew install nlohmann-json``` на MacOs

## 2. Сборка проекта 

Далее нужно выполнить следующие шаги:
1. ```mkdir build && cd build```
2. ```cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .```

Если все прошло успешно - в папке `build/` появится исполняемый файл **app**.

Для сборки в `Debug` моде вместо пункта 2 выполнить команду ```cmake -DCMAKE_BUILD_TYPE=Debug .. && cmake --build .```.

В данном формате:
1. В консоль будут выводиться промжеточные стадии улучшения маршрутов.
2. Будет происходить валидация маршрута после операции перестройки `crossover` (см. `include/crossover.hpp`).

## 3. Аргументы программы 
Необходимо выполнить следующее: 
```./app -p <problem> -s <solution> -t <time>```
Где:
1. @problem - файл с разрешением `.json`, в котором данные параметры системы.
2. @solution - файл с разерешние `.json`, в который после работы алгоритма будет записано решение.
3. @time - время работы второй части алгоритма в секундах.

Пример:
```./app -p ../data/vrp_problems/1.json -s ../tests/vrp_temp/1.json -t 10```
