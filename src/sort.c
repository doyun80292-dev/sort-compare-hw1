/* sort.c - 공통 함수들이랑 정렬 목록(SORT_ALGORITHMS) */
#include "sort.h"

#include <stdlib.h>
#include <string.h>

#include "sortctx.h"

void sortStatsReset(SortStats *stats) {
    if (stats == NULL) {
        return;
    }
    stats->compares = 0;
    stats->moves = 0;
    stats->extraBytes = 0;
    stats->maxDepth = 1;
}

int sortCompareInt(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y); /* x - y 로 하면 overflow 날 수 있음 */
}

/* minstd: state = state * 16807 mod (2^31 - 1) */
void minstdSeed(Minstd *g, unsigned long seed) {
    seed %= 2147483647UL;
    g->state = seed == 0 ? 1 : seed;
}

unsigned long minstdNext(Minstd *g) {
    g->state = (unsigned long)((unsigned long long)g->state * 16807ULL % 2147483647ULL);
    return g->state;
}

int sortBegin(SortCtx *c, void *base, size_t n, size_t size,
              SortCompare cmp, SortStats *stats) {
    sortStatsReset(stats);
    if (base == NULL || cmp == NULL || size == 0 || n < 2) {
        return 0;
    }
    c->base = (char *)base;
    c->size = size;
    c->cmp = cmp;
    c->stats = stats;
    c->tmp = (char *)malloc(size);
    if (c->tmp == NULL) {
        return 0;
    }
    if (stats != NULL) {
        stats->extraBytes = size;
    }
    return 1;
}

void sortEnd(SortCtx *c) {
    free(c->tmp);
    c->tmp = NULL;
}

char *sortElemAt(const SortCtx *c, size_t i) {
    return c->base + i * c->size;
}

int sortComparePtr(SortCtx *c, const void *x, const void *y) {
    if (c->stats != NULL) {
        c->stats->compares++;
    }
    return c->cmp(x, y);
}

int sortCompareAt(SortCtx *c, size_t i, size_t j) {
    return sortComparePtr(c, sortElemAt(c, i), sortElemAt(c, j));
}

void sortMove(SortCtx *c, void *dst, const void *src) {
    memcpy(dst, src, c->size);
    if (c->stats != NULL) {
        c->stats->moves++;
    }
}

void sortSwap(SortCtx *c, size_t i, size_t j) {
    if (i == j) {
        return; /* 수업 코드처럼 자기 자신이랑은 안 바꿈 */
    }
    sortMove(c, c->tmp, sortElemAt(c, i));
    sortMove(c, sortElemAt(c, i), sortElemAt(c, j));
    sortMove(c, sortElemAt(c, j), c->tmp);
}

void sortEnterDepth(SortCtx *c, size_t depth) {
    if (c->stats != NULL && depth > c->stats->maxDepth) {
        c->stats->maxDepth = depth;
    }
}

/* 정렬 추가하면 여기에 한 줄 추가 */
const SortAlgorithm SORT_ALGORITHMS[] = {
    {"mergeSort", "n log n / n log n", "O(n)",     1, mergeSort},
    {"quickSort", "n log n / n^2",     "O(log n)", 0, quickSort},
    {"heapSort",  "n log n / n log n", "O(1)",     0, heapSort},
};

const size_t SORT_ALGORITHM_COUNT = sizeof(SORT_ALGORITHMS) / sizeof(SORT_ALGORITHMS[0]);
