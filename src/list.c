#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Элемент списка (нода)
typedef struct {
    int data;
    int next;
    int prev;
} Node;

typedef struct {
    Node *nodes;       // Массив структур вместо трех отдельных массивов
    int free_head;     // Индекс первой свободной ячейки для записи
    int guard_node;    // Индекс фиктивного элемента-стража, который закольцовывает список
    int capacity;      // Размер массива
} List;

// Статический счётчик для создания уникальных имён картинок png
static int step_counter = 0;

// ==========================================
// ГЕТТЕРЫ И СЕТТЕРЫ
// ==========================================

int getNodeData(const List *list, int idx) {
    return list->nodes[idx].data;
}

void setNodeData(List *list, int idx, int val) {
    list->nodes[idx].data = val;
}

int getNodeNext(const List *list, int idx) {
    return list->nodes[idx].next;
}

void setNodeNext(List *list, int idx, int next_idx) {
    list->nodes[idx].next = next_idx;
}

int getNodePrev(const List *list, int idx) {
    return list->nodes[idx].prev;
}

void setNodePrev(List *list, int idx, int prev_idx) {
    list->nodes[idx].prev = prev_idx;
}

// ==========================================
// ИНИЦИАЛИЗАЦИЯ И ЗАКРЫТИЕ ЛОГ-ФАЙЛА
// ==========================================

void initHtmlLog(const char *filename) {
    FILE *f = fopen(filename, "w"); // Стираем старый лог
    if (!f) return;

    fprintf(f, "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n");
    fprintf(f, "<title>List System Graphviz Session Log</title>\n");
    fprintf(f, "<style>\n");
    fprintf(f, "  body { font-family: monospace; background: #222222; color: #dddddd; padding: 20px; }\n");
    fprintf(f, "  .session-title { font-size: 1.4em; font-weight: bold; color: #ffffff; margin-bottom: 20px; border-bottom: 1px solid #555555; padding-bottom: 5px; }\n");
    fprintf(f, "  .step-block { border-bottom: 2px solid #444444; padding: 15px 0; margin-bottom: 15px; }\n");
    fprintf(f, "  .step-header { font-size: 1.1em; color: #ffffff; font-weight: bold; margin-bottom: 5px; }\n");
    fprintf(f, "  .meta-info { color: #aaaaaa; margin-bottom: 10px; font-size: 0.9em; }\n");
    fprintf(f, "  .logic-chain { font-size: 0.95em; margin-top: 10px; color: #ffffff; }\n");
    fprintf(f, "  img { background: #2b2b2b; padding: 15px; border: 1px solid #444444; max-width: 100%%; height: auto; display: block; margin: 10px 0; }\n");
    fprintf(f, "</style>\n</head>\n<body>\n");
    fprintf(f, "<div class=\"session-title\">LIST DUMP SESSION LOG (SYSTEM GRAPHVIZ)</div>\n");

    fclose(f);
}

void closeHtmlLog(const char *filename) {
    FILE *f = fopen(filename, "a");
    if (!f) return;
    fprintf(f, "</body>\n</html>\n");
    fclose(f);
}

// ==========================================
// !!! ЗАМЕТНАЯ ВСТАВКА ПОД SYSTEM LOG !!!
// ==========================================

