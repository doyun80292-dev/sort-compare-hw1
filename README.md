# 정렬 비교 — 병합 · 퀵 · 힙

2026-2 **고급알고리즘**(SIT2001-01) 과제 1. 배운 정렬 둘(**병합 · 퀵**)과 배우지 않은 정렬 하나(**힙**)를
하나의 공통 인터페이스로 묶고, 같은 입력 · 같은 계측기로 비교했다.
[`algorithm-env`](https://github.com/lec-algorithm/algorithm-env) 템플릿에서 시작했다.

- 보고서: [report/REPORT.md](report/REPORT.md) (제출본은 PDF)
- 측정값 원본: `report/*.csv` · 그래프: `report/*.svg`

## 돌려보기

Codespaces(**Code → Codespaces → Create codespace**) 또는 로컬 컨테이너(`docker compose up -d && docker compose exec lab bash`)
안에서:

| 명령 | 하는 일 |
| --- | --- |
| `make test` | 유닛 테스트 55개 (`55 checks, 0 failures`가 나와야 한다) |
| `make run` | 입력 모양별 · n 배가 비교 표 |
| `make charts` | 모든 실험을 다시 재서 `report/`의 CSV · SVG를 새로 쓴다 (약 1분) |
| `make asan` | AddressSanitizer · UBSan을 붙여 테스트 |
| `make clean` | 빌드 산출물 정리 |

실험별 CSV를 직접 볼 수도 있다: `./src/main.out --csv` · `--pivot` · `--theory` · `--big`.
입력은 수업 topic-04의 minstd 생성기로 만들므로 **비교 · 이동 횟수는 어느 기계에서나 보고서와 같다.** 시간(ms)만 기계마다 다르다.

## 구조

```plaintext
src/
  sort.h · sortctx.h · sort.c    공통 인터페이스(SortAlgorithm) · 구현 전용 도구 · 구현 표
  mergeSort.c                    병합 정렬 (수업 topic-03을 일반화)
  quickSort.c                    퀵 정렬 (수업 topic-04 파티션 + 무작위 피벗)
  heapSort.c                     힙 정렬 (새로 학습)
  bench.h · bench.c              입력 생성 · 시간 · 안정성 측정
  main.c                         비교 표와 실험별 CSV
tests/test_sort.c                유닛 테스트 (표준 C만)
tools/plot.py · svgchart.py      CSV → SVG 그래프 (표준 모듈만)
report/                          보고서 · 그래프 · 측정값
```

외부 라이브러리를 쓰지 않는다. 실행 파일은 `*.out`으로 만들어 `.gitignore`가 걸러낸다.
`tools/svgchart.py`와 인터페이스 설계는 과제 샘플(`lec-algorithm/hw1-sample-2026`)의 것을 가져와 고쳐 썼다.
