/* sort.h - 정렬 3개(병합, 퀵, 힙)를 같은 방식으로 부르기 위한 헤더
 * 샘플 저장소(hw1-sample-2026)의 구조체 + 함수 포인터 방식을 참고함 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>

/* qsort와 같은 비교 함수 형태 (a<b 음수, 같으면 0, a>b 양수) */
typedef int (*SortCompare)(const void *a, const void *b);

/* 정렬 한 번 할 때 세는 값들 */
typedef struct SortStats {
    size_t compares;   /* 비교 횟수 */
    size_t moves;      /* 원소 복사 횟수 (swap 1번 = 3) */
    size_t extraBytes; /* 따로 malloc한 메모리 */
    size_t maxDepth;   /* 최대 재귀 깊이 */
} SortStats;

typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;
    const char *spaceComplexity;
    int stable; /* 안정 정렬이면 1 */
    void (*sort)(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
} SortAlgorithm;

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

/* 퀵 정렬 피벗 선택 방법 (실험용으로 바꿀 수 있게 전역변수로 둠) */
typedef enum QuickPivot {
    PIVOT_RANDOM, /* 랜덤 원소를 피벗으로 */
    PIVOT_FIRST   /* 수업 코드처럼 첫 원소 */
} QuickPivot;

extern QuickPivot quickSortPivot;

void sortStatsReset(SortStats *stats);
int sortCompareInt(const void *a, const void *b);

/* 수업 topic-04 01_random 에 나온 minstd 난수 생성기 */
typedef struct Minstd {
    unsigned long state;
} Minstd;

void minstdSeed(Minstd *g, unsigned long seed);
unsigned long minstdNext(Minstd *g);

#endif
