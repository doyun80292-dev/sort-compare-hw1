/* mergeSort.c - 병합 정렬
 * 수업 코드(topic-03 mergeSort.c)를 void* 로 바꾼 것. temp 배열 n칸을 더 씀 */
#include "sort.h"

#include <stdlib.h>

#include "sortctx.h"

/* a[lo..mid], a[mid+1..hi] 를 합친다 */
static void merge(SortCtx *c, char *temp, size_t lo, size_t mid, size_t hi) {
    size_t i = lo;
    size_t j = mid + 1;
    size_t k = lo;
    size_t s = c->size;

    while (i <= mid && j <= hi) {
        if (sortCompareAt(c, i, j) <= 0) { /* <= 라서 같으면 왼쪽 먼저 -> 안정 */
            sortMove(c, temp + (k++) * s, sortElemAt(c, i++));
        } else {
            sortMove(c, temp + (k++) * s, sortElemAt(c, j++));
        }
    }
    while (i <= mid) {
        sortMove(c, temp + (k++) * s, sortElemAt(c, i++));
    }
    while (j <= hi) {
        sortMove(c, temp + (k++) * s, sortElemAt(c, j++));
    }
    for (k = lo; k <= hi; k++) {
        sortMove(c, sortElemAt(c, k), temp + k * s);
    }
}

static void mergeSortRange(SortCtx *c, char *temp, size_t lo, size_t hi, size_t depth) {
    sortEnterDepth(c, depth);
    if (lo >= hi) {
        return;
    }
    size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(c, temp, lo, mid, depth + 1);
    mergeSortRange(c, temp, mid + 1, hi, depth + 1);
    merge(c, temp, lo, mid, hi);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    char *temp = (char *)malloc(n * size);
    if (temp == NULL) {
        sortEnd(&c);
        return;
    }
    if (stats != NULL) {
        stats->extraBytes += n * size;
    }
    mergeSortRange(&c, temp, 0, n - 1, 1);
    free(temp);
    sortEnd(&c);
}