// Запись текущего состояния. Генерирует DOT, вызывает Graphviz через system() и вставляет PNG в HTML
void logHtmlStep(const List *list, const char *filename, const char *operation_name) {
    char dot_filename[] = "temp_graph.dot";
    char png_filename[64];
    char sys_cmd[256];
    
    // Формируем уникальное имя для картинки текущего шага
    sprintf(png_filename, "step_%d.png", step_counter++);

    // ОТКРЫВАЕМ СТРОГО DOT-ФАЙЛ ДЛЯ ЗАПИСИ ГРАФА
    FILE *dot_f = fopen(dot_filename, "w");
    if (!dot_f) return;

    // Генерируем чистый DOT-код Graphviz именно в dot_f, а не в HTML!
    fprintf(dot_f, "digraph G {\n");
    fprintf(dot_f, "  bgcolor=\"#2b2b2b\";\n");
    fprintf(dot_f, "  rankdir=LR;\n"); // Горизонтальное расположение (слева направо)
    fprintf(dot_f, "  edge [fontname=\"monospace\", fontsize=9, arrowsize=0.7];\n");
    fprintf(dot_f, "  node [fontname=\"monospace\", fontsize=10];\n");

    // 1. Отрисовка указателей-меток (HEAD, TAIL, FREE)
    int head_idx = getNodeNext(list, list->guard_node);
    int tail_idx = getNodePrev(list, list->guard_node);

    fprintf(dot_f, "  head_ptr [shape=box, style=\"filled,rounded\", fillcolor=\"#ffd54f\", color=\"#ffb300\", fontcolor=\"#000000\", label=\"HEAD\"];\n");
    fprintf(dot_f, "  tail_ptr [shape=box, style=\"filled,rounded\", fillcolor=\"#ce93d8\", color=\"#ab47bc\", fontcolor=\"#ffffff\", label=\"TAIL\"];\n");
    fprintf(dot_f, "  free_ptr [shape=box, style=\"filled,rounded\", fillcolor=\"#a5d6a7\", color=\"#66bb6a\", fontcolor=\"#000000\", label=\"FREE\"];\n");

    // 2. Создаем элементы в виде многострочных ячеек (Mrecord)
    for (int i = 0; i < list->capacity; i++) {
        int is_free = 0;
        if (i != list->guard_node) {
            int curr_free = list->free_head;
            while (curr_free != -1) {
                if (curr_free == i) { is_free = 1; break; }
                curr_free = getNodeNext(list, curr_free);
            }
        }

        const char *bg = "#3c4d3c";
        const char *border = "#66bb6a";
        if (i == list->guard_node) { bg = "#4d3c3c"; border = "#ef5350"; }
        else if (is_free) { bg = "#2a2a2a"; border = "#555555"; }

        if (i == list->guard_node) {
            fprintf(dot_f, "  node_%d [shape=Mrecord, style=filled, fillcolor=\"%s\", color=\"%s\", fontcolor=\"#ffffff\", label=\"index: %d (GUARD)|value: %d|<p> prev: %d | <n> next: %d\"];\n", 
                    i, bg, border, i, getNodeData(list, i), getNodePrev(list, i), getNodeNext(list, i));
        } else if (is_free) {
            fprintf(dot_f, "  node_%d [shape=Mrecord, style=filled, fillcolor=\"%s\", color=\"%s\", fontcolor=\"#888888\", label=\"index: %d (FREE)|value: %d|<p> prev: %d | <n> next: %d\"];\n", 
                    i, bg, border, i, getNodeData(list, i), getNodePrev(list, i), getNodeNext(list, i));
        } else {
            fprintf(dot_f, "  node_%d [shape=Mrecord, style=filled, fillcolor=\"%s\", color=\"%s\", fontcolor=\"#ffffff\", label=\"index: %d|value: %d|<p> prev: %d | <n> next: %d\"];\n", 
                    i, bg, border, i, getNodeData(list, i), getNodePrev(list, i), getNodeNext(list, i));
        }
    }

    // Привязываем указатели-метки к соответствующим узлам
    fprintf(dot_f, "  head_ptr -> node_%d [color=\"#ffd54f\", penwidth=1.5];\n", head_idx);
    fprintf(dot_f, "  tail_ptr -> node_%d [color=\"#ce93d8\", penwidth=1.5];\n", tail_idx);
    if (list->free_head != -1) {
        fprintf(dot_f, "  free_ptr -> node_%d [color=\"#a5d6a7\", style=dashed, penwidth=1.5];\n", list->free_head);
    }

    // 3. Направленные стрелочки, выходящие из конкретных портов полей ноды
    for (int i = 0; i < list->capacity; i++) {
        int next = getNodeNext(list, i);
        int prev = getNodePrev(list, i);

        int is_free = 0;
        if (i != list->guard_node) {
            int curr_free = list->free_head;
            while (curr_free != -1) {
                if (curr_free == i) { is_free = 1; break; }
                curr_free = getNodeNext(list, curr_free);
            }
        }

        if (is_free) {
            if (next != -1) {
                fprintf(dot_f, "  node_%d:n -> node_%d [color=\"#888888\", style=dashed, constraint=false];\n", i, next);
            }
        } else {
            if (next >= 0 && next < list->capacity) {
                fprintf(dot_f, "  node_%d:n -> node_%d [color=\"#66bb6a\", penwidth=1.2, constraint=false];\n", i, next);
            }
            if (prev >= 0 && prev < list->capacity) {
                fprintf(dot_f, "  node_%d:p -> node_%d [color=\"#42a5f5\", style=dashed, penwidth=1.2, constraint=false];\n", i, prev);
            }
        }
    }

    fprintf(dot_f, "}\n");
    fclose(dot_f); // Закрываем DOT-файл, он полностью готов для компиляции

    // ВЫЗЫВАЕМ GRAPHVIZ ПО ПРЯМОМУ СИСТЕМНОМУ ПУТИ
    sprintf(sys_cmd, "\"C:\\Program Files\\Graphviz\\bin\\dot.exe\" -Tpng %s -o %s", dot_filename, png_filename);
    system(sys_cmd);

    // Удаляем временный текстовый .dot файл
    remove(dot_filename);

    // ОТКРЫВАЕМ HTML-ЛОГ ДЛЯ ЗАПИСИ ШАГА
    FILE *html_f = fopen(filename, "a");
    if (!html_f) return;

    fprintf(html_f, "<div class=\"step-block\">\n");
    fprintf(html_f, "  <div class=\"step-header\">> %s</div>\n", operation_name);
    fprintf(html_f, "  <div class=\"meta-info\">capacity: %d | guard: %d | free_head: %d</div>\n", 
            list->capacity, list->guard_node, list->free_head);

    // Вставляем скомпилированную картинку в HTML
    fprintf(html_f, "  <img src=\"%s\" alt=\"List Graph State\">\n", png_filename);

    // Вывод текстового логического пути под картинкой
    fprintf(html_f, "  <div class=\"logic-chain\">\n");
    fprintf(html_f, "    <b>Logical order:</b> guard(%d)", list->guard_node);
    
    int curr = getNodeNext(list, list->guard_node);
    while (curr != list->guard_node) {
        fprintf(html_f, " -> [%d: %d]", curr, getNodeData(list, curr));
        curr = getNodeNext(list, curr);
    }
    fprintf(html_f, " -> guard(%d)\n", list->guard_node);
    fprintf(html_f, "  </div>\n");
    fprintf(html_f, "</div>\n"); 

    fclose(html_f);
}


