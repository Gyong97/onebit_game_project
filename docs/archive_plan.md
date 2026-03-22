# 프로젝트 시작
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

## [x] 4단계: 전투 및 아이템 시스템
- [x] `include/player.h` 수정: PLAYER_MOVE_ATTACK(2), PLAYER_MOVE_CHEST(3) 반환 코드 상수 추가
- [x] `include/monster.h` 수정: monster_step 시그니처 `const player_t*` → `player_t*` (데미지 적용 위해)
- [x] `include/turn_manager.h` 수정: TURN_GAME_OVER(2), CHEST_* 상수, turn_manager_open_chest 선언 추가
- [x] `tests/test_combat.c` 실패 테스트 먼저 작성: 플레이어→몬스터 공격/사망, 몬스터→플레이어 공격, 게임오버, 상자 상호작용
- [x] `src/logic/player.c` 수정: TILE_MONSTER → PLAYER_MOVE_ATTACK, TILE_CHEST → PLAYER_MOVE_CHEST 반환
- [x] `src/logic/monster.c` 수정: try_step() TILE_PLAYER 시 player.hp -= monster.atk 실제 데미지 적용
- [x] `src/logic/turn_manager.c` 수정: PLAYER_MOVE_ATTACK 처리(몬스터 탐색·데미지·사망), PLAYER_MOVE_CHEST 처리(보상·타일 제거), monster_acts 후 player.hp ≤ 0 → TURN_GAME_OVER, spawn_row에 CHEST_SPAWN_PCT 확률 상자 스폰 추가