# 플레이어 직업군 사전 (Playable Class Dictionary)

본 문서는 OneBit Adventure에 등장하는 **플레이어 직업군**을 정의합니다.
게임 시작 시 직업군에 따라 초기 스탯이 결정되며, 이후 레벨업/장비로 스탯이 변화합니다.
코드에서 새 직업군을 추가할 때는 `playable_db.c`의 `g_playable_table`에 행을 추가합니다.

---

## 등록 직업군 목록

### Warrior (전사)
| 항목 | 값 |
|------|-----|
| **타입 ID** | `PLAYABLE_WARRIOR` (0) |
| **초기 HP** | 100 |
| **최대 HP** | 100 |
| **초기 ATK** | 10 |
| **초기 DEF** | 0 |
| **특징** | 균형잡힌 기본 직업군. 방어구 장착으로 방어력 확보 가능. |

---

## 직업군 규칙

- 게임 시작 시 `player_init_typed(p, class_type)`로 스탯 초기화.
- `player_init()`은 기본값으로 `PLAYABLE_WARRIOR`를 사용.
- ATK/DEF는 장비 장착/해제 시 보너스가 가산/차감됨.
- HP는 소모형 아이템 사용, 몬스터 피격, 레벨업 시 변화.

---

## 새 직업군 추가 방법

1. `include/playable_db.h`의 `playable_type_t`에 `PLAYABLE_NEWCLASS` 값 추가 (`PLAYABLE_TYPE_COUNT` 앞에).
2. `NEWCLASS_INIT_HP`, `NEWCLASS_INIT_MAXHP`, `NEWCLASS_INIT_ATK`, `NEWCLASS_INIT_DEF` 상수 정의.
3. `PLAYABLE_TYPE_COUNT`를 1 증가.
4. `src/logic/playable_db.c`의 `g_playable_table`에 1행 추가.
5. 본 문서에 항목 추가.
