#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    int *next;
    int *prev;
    int head;      // Индекс первого логического элемента (-1 если пуст)
    int tail;      // Индекс последнего логического элемента (-1 если пуст)
    int free_head; // Индекс первой свободной ячейки для записи
    int capacity;  // Размер массива
} List;

//ИНИЦИАЛИЗАЦИЯ СПИСКА
void initList(List *list, int user_capacity) {
    list->capacity = user_capacity;
    list->data = (int *)calloc(list->capacity, sizeof(int));
    list->next = (int *)calloc(list->capacity, sizeof(int));
    list->prev = (int *)calloc(list->capacity, sizeof(int));

    list->head = -1; 
    list->tail = -1;
    list->free_head = 0; // Начальная ячейка нолевая?
    for (int i = 0; i < list->capacity; i++) {
        list->next[i] = i + 1;
        list->prev[i] = -1;
    }
    list->next[list->capacity - 1] = -1; // Последняя ячейка указывает в несущевование?
}

// ВССТАВКА + Сортировка
void insertSorted(List *list, int new_val) {
    if (list->free_head == -1) {
        printf("Error. List is full. can`t add new elements %d\n", new_val);
        return;
    }

    //Забираем первую свободную ячейку из пула free_head
    int new_idx = list->free_head;
    list->free_head = list->next[new_idx]; //Двигаем указатель на следующую свободную
    // Запись
    list->data[new_idx] = new_val;
    list->next[new_idx] = -1;
    list->prev[new_idx] = -1;

    //Если список пустой или новое число <= head
    if (list->head == -1 || list->data[list->head] >= new_val) {
        list->next[new_idx] = list->head;
        if (list->head != -1) {
            list->prev[list->head] = new_idx;
        } else {
            list->tail = new_idx; // Если был пустой то хвост
        }
        list->head = new_idx; // новый элемент становится головой списка
        return;
    }

    // Поиск индекса для вставки в списке
    int curr = list->head;

    while (list->next[curr] != -1 && list->data[list->next[curr]] < new_val) {
        curr = list->next[curr];
    }

    //curr - индекс элемента, после которого вставляем
    int next_idx = list->next[curr]; // Индекс элемента перед которым вставляем

    //смена указателей
    list->next[new_idx] = next_idx;
    list->prev[new_idx] = curr;

    list->next[curr] = new_idx;

    if (next_idx != -1) {
        list->prev[next_idx] = new_idx;
    } else {
        list->tail = new_idx; // если засунули в самый конец, нужно обновить tail
    }
}

//ПЕЧАТЬ ЛОГИЧЕСКОГО ПОРЯДКА СПИСКА
void printListLogically(List *list) {
    int curr = list->head;
    printf("Logical order of elements: ");
    if (curr == -1) {
        printf("Пусто\n");
        return;
    }
    while (curr != -1) {
        printf("[%d: %d] <-> ", curr, list->data[curr]);
        curr = list->next[curr];
    }
    printf("NULL\n");
}


void freeListMemory(List *list) {
    free(list->data);
    free(list->next);
    free(list->prev);
}

int main() {
    List myList;
    int dynamic_size = 5; 
    initList(&myList, dynamic_size);
    insertSorted(&myList, 40);
    insertSorted(&myList, 20);
    insertSorted(&myList, 30);
    printListLogically(&myList);
    // Проверка защиты от заполнения
    insertSorted(&myList, 10);
    insertSorted(&myList, 50);

    // не должен влезть потому что размер = 5 
    insertSorted(&myList, 60);
    printListLogically(&myList);
    freeListMemory(&myList);
    return 0;
}
