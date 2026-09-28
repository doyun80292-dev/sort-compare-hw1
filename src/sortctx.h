/* sortctx.h - 정렬 코드끼리만 쓰는 도구
 * 비교/이동을 전부 이 함수들로 해야 세 정렬이 같은 기준으로 세어진다 */
#ifndef SORTCTX_H
#define SORTCTX_H

#include <stddef.h>

#include "sort.h"

typedef struct SortCtx {
    char *base;
    size_t size;
    SortCompare cmp;
    SortStats *stats;
    char *tmp; /* swap할 때 쓰는 원소 한 칸 */
} SortCtx;

int sortBegin(SortCtx *c, void *base, size_t n, size_t size,
              SortCompare cmp, SortStats *stats);
void sortEnd(SortCtx *c);

char *sortElemAt(const SortCtx *c, size_t i);
int sortCompareAt(SortCtx *c, size_t i, size_t j);
int sortComparePtr(SortCtx *c, const void *x, const void *y);
void sortMove(SortCtx *c, void *dst, const void *src);
void sortSwap(SortCtx *c, size_t i, size_t j);
void sortEnterDepth(SortCtx *c, size_t depth);

#endif
