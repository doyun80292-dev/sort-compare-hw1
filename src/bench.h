/* bench.h - 입력 만들기, 시간 재기, 안정성 확인 */
#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>

#include "sort.h"

/* key로 정렬, tag = 원래 순서. 정렬 후 같은 key끼리 tag가 오름차순이면 안정 */
typedef struct Record {
    int key;
    int tag;
} Record;

int recordCompare(const void *a, const void *b); /* key만 본다 */

typedef enum InputKind {
    INPUT_RANDOM,     /* 무작위 */
    INPUT_SORTED,     /* 이미 정렬됨 */
    INPUT_REVERSED,   /* 역순 */
    INPUT_FEW_UNIQUE, /* 서로 다른 key가 8개뿐 */
    INPUT_KIND_COUNT
} InputKind;

const char *inputKindName(InputKind kind); /* 표 출력용 */
const char *inputKindKey(InputKind kind);  /* csv용 */

/* seed 같으면 항상 같은 입력 */
void makeInput(Record *a, size_t n, InputKind kind, unsigned long seed);

int recordsSorted(const Record *a, size_t n);
int recordsStable(const Record *a, size_t n);

typedef struct BenchResult {
    const SortAlgorithm *algo;
    size_t n;
    double millis;   /* reps회 평균 */
    SortStats stats; /* 마지막 회차 */
    int sorted;
    int stable;
} BenchResult;

BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps);

#endif
