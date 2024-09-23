#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESCAPE 27

void display_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    // Подсчитываем количество строк и символов в файле
    int num_lines = 0;
    int num_chars = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        num_chars++; // Считаем каждый символ
        if (ch == '\n') {
            num_lines++; // Считаем строки
        }
    }
    rewind(file); // Возвращаем указатель файла в начало

    // Получаем размеры терминала
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    rows -= 2; // Уменьшаем на 2 для учета рамки и строки с именем файла

    // Выделяем память для хранения строк файла
    char **lines = malloc(sizeof(char*) * num_lines);
    if (!lines) {
        perror("Memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Чтение файла построчно
    size_t len = 0;
    for (int i = 0; i < num_lines; i++) {
        lines[i] = NULL;
        if (getline(&lines[i], &len, file) == -1) {
            break;
        }
    }
    fclose(file);

    int offset = 0; // текущая позиция просмотра в файле

    // Цветовая схема
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Параметры для зелёной рамки

    while (1) {
        // Очищаем окно и добавляем рамку
        werase(stdscr);
        attron(COLOR_PAIR(1));  // Включаем цвет
        box(stdscr, 0, 0);      // Отрисовываем рамку
        attroff(COLOR_PAIR(1)); // Выключаем цвет

        // Печатаем имя файла, количество строк и символов в первой строке
        mvprintw(0, 2, " File: %s | Lines: %d | Chars: %d ", filename, num_lines, num_chars);

        // Печатаем строки файла постранично внутри рамки с учётом переноса длинных строк
        int line_index = offset;
        int current_row = 1;  // Стартовая позиция вывода текста
        while (current_row < rows && line_index < num_lines) {
            char *line = lines[line_index];
            int line_length = strlen(line);
            int remaining_length = line_length;
            int start = 0;

            // Печатаем строку с учётом её переноса на несколько строк экрана
            while (remaining_length > 0 && current_row < rows) {
                int print_length = (remaining_length > cols - 2) ? cols - 2 : remaining_length;
                mvprintw(current_row, 1, "%.*s", print_length, line + start);
                remaining_length -= print_length;
                start += print_length;
                current_row++;
            }
            line_index++;
        }

        // Обновляем экран
        refresh();

        // Ожидаем ввод пользователя
        int ch = getch();
        if (ch == ' ' && offset + rows < num_lines) {
            offset += rows; // Прокручиваем файл вниз на одну страницу
        } else if (ch == ESCAPE) {
            break; // Завершаем программу по ESC
        }
    }

    // Освобождаем память
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Инициализация ncurses
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // Вывод файла
    display_file(argv[1]);

    // Завершение работы ncurses
    endwin();
    return 0;
}

