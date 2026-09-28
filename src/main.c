/* main.c - 정렬 비교 실행
 *   make run                 비교 표
 *   ./src/main.out --csv     같은 측정을 csv로 (그래프용)
 *   ./src/main.out --pivot   퀵 정렬 피벗 실험
 *   ./src/main.out --theory  시드 10개 평균 비교 횟수 (이론값 비교용)
 *   ./src/main.out --big     n = 100만, 400만, 1600만
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

#define SEED 20260901UL

typedef struct Spec {
    const char *scope; /* kinds: 입력 모양별 · growth: n을 두 배씩 */
    InputKind kind;
    size_t n;
    int reps;
} Spec;

static const Spec SPECS[] = {
    {"kinds", INPUT_RANDOM, 100000, 3},
    {"kinds", INPUT_SORTED, 100000, 3},
    {"kinds", INPUT_REVERSED, 100000, 3},
    {"kinds", INPUT_FEW_UNIQUE, 100000, 3},
    /* 배가 실험 (수업 topic-05) */
    {"growth", INPUT_RANDOM, 1000, 5},
    {"growth", INPUT_RANDOM, 2000, 5},
    {"growth", INPUT_RANDOM, 4000, 5},
    {"growth", INPUT_RANDOM, 8000, 5},
    {"growth", INPUT_RANDOM, 16000, 5},
    {"growth", INPUT_RANDOM, 32000, 5},
    {"growth", INPUT_RANDOM, 64000, 5},
    {"growth", INPUT_RANDOM, 128000, 5},
    {"growth", INPUT_RANDOM, 256000, 5},
    {"growth", INPUT_RANDOM, 512000, 5},
    {"growth", INPUT_RANDOM, 1024000, 5},
};

static const size_t SPEC_COUNT = sizeof(SPECS) / sizeof(SPECS[0]);

/* 표로 출력할지 csv로 출력할지 함수 포인터로 바꿈 */
typedef void (*RowSink)(const Spec *spec, const BenchResult *r);

static void measureAll(RowSink sink, void (*onSpec)(const Spec *spec)) {
    for (size_t s = 0; s < SPEC_COUNT; s++) {
        const Spec *spec = &SPECS[s];
        Record *input = (Record *)malloc(spec->n * sizeof(Record));
        if (input == NULL) {
            return;
        }
        makeInput(input, spec->n, spec->kind, SEED);
        if (onSpec != NULL) {
            onSpec(spec);
        }
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, spec->n, spec->reps);
            sink(spec, &r);
        }
        free(input);
    }
}

/* --- 사람이 읽는 표 ---------------------------------------------------- */

#define ROW_FORMAT "%-10s %10.3f %12zu %12zu %10zu B %6zu %5s %6s\n"
#define ROW_HEADER "알고리즘     시간(ms)         비교         이동     추가메모리 재귀깊이 정렬 안정성\n"
#define ROW_RULE   "-------------------------------------------------------------------------------------\n"

static void tableRow(const Spec *spec, const BenchResult *r) {
    (void)spec;
    printf(ROW_FORMAT, r->algo->name, r->millis, r->stats.compares, r->stats.moves,
           r->stats.extraBytes, r->stats.maxDepth, r->sorted ? "yes" : "NO!",
           r->stable ? "yes" : "no");
}

static void tableSpecHeader(const Spec *spec) {
    static const char *lastScope = NULL;
    if (lastScope == NULL || strcmp(lastScope, spec->scope) != 0) {
        if (strcmp(spec->scope, "kinds") == 0) {
            printf("입력 모양별 비교 (n = %zu, %d회 평균)\n", spec->n, spec->reps);
        } else {
            printf("\nn을 두 배씩 키우며 (무작위 입력, %d회 평균)\n", spec->reps);
        }
        lastScope = spec->scope;
    }
    if (strcmp(spec->scope, "kinds") == 0) {
        printf("\n[%s]\n", inputKindName(spec->kind));
    } else {
        printf("\n[n = %zu]\n", spec->n);
    }
    printf("%s%s", ROW_HEADER, ROW_RULE);
}

static void printDeclarations(void) {
    printf("구현 표 (SortAlgorithm이 주장하는 값)\n");
    printf("알고리즘   시간(평균/최악)      메모리     안정성\n");
    printf("%s", ROW_RULE);
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *a = &SORT_ALGORITHMS[k];
        printf("%-10s %-20s %-10s %s\n", a->name, a->timeComplexity, a->spaceComplexity,
               a->stable ? "stable" : "unstable");
    }
    printf("\n");
}

static void reportTable(void) {
    printf("=== 정렬 비교: 병합 · 퀵 · 힙 ===\n");
    printf("원소는 (key, tag) %zu바이트. key로 정렬하고 tag로 안정성을 본다.\n\n",
           sizeof(Record));
    printDeclarations();
    measureAll(tableRow, tableSpecHeader);
    printf("\n읽는 법\n");
    printf("  시간     : 같은 기계에서만 견준다. 비교·이동 횟수는 언제 돌려도 같다.\n");
    printf("  추가메모리: 입력 배열 밖에 malloc한 바이트. 병합 정렬만 n에 비례한다.\n");
    printf("  재귀깊이 : 스택 사용량의 대리 지표. 힙 정렬은 반복문뿐이라 1이다.\n");
    printf("  안정성   : 표의 주장이 아니라 tag 순서로 실측한 값이다.\n");
}

