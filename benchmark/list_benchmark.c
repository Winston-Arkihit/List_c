#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { ELEMENTS = 200000, TESTS = 1000, CHANGES_PER_TEST = 100 };

typedef struct {
    int data;
    int next;
    int prev;
} Node;

typedef struct {
    Node *nodes;
    int free_head;
    int guard_node;
    int capacity;
} List;

static volatile long long result_guard = 0;

static int init_list(List *list, int elements) {
    list->capacity = elements + 1;
    list->guard_node = elements;
    list->nodes = (Node *)calloc((size_t)list->capacity, sizeof(Node));
    if (list->nodes == NULL) return 0;

    list->free_head = 0;
    list->nodes[list->guard_node].next = list->guard_node;
    list->nodes[list->guard_node].prev = list->guard_node;
    for (int i = 0; i < elements; ++i) {
        list->nodes[i].next = (i + 1 < elements) ? i + 1 : -1;
        list->nodes[i].prev = -1;
    }
    return 1;
}

static int insert_after(List *list, int target, int value) {
    if (list->free_head == -1) return -1;

    int new_index = list->free_head;
    list->free_head = list->nodes[new_index].next;
    int next_index = list->nodes[target].next;

    list->nodes[new_index].data = value;
    list->nodes[new_index].next = next_index;
    list->nodes[new_index].prev = target;
    list->nodes[target].next = new_index;
    list->nodes[next_index].prev = new_index;
    return new_index;
}

static void remove_node(List *list, int index) {
    int previous = list->nodes[index].prev;
    int next = list->nodes[index].next;

    list->nodes[previous].next = next;
    list->nodes[next].prev = previous;
    list->nodes[index].next = list->free_head;
    list->nodes[index].prev = -1;
    list->free_head = index;
}

static double now_seconds(void) {
    struct timespec value;
    timespec_get(&value, TIME_UTC);
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
}

int main(void) {
    List list;
    if (!init_list(&list, ELEMENTS)) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    int tail = list.guard_node;
    for (int i = 0; i < ELEMENTS; ++i) {
        tail = insert_after(&list, tail, i + 1);
        if (tail == -1) return 1;
    }

    int middle = list.nodes[list.guard_node].next;
    for (int i = 0; i < ELEMENTS / 2; ++i) middle = list.nodes[middle].next;
    int before_middle = list.nodes[middle].prev;

    double min_time = 1e100;
    double total_begin = now_seconds();
    for (int test = 0; test < TESTS; ++test) {
        double begin = now_seconds();

        for (int change = 0; change < CHANGES_PER_TEST; ++change) {
            int saved_value = list.nodes[middle].data;
            remove_node(&list, middle);
            middle = insert_after(&list, before_middle, saved_value);
            if (middle == -1) return 1;
        }

        long long sum = 0;
        int current = list.nodes[list.guard_node].next;
        while (current != list.guard_node) {
            sum += list.nodes[current].data;
            current = list.nodes[current].next;
        }
        double elapsed = now_seconds() - begin;
        if (elapsed < min_time) min_time = elapsed;
        result_guard += sum;
    }
    double total_end = now_seconds();

    double total = total_end - total_begin;
    printf("LIST benchmark\n");
    printf("Elements: %d\nTests: %d\n", ELEMENTS, TESTS);
    printf("Delete+insert pairs per test: %d\n", CHANGES_PER_TEST);
    printf("Total time: %.6f s\n", total);
    printf("Average test: %.6f ms\n", total * 1000.0 / TESTS);
    printf("Minimum test: %.6f ms\n", min_time * 1000.0);
    printf("Checksum: %lld\n", result_guard);

    free(list.nodes);
    return 0;
}
