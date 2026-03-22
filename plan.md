# 프로젝트 이행 계획서 (Roadmap)

클로드 코드는 본 문서의 체크리스트를 기반으로 작업을 수행하며, 각 세부 항목이 끝날 때마다 상태를 업데이트해야 합니다.

## [x] 1단계: 프로젝트 기초 골격 및 추상화 인터페이스
- [x] 디렉터리 구조 생성 (`src/logic`, `src/renderer`, `include`, `tests`, `docs`)
- [x] `include/renderer.h` 및 `include/input.h` 추상 인터페이스 헤더 정의
- [x] TUI 기반 기초 렌더러 구현 (`src/renderer/renderer_tui.c`)
- [x] 소스와 헤더를 분리하여 빌드할 수 있는 `Makefile` 작성

## [x] 2단계: 핵심 엔진 및 플레이어 로직 (수직 스크롤 맵)
- [x] `include/map.h` 정의: 10×10 뷰포트 버퍼(`tile_type_t rows[VIEWPORT_H][MAP_WIDTH]`) 및 `scroll_count` 포함 (TDD)
- [x] `include/player.h` 정의: `player_t` 구조체 및 초기 스탯(HP 100 / MaxHP 100 / Atk 10) (TDD)
- [x] `tests/test_map.c` 실패 테스트 먼저 작성: 맵 초기화, 스크롤, 타일 접근 검증
- [x] `tests/test_player.c` 실패 테스트 먼저 작성: 플레이어 초기화, WASD 이동, 충돌 검증
- [x] `src/logic/map.c` 구현: 뷰포트 스크롤(위로 이동 시 하단 1줄 소멸, 상단에 새 줄 생성), 절차적 행 생성
- [x] `src/logic/player.c` 구현: WASD 이동, 벽 충돌, 뷰포트 상단 도달 시 맵 스크롤 트리거, 하단 후진 차단

## [x] 3단계: 몬스터 AI 및 턴제 시스템
- [x] `include/monster.h` 정의: `monster_t`(HP 20/Atk 5/alive 플래그) 및 API (TDD)
- [x] `include/turn_manager.h` 정의: `game_state_t`(map+player+monsters 통합) 및 턴 제어 API
- [x] `tests/test_monster.c` 실패 테스트 먼저 작성: 초기화, 추적 AI, 벽 충돌 검증
- [x] `tests/test_turn.c` 실패 테스트 먼저 작성: 플레이어 행동 후 몬스터 거리 단축 검증, 스크롤 연동
- [x] `src/logic/player.c` 수정: 이동/스크롤 시 맵 타일(TILE_PLAYER) 동기화
- [x] `src/logic/monster.c` 구현: 추적 AI (|dx|≥|dy| 시 가로 우선, 차단 시 세로 대체)
- [x] `src/logic/turn_manager.c` 구현: 플레이어 행동→몬스터 행동 게임 루프, 스크롤 감지 후 몬스터 하강/소멸, 신규 행에 MONSTER_SPAWN_PCT 확률 스폰

## [ ] 4단계: 전투 및 아이템 시스템
- [ ] 플레이어와 몬스터 좌표 중첩 시 전투(체력 차감)가 발생하는 로직 (TDD)
- [ ] 맵 상에 상자(Chest) 배치 및 충돌 시 아이템 획득 처리
- [ ] 아이템 장착에 따른 플레이어 공격력/최대 체력 증가 스탯 반영