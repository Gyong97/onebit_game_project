# 시스템 아키텍처 및 확장성 설계 (Architecture)

본 프로젝트는 추후 SDL2나 Raylib 같은 GUI 라이브러리로 쉽게 전환할 수 있도록 MVC(Model-View-Controller) 패턴과 유사한 계층 분리 설계를 채택합니다.

## 1. 디렉터리 및 계층 구조

* **`src/logic/` (Core Logic / Model)**
  * 게임의 상태, 체력 계산, 이동 규칙, 턴 관리 등 순수 게임 로직만 존재합니다.
  * **제약사항:** `stdio.h`를 통한 화면 출력 코드가 절대 포함되어서는 안 됩니다.

* **`include/` (Abstract Interfaces)**
  * `renderer.h`: 화면을 지우고, 개체를 그리고, 화면을 갱신하는 추상화된 함수 원형만 선언합니다.
  * `input.h`: 사용자 입력을 `CMD_UP`, `CMD_DOWN` 같은 추상화된 열거형(Enum)으로 변환하는 함수 원형을 선언합니다.

* **`src/renderer/` (Platform Implementation / View)**
  * `include/renderer.h`에 선언된 함수들을 실제로 구현합니다.
  * 현재는 `renderer_tui.c`를 통해 ASCII 문자로 콘솔에 출력하지만, 추후 이 폴더에 `renderer_sdl.c`를 추가하는 것만으로 GUI 전환이 가능해야 합니다.

## 2. 인터페이스 교환 원칙
게임 로직은 개체의 좌표(x, y)와 타입(플레이어, 몬스터, 벽) 데이터만 렌더러에 전달(`render_draw_entity(x, y, TYPE_PLAYER)`)하며, 렌더러는 전달받은 데이터를 화면에 어떻게 그릴지만 결정합니다.

## 3. 수직 스크롤 맵 설계

### 3.1 데이터 구조 (`map_t`)
* **뷰포트 버퍼:** `tile_type_t rows[VIEWPORT_H][MAP_WIDTH]`
  * `rows[0]` = 화면 최상단 행 (플레이어가 전진하는 방향)
  * `rows[VIEWPORT_H-1]` = 화면 최하단 행 (사라지는 방향)
  * 가로 폭 `MAP_WIDTH = 10` 고정; `x=0`과 `x=9`는 항상 벽(`TILE_WALL`)
* **스크롤 카운터:** `long scroll_count` — 누적 스크롤 횟수(탐험 거리 표시용)

### 3.2 스크롤 발동 조건 (`map_scroll`)
플레이어가 뷰포트 최상단(`y=0`)에서 위쪽(`ACTION_MOVE_UP`)으로 이동을 시도할 때:
1. `rows[VIEWPORT_H-1]` (최하단 행) 소멸 (덮어쓰기)
2. `rows[r] ← rows[r-1]` (r = VIEWPORT_H-1 → 1) — 기존 행을 아래로 이동
3. `rows[0]` ← 새로 절차적 생성(양 끝 벽, 내부 바닥 + 랜덤 오브젝트)
4. `scroll_count++`
5. 플레이어는 `y=0` 유지 (실제로는 새 행으로 진입한 것)

### 3.3 이동 규칙 요약
| 상황 | 결과 |
|---|---|
| `y=0`에서 위(`ACTION_MOVE_UP`) | `map_scroll()` 호출, 플레이어 `y=0` 유지 |
| `y=VIEWPORT_H-1`에서 아래(`ACTION_MOVE_DOWN`) | 차단(후진 불가, 뷰포트 경계) |
| 이동 대상 타일 = `TILE_WALL` | 차단 |
| 이동 대상 타일 = `TILE_FLOOR` | 이동 성공 |

### 3.4 렌더러 연동
* `render_frame_t.tiles[VIEWPORT_H][MAP_WIDTH]`는 항상 현재 뷰포트 스냅샷
* `render_frame_t.scroll_count`를 HUD에 표시하여 탐험 거리 제공
* 로직 레이어는 매 턴 `map_t`에서 `render_frame_t`로 타일을 복사하여 전달