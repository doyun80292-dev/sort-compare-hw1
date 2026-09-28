/* quickSort.c - 퀵 정렬
 * partition은 수업 코드(topic-04 quickSort.c)랑 같음 (첫 원소 피벗, a[j] < pivot 이면 왼쪽).
 * 수업 README에서 말한 대로 랜덤 피벗 옵션을 추가함. */
#include "sort.h"

#include "sortctx.h"

QuickPivot quickSortPivot = PIVOT_RANDOM;

static size_t partition(SortCtx *c, Minstd *rng, size_t lo, size_t hi) {
    if (quickSortPivot == PIVOT_RANDOM) {
        size_t r = lo + (size_t)(minstdNext(rng) % (hi - lo + 1));
        sortSwap(c, lo, r); /* 랜덤으로 고른 걸 맨 앞으로 보내고 나머진 똑같이 */
    }
    size_t i = lo;
    for (size_t j = lo + 1; j <= hi; j++) {
        /* < 라서 피벗이랑 같은 값은 다 오른쪽으로 감 (중복 많으면 느려지는 원인) */
        if (sortCompareAt(c, j, lo) < 0) {
            i++;
            sortSwap(c, i, j);
        }
    }
    sortSwap(c, lo, i);
    return i;
}

static void quickSortRange(SortCtx *c, Minstd *rng, size_t lo, size_t hi, size_t depth) {
    sortEnterDepth(c, depth);
    if (lo >= hi) {
        return;
    }
    size_t p = partition(c, rng, lo, hi);
    if (p > lo) { /* size_t 라서 p - 1 이 음수 되는 거 방지 */
        quickSortRange(c, rng, lo, p - 1, depth + 1);
    }
    quickSortRange(c, rng, p + 1, hi, depth + 1);
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    Minstd rng;
    minstdSeed(&rng, 20260930UL); /* 시드 고정 -> 결과 재현 가능 */
    quickSortRange(&c, &rng, 0, n - 1, 1);
    sortEnd(&c);
}
