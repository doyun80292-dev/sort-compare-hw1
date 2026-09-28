/* heapSort.c - 힙 정렬 (수업에서 안 배운 정렬)
 *
 * 배열을 완전 이진 트리로 봄 (0번부터)
 *   왼쪽 자식 = 2i+1, 오른쪽 자식 = 2i+2, 부모 = (i-1)/2
 * 1) 최대 힙 만들기: n/2-1 번부터 0번까지 siftDown
 * 2) a[0](최댓값)을 맨 뒤랑 바꾸고 힙 크기 1 줄이고 siftDown, 반복
 */
#include "sort.h"

#include "sortctx.h"

/* a[i]를 자기 자리까지 내려보냄 (힙 크기 n) */
static void siftDown(SortCtx *c, size_t i, size_t n) {
    while (1) {
        size_t child = 2 * i + 1;
        if (child >= n) {
            return; /* 자식 없음 */
        }
        /* 두 자식 중 큰 쪽 */
        if (child + 1 < n && sortCompareAt(c, child + 1, child) > 0) {
            child++;
        }
        if (sortCompareAt(c, child, i) <= 0) {
            return; /* 자식보다 크거나 같으면 끝 */
        }
        sortSwap(c, i, child);
        i = child;
    }
}

void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 1) 힙 만들기. i가 size_t라서 i-- > 0 으로 씀 */
    for (size_t i = n / 2; i-- > 0;) {
        siftDown(&c, i, n);
    }
    /* 2) 최댓값을 뒤로 */
    for (size_t end = n - 1; end > 0; end--) {
        sortSwap(&c, 0, end);
        siftDown(&c, 0, end);
    }
    sortEnd(&c);
}
