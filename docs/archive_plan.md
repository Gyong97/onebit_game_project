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

# 프로젝트 실행 계획 (Phase 2: 시스템 고도화)

## [x] 1단계: 맵 엔진 및 스크롤 로직 수정 (최우선)
- [x] 맵 생성 시 내부 장애물(`#`) 및 엔티티 무작위 스폰 로직 구현
- [x] 5-Buffer 스크롤 알고리즘 적용 (상단 5칸 유지)
- [x] 뷰포트 기반 상대 좌표 렌더링 시스템 구축 (플레이어 실종 버그 해결)
- [x] 하단 이동(Backtracking) 차단 로직 구현

## [x] 2단계: 아이템 및 수집 시스템 구현
- [x] 코인(`$`) 엔티티 정의 및 플레이어 수집 로직 구현
- [x] 인벤토리 시스템(배열 구조) 및 아이템 획득 로직 구현
- [x] 상자(`C`) 오픈 시 무작위 아이템 생성 및 인벤토리 저장 연동

## [x] 3단계: 장비 및 스탯 시스템 고도화
- [x] 장비 슬롯(무기/방어구) 정의 및 장착/해제 로직 구현
- [x] 장비 장착에 따른 공격력(Atk), 방어력(Def) 보정 시스템 구현
- [x] UI에 인벤토리 및 장착 상태 표시 기능 추가

### 3. `plan.md` 리뉴얼 (새로운 로드맵)

# 프로젝트 실행 계획 (Phase 3: RPG 시스템 및 영속성)

## 1단계: 성장 및 스케일링 시스템 구현
- [x] `player_t`에 `level`, `xp` 필드 추가 및 레벨업 로직 구현
- [x] 몬스터 처치 시 XP 획득 로직 연동
- [x] 층수(scroll_count) 기반 몬스터 스탯 스케일링 공식 적용

## 2단계: 아이템/몬스터 사전화(Dictionary) 및 다양화
- [x] `item_db` 모듈 생성: 아이템 정보를 ID 기반 테이블로 분리
- [x] 몬스터 종류 추가 (고블린-기본, 슬라임-고체력, 박쥐-고공격력)
- [x] 몬스터 타입에 따른 무작위 스폰 로직 고도화

## 3단계: 상점 및 경제 시스템
- [x] 맵 생성 로직에 상점(`S`) 타일 생성 규칙 추가
- [x] 코인 소모 아이템 구매 및 인벤토리 연동
- [x] 간단한 상점 UI(텍스트 기반) 구현

## 4단계: 세이브 및 메타 데이터 관리
- [x] `save_manager` 모듈 구현: 파일 입출력 및 데이터 직렬화
- [x] 게임 오버 시 저장/삭제 데이터 분리 처리
- [x] 메인 화면에 최고 기록(Best Depth) 표시 기능

# 프로젝트 실행 계획 (Phase 4: 게임 확장성 부여 및 UI 확장)

## 1단계: 맵 시스템 확장
- [x] 사용자에게 보이는 맵과, 보이지 않은 상태로 엔티티 행동 추가
- [x] 상자와 상점은 열고 나서 상호작용 종료 시 열린 채로 맵에 존재하도록 수정
- [x] 맵에 무작위 엔티티 생성 시

## 2단계: 아이템/몬스터 사전화(Dictionary) 및 확장성 개선
- [x] 구현한 기본 몬스터 고블린, 박쥐 등을 game_spec.md에 정리된 내용에 따라 game_monster.md에 정의
- [x] 아이템을 기본 아이템을 정의하여 game_spec.md에 따라 game_use_item.md와 game_equip_item.md 에 정의
- [x] 플레이어 기본 스펙을 정의하여 game_playable.md에 정의
- [x] 이후 몬스터, 아이템, 플레이어 직업군 추가가 용이하도록 코드 확장성 개선

## 3단계: 상점 기능 구현
- [x] 상점 입장/퇴장 기능 구현
- [x] 상점 구매 기능 구현
- [x] 상점 판매 기능 구현

## 4단계: 시스템 UI 구현
- [x] UI 구현 인터페이스 함수 구현 ( 이후 CLI -> GUI 확장성 고려 )
- [x] game_spec.md의 UI 부분 몬스터 UI 구현
- [x] game_spec.md의 UI 부분 상자 UI 구현
- [x] game_spec.md의 UI 부분 상점 UI 구현
- [x] game_spec.md의 UI 부분 플레이어 UI 구현