/* --- 기계가 읽는 CSV --------------------------------------------------- */

static void csvRow(const Spec *spec, const BenchResult *r) {
    printf("%s,%s,%zu,%s,%.4f,%zu,%zu,%zu,%zu,%d,%d\n", spec->scope,
           inputKindKey(spec->kind), spec->n, r->algo->name, r->millis,
           r->stats.compares, r->stats.moves, r->stats.extraBytes,
           r->stats.maxDepth, r->sorted, r->stable);
}

static void reportCsv(void) {
    printf("scope,input,n,algo,millis,compares,moves,extraBytes,maxDepth,sorted,stable\n");
    measureAll(csvRow, NULL);
}

/* --- 피벗 실험 --------------------------------------------------------- */

static const SortAlgorithm *findAlgorithm(const char *name) {
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        if (strcmp(SORT_ALGORITHMS[k].name, name) == 0) {
            return &SORT_ALGORITHMS[k];
        }
    }
    return NULL;
}

/* 첫 원소 피벗 vs 랜덤 피벗. 정렬된 입력+첫 원소면 재귀 깊이가 n이라 16000까지만 */
static void reportPivot(void) {
    static const size_t SIZES[] = {1000, 2000, 4000, 8000, 16000};
    static const InputKind KINDS[] = {INPUT_RANDOM, INPUT_SORTED, INPUT_FEW_UNIQUE};
    static const QuickPivot PIVOTS[] = {PIVOT_FIRST, PIVOT_RANDOM};
    const SortAlgorithm *algo = findAlgorithm("quickSort");
    const QuickPivot saved = quickSortPivot;

    if (algo == NULL) {
        return;
    }
    printf("input,pivot,n,compares,moves,maxDepth,millis,sorted\n");
    for (size_t k = 0; k < sizeof(KINDS) / sizeof(KINDS[0]); k++) {
        for (size_t s = 0; s < sizeof(SIZES) / sizeof(SIZES[0]); s++) {
            size_t n = SIZES[s];
            Record *input = (Record *)malloc(n * sizeof(Record));
            if (input == NULL) {
                return;
            }
            makeInput(input, n, KINDS[k], SEED);
            for (size_t p = 0; p < sizeof(PIVOTS) / sizeof(PIVOTS[0]); p++) {
                quickSortPivot = PIVOTS[p];
                BenchResult r = benchRun(algo, input, n, 3);
                printf("%s,%s,%zu,%zu,%zu,%zu,%.4f,%d\n", inputKindKey(KINDS[k]),
                       PIVOTS[p] == PIVOT_FIRST ? "first" : "random", n,
                       r.stats.compares, r.stats.moves, r.stats.maxDepth, r.millis,
                       r.sorted);
            }
            free(input);
        }
    }
    quickSortPivot = saved;
}

/* --- 이론값과 견주기 --------------------------------------------------- */

/* 이론식은 평균이라 시드 10개로 평균 냄. n은 2의 거듭제곱 */
static void reportTheory(void) {
    static const size_t SIZES[] = {1u << 10, 1u << 13, 1u << 16, 1u << 20};
    enum { SEEDS = 10 };
    printf("n,algo,meanCompares,minCompares,maxCompares\n");
    for (size_t s = 0; s < sizeof(SIZES) / sizeof(SIZES[0]); s++) {
        size_t n = SIZES[s];
        Record *input = (Record *)malloc(n * sizeof(Record));
        if (input == NULL) {
            return;
        }
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            double sum = 0.0;
            size_t lo = (size_t)-1, hi = 0;
            for (unsigned long seed = 1; seed <= SEEDS; seed++) {
                makeInput(input, n, INPUT_RANDOM, seed * 7919UL);
                BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, n, 1);
                sum += (double)r.stats.compares;
                lo = r.stats.compares < lo ? r.stats.compares : lo;
                hi = r.stats.compares > hi ? r.stats.compares : hi;
            }
            printf("%zu,%s,%.1f,%zu,%zu\n", n, SORT_ALGORITHMS[k].name, sum / SEEDS, lo, hi);
        }
        free(input);
    }
}

/* --- 큰 n에서 비교 1회의 값 ---------------------------------------------- */

/* 배열을 캐시보다 크게 해서 비교 1번당 시간 비교 (오래 걸려서 1번만) */
static void reportBig(void) {
    static const size_t SIZES[] = {1000000, 4000000, 16000000};
    printf("n,algo,millis,compares,nsPerCompare\n");
    for (size_t s = 0; s < sizeof(SIZES) / sizeof(SIZES[0]); s++) {
        size_t n = SIZES[s];
        Record *input = (Record *)malloc(n * sizeof(Record));
        if (input == NULL) {
            return;
        }
        makeInput(input, n, INPUT_RANDOM, SEED);
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, n, 1);
            printf("%zu,%s,%.1f,%zu,%.2f\n", n, r.algo->name, r.millis, r.stats.compares,
                   r.millis * 1e6 / (double)r.stats.compares);
        }
        free(input);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        reportCsv();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--pivot") == 0) {
        reportPivot();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--theory") == 0) {
        reportTheory();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--big") == 0) {
        reportBig();
        return 0;
    }
    reportTable();
    return 0;
}