// ==========================================
// ИНИЦИАЛИЗАЦИЯ И ЛОГИКА СПИСКА
// ==========================================

void initList(List *list, int user_capacity) {
    list->capacity = user_capacity;
    list->nodes = (Node *)calloc(list->capacity, sizeof(Node));

    list->guard_node = list->capacity - 1;

    setNodeNext(list, list->guard_node, list->guard_node);
    setNodePrev(list, list->guard_node, list->guard_node);

    list->free_head = 0; 
    for (int i = 0; i < list->capacity - 1; i++) {
        setNodeNext(list, i, i + 1);
        setNodePrev(list, i, -1);
    }
    setNodeNext(list, list->capacity - 2, -1);
}

int insertAtIndex(List *list, int target_idx, int new_val) {
    if (list->free_head == -1) {
        printf("Error. List is full. Can't add new element: %d\n", new_val);
        return -1;
    }

    int new_idx = list->free_head;
    list->free_head = getNodeNext(list, new_idx);

    setNodeData(list, new_idx, new_val);

    int next_idx = getNodeNext(list, target_idx);

    setNodeNext(list, new_idx, next_idx);
    setNodePrev(list, new_idx, target_idx);

    setNodeNext(list, target_idx, new_idx);
    setNodePrev(list, next_idx, new_idx);

    return new_idx;
}

void removeAtIndex(List *list, int remove_idx) {
    if (remove_idx < 0 || remove_idx >= list->capacity || remove_idx == list->guard_node) {
        printf("Error: Invalid remove index: %d\n", remove_idx);
        return;
    }

    int prev_idx = getNodePrev(list, remove_idx);
    int next_idx = getNodeNext(list, remove_idx);

    setNodeNext(list, prev_idx, next_idx);
    setNodePrev(list, next_idx, prev_idx);

    setNodeNext(list, remove_idx, list->free_head);
    setNodePrev(list, remove_idx, -1);
    list->free_head = remove_idx;
}

void freeListMemory(List *list) {
    free(list->nodes);
}

int main() {
    const char *log_file = "session_dump.html";
    
    initHtmlLog(log_file);

    List myList;
    int dynamic_size = 6; 
    initList(&myList, dynamic_size);
    
    logHtmlStep(&myList, log_file, "Creating an array with empty list");

    int idx1 = insertAtIndex(&myList, myList.guard_node, 12);
    logHtmlStep(&myList, log_file, "Adding element: 12");

    int idx2 = insertAtIndex(&myList, idx1, 13);
    logHtmlStep(&myList, log_file, "Adding element: 13 after 12");

    int idx3 = insertAtIndex(&myList, idx2, 14);
    logHtmlStep(&myList, log_file, "Adding element: 14 after 13");

    removeAtIndex(&myList, idx2);
    logHtmlStep(&myList, log_file, "Removing element: 13");

    closeHtmlLog(log_file);

    freeListMemory(&myList);
    printf("Success! HTML log generated in '%s'.\n", log_file);
    return 0;
}
