/* test_sort.c - 유닛 테스트 (실행: make test)
 * SORT_ALGORITHMS에 있는 정렬 전부에 같은 테스트를 돌림 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int checks = 0;
static int failures = 0;

static void report(const char *who, const char *name, int ok) {
    checks++;
    if (ok) {
        printf("ok    %-10s %s\n", who, name);
        return;
    }
    failures++;
    printf("FAIL  %-10s %s\n", who, name);
}

/* --- 기본 케이스 --- */

static void expectSorted(const SortAlgorithm *algo, const char *name,
                         const int input[], const int want[], size_t n) {
    int a[32];
    SortStats stats;
    memcpy(a, input, n * sizeof(int));
    algo->sort(a, n, sizeof(a[0]), sortCompareInt, &stats);
    report(algo->name, name, n == 0 || memcmp(a, want, n * sizeof(int)) == 0);
}

static void basicCases(const SortAlgorithm *algo) {
    {
        const int a[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
        const int w[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        expectSorted(algo, "섞인 배열", a, w, 10);
    }
    {
        const int a[] = {1, 2, 3, 4, 5};
        const int w[] = {1, 2, 3, 4, 5};
        expectSorted(algo, "이미 정렬된 배열", a, w, 5);
    }
    {
        const int a[] = {5, 4, 3, 2, 1};
        const int w[] = {1, 2, 3, 4, 5};
        expectSorted(algo, "역순 배열", a, w, 5);
    }
    {
        const int a[] = {3, 1, 3, 1, 2};
        const int w[] = {1, 1, 2, 3, 3};
        expectSorted(algo, "중복이 있는 배열", a, w, 5);
    }
    {
        const int a[] = {2, 2, 2, 2};
        const int w[] = {2, 2, 2, 2};
        expectSorted(algo, "모두 같은 값", a, w, 4);
    }
    {
        const int a[] = {-3, 2147483647, 0, -2147483647 - 1, 7};
        const int w[] = {-2147483647 - 1, -3, 0, 7, 2147483647};
        expectSorted(algo, "INT_MIN·INT_MAX가 섞인 배열", a, w, 5);
    }
    {
        const int a[] = {42};
        const int w[] = {42};
        expectSorted(algo, "원소 하나", a, w, 1);
    }
    {
        const int a[] = {2, 1};
        const int w[] = {1, 2};
        expectSorted(algo, "원소 둘", a, w, 2);
    }
    {
        const int a[1] = {0};
        const int w[1] = {0};
        expectSorted(algo, "빈 배열", a, w, 0);
    }
}

/* --- qsort 대조 + 크기 훑기 --- */

/* n = 0~300 전부 qsort랑 비교. 안정 정렬이면 tag 순서도 확인 */
static void expectManySizes(const SortAlgorithm *algo) {
    enum { MAX_N = 300 };
    Record a[MAX_N];
    Record want[MAX_N];
    SortStats stats;
    Minstd g;
    int ok = 1;

    minstdSeed(&g, 20260902UL);
    for (size_t n = 0; n <= MAX_N && ok; n++) {
        for (size_t i = 0; i < n; i++) {
            a[i].key = (int)(minstdNext(&g) % 20); /* 중복이 많은 입력 */
            a[i].tag = (int)i;
            want[i] = a[i];
        }
        qsort(want, n, sizeof(want[0]), recordCompare);
        algo->sort(a, n, sizeof(a[0]), recordCompare, &stats);
        for (size_t i = 0; i < n; i++) {
            if (a[i].key != want[i].key) {
                ok = 0;
            }
        }
        if (algo->stable && !recordsStable(a, n)) {
            ok = 0;
        }
        if (!ok) {
            printf("      n = %zu에서 어긋났다\n", n);
        }
    }
    report(algo->name, "n = 0..300 전부 qsort와 같은 key 순서", ok);
}

/* 정렬 후에 원소가 없어지거나 중복되지 않았는지 (tag별로 key 확인) */
static void expectPermutation(const SortAlgorithm *algo) {
    enum { N = 2000 };
    Record *a = malloc(N * sizeof(Record));
    Record *orig = malloc(N * sizeof(Record));
    int *seen = calloc(N, sizeof(int));
    SortStats stats;
    int ok = a != NULL && orig != NULL && seen != NULL;

    if (ok) {
        makeInput(a, N, INPUT_FEW_UNIQUE, 7UL);
        memcpy(orig, a, N * sizeof(Record));
        algo->sort(a, N, sizeof(Record), recordCompare, &stats);
        for (int i = 0; i < N && ok; i++) {
            int t = a[i].tag;
            if (t < 0 || t >= N || seen[t] || orig[t].key != a[i].key) {
                ok = 0;
            } else {
                seen[t] = 1;
            }
        }
    }
    report(algo->name, "원소를 잃거나 복제하지 않는다 (순열)", ok);
    free(a);
    free(orig);
    free(seen);
}

/* --- 안정성 --- */

/* 실제 안정성 결과 == 표에 적은 stable 값 */
static void expectStability(const SortAlgorithm *algo) {
    enum { N = 1000 };
    Record a[N];
    SortStats stats;
    makeInput(a, N, INPUT_FEW_UNIQUE, 20260903UL);
    algo->sort(a, N, sizeof(a[0]), recordCompare, &stats);
    report(algo->name, "실측 안정성 = 표의 stable 값",
           recordsSorted(a, N) && recordsStable(a, N) == algo->stable);
}

/* --- 측정값 --- */

static void expectStats(const SortAlgorithm *algo) {
    int a[] = {5, 1, 4, 2, 3};
    SortStats stats;
    algo->sort(a, 5, sizeof(a[0]), sortCompareInt, &stats);
    report(algo->name, "측정값이 채워진다",
           stats.compares > 0 && stats.moves > 0 && stats.extraBytes >= sizeof(a[0]) &&
           stats.maxDepth >= 1);
}

static void expectBenchRun(const SortAlgorithm *algo) {
    enum { N = 500 };
    Record input[N];
    makeInput(input, N, INPUT_RANDOM, 20260904UL);
    BenchResult r = benchRun(algo, input, N, 2);
    report(algo->name, "benchRun이 결과를 채운다",
           r.sorted && r.millis >= 0.0 && r.stats.compares > 0 && r.n == N &&
           r.algo == algo);
}

/* --- 알고리즘별 성질 --- */

/* 병합은 n+1칸, 힙은 1칸 */
static void expectMemoryClaims(void) {
    enum { N = 1000 };
    Record a[N];
    Record b[N];
    SortStats sm, sh;
    makeInput(a, N, INPUT_RANDOM, 1UL);
    memcpy(b, a, sizeof(a));
    mergeSort(a, N, sizeof(Record), recordCompare, &sm);
    heapSort(b, N, sizeof(Record), recordCompare, &sh);
    report("mergeSort", "추가 메모리 = (n + 1)칸", sm.extraBytes == (N + 1) * sizeof(Record));
    report("heapSort", "추가 메모리 = 1칸, 재귀 없음",
           sh.extraBytes == sizeof(Record) && sh.maxDepth == 1);
}

/* 첫 원소 피벗 + 정렬된 입력 -> 비교 n(n-1)/2, 깊이 n */
static void expectQuickPivotModes(void) {
    enum { N = 500 };
    Record a[N];
    SortStats stats;
    const QuickPivot saved = quickSortPivot;

    quickSortPivot = PIVOT_FIRST;
    makeInput(a, N, INPUT_RANDOM, 3UL);
    quickSort(a, N, sizeof(Record), recordCompare, &stats);
    report("quickSort", "첫 원소 피벗으로도 정렬된다", recordsSorted(a, N));

    makeInput(a, N, INPUT_SORTED, 3UL);
    quickSort(a, N, sizeof(Record), recordCompare, &stats);
    report("quickSort", "정렬된 입력 + 첫 원소 피벗 = 비교 n(n-1)/2",
           stats.compares == (size_t)N * (N - 1) / 2);
    report("quickSort", "그때 재귀 깊이 = n", stats.maxDepth == N);

    quickSortPivot = PIVOT_RANDOM;
    makeInput(a, N, INPUT_SORTED, 3UL);
    quickSort(a, N, sizeof(Record), recordCompare, &stats);
    report("quickSort", "무작위 피벗이면 같은 입력에서 비교가 1/5 미만",
           recordsSorted(a, N) && stats.compares < (size_t)N * (N - 1) / 2 / 5);
    quickSortPivot = saved;
}

/* 힙 정렬 비교 횟수 <= 2 n log2 n */
static void expectHeapBound(void) {
    enum { N = 4096 }; /* log2 N = 12 */
    Record *a = malloc(N * sizeof(Record));
    SortStats stats;
    int ok = 0;
    if (a != NULL) {
        makeInput(a, N, INPUT_RANDOM, 5UL);
        heapSort(a, N, sizeof(Record), recordCompare, &stats);
        ok = recordsSorted(a, N) && stats.compares <= 2UL * N * 12;
    }
    report("heapSort", "비교 횟수 <= 2 n log2 n (n = 4096)", ok);
    free(a);
}

/* --- bench 함수들 --- */

static void expectBenchTools(void) {
    enum { N = 40 };
    Record a[N];
    Record b[N];

    makeInput(a, N, INPUT_SORTED, 1UL);
    report("bench", "makeInput(정렬됨)은 정렬돼 있다", recordsSorted(a, N));
    makeInput(a, N, INPUT_REVERSED, 1UL);
    report("bench", "makeInput(역순)은 첫 key가 마지막보다 크다",
           !recordsSorted(a, N) && a[0].key > a[N - 1].key);
    makeInput(a, N, INPUT_FEW_UNIQUE, 1UL);
    int ok = 1;
    for (int i = 0; i < N; i++) {
        ok = ok && a[i].key >= 0 && a[i].key < 8 && a[i].tag == i;
    }
    report("bench", "makeInput(중복많음)의 key는 0..7, tag는 입력 순서", ok);
    makeInput(a, N, INPUT_RANDOM, 9UL);
    makeInput(b, N, INPUT_RANDOM, 9UL);
    report("bench", "같은 씨앗이면 같은 입력", memcmp(a, b, sizeof(a)) == 0);

    Record good[4] = {{1, 0}, {1, 1}, {2, 2}, {2, 3}};
    Record flipped[4] = {{1, 1}, {1, 0}, {2, 2}, {2, 3}};
    report("bench", "안정성 판정기가 뒤집힌 tag를 잡는다",
           recordsStable(good, 4) && !recordsStable(flipped, 4));

    Minstd g;
    minstdSeed(&g, 1UL);
    unsigned long x1 = minstdNext(&g), x2 = minstdNext(&g), x3 = minstdNext(&g);
    /* 수업 topic-04 01_random README 출력이랑 같은지 */
    report("bench", "minstd(1) = 16807 282475249 1622650073",
           x1 == 16807UL && x2 == 282475249UL && x3 == 1622650073UL);
}

int main(void) {
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        basicCases(algo);
        expectManySizes(algo);
        expectPermutation(algo);
        expectStability(algo);
        expectStats(algo);
        expectBenchRun(algo);
        printf("\n");
    }
    expectMemoryClaims();
    expectQuickPivotModes();
    expectHeapBound();
    expectBenchTools();

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
