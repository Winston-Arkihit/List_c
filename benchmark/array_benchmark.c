#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { ELEMENTS = 200000, TESTS = 1000, CHANGES_PER_TEST = 100 };

static volatile long long result_guard = 0;

static double now_seconds(void) {
    struct timespec value;
    timespec_get(&value, TIME_UTC);
    return (double)value.tv_sec + (double)value.tv_nsec / 1000000000.0;
}

int main(void) {
    int *array = (int *)calloc((size_t)ELEMENTS * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < ELEMENTS; ++i) array[i] = i + 1;

    double min_time = 1e100;
    double total_begin = now_seconds();
    for (int test = 0; test < TESTS; ++test) {
        double begin = now_seconds();

        int size = ELEMENTS;
        const int middle = ELEMENTS / 2;
        for (int change = 0; change < CHANGES_PER_TEST; ++change) {
            int saved_value = array[middle];

            /* Удаление из середины: сдвигаем хвост массива влево. */
            memmove(&array[middle], &array[middle + 1],
                    (size_t)(size - middle - 1) * sizeof(array[0]));
            --size;

            /* Вставка обратно: освобождаем место сдвигом вправо. */
            memmove(&array[middle + 1], &array[middle],
                    (size_t)(size - middle) * sizeof(array[0]));
            array[middle] = saved_value;
            ++size;
        }

        long long sum = 0;
        for (int i = 0; i < size; ++i) sum += array[i];
        double elapsed = now_seconds() - begin;
        if (elapsed < min_time) min_time = elapsed;
        result_guard += sum;
    }
    double total_end = now_seconds();

    double total = total_end - total_begin;
    printf("ARRAY benchmark\n");
    printf("Elements: %d\nTests: %d\n", ELEMENTS, TESTS);
    printf("Delete+insert pairs per test: %d\n", CHANGES_PER_TEST);
    printf("Total time: %.6f s\n", total);
    printf("Average test: %.6f ms\n", total * 1000.0 / TESTS);
    printf("Minimum test: %.6f ms\n", min_time * 1000.0);
    printf("Checksum: %lld\n", result_guard);

    free(array);
    return 0;
}
