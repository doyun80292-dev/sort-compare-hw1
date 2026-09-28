# CLAUDE.md

고급알고리즘 과제 1 (정렬 비교: 병합 · 퀵 · 힙) 저장소. AI 도구로 이 저장소를 다룰 때 참고할 규칙.

## 구조

```plaintext
src/    sort.h · sortctx.h · sort.c · mergeSort.c · quickSort.c · heapSort.c · bench.* · main.c
tests/  test_sort.c
tools/  plot.py · svgchart.py
report/ REPORT.md · 그래프(svg) · 측정값(csv)
```

## 규칙

- 외부 라이브러리 안 씀. C는 표준 라이브러리만, Python은 표준 모듈만.
- 실행 파일은 `*.out`으로 만든다 (`.gitignore`가 걸러냄).
- `-Wall -Wextra` 경고 없이 빌드돼야 한다.
- 커밋 전에 `make test` 통과 확인.
- 정렬을 추가하면 `src/sort.c`의 `SORT_ALGORITHMS`와 `Makefile`의 `SORT_SRC`에 같이 추가.
- 보고서 숫자는 `make charts`로 만든 `report/*.csv`와 맞아야 한다.

## 실행

```sh
docker compose up -d
docker compose exec lab bash
make test
make run
```
