# Changelog

형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.1.0/)를 따른다.

## [1.0] - 2026-09-28

### Added

- 공통 인터페이스 `SortAlgorithm`과 구현 표 — 병합 · 퀵 · 힙 정렬
- 측정 도구: 비교 · 이동 · 추가 메모리 · 재귀 깊이 · `(key, tag)` 안정성 실측, minstd 입력 생성
- 실험: 입력 모양별(n = 100,000), n 배가(1K ~ 1M), 퀵 피벗(`--pivot`), 이론식 대조(`--theory`), 큰 n(`--big`)
- 유닛 테스트 55개, `make asan`
- 그래프 도구(`tools/`)와 보고서(`report/REPORT.md`)

### Removed

- 템플릿의 예제(버블 정렬)와 Python 구현
