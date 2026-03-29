# 소모형 아이템 사전 (Consumable Item Dictionary)

본 문서는 OneBit Adventure에 등장하는 **소모형 아이템**(`ITEM_POTION`)을 정의합니다.
소모 시 즉시 효과가 발동되고 인벤토리에서 사라집니다.
코드에서 새 아이템을 추가할 때는 `item_db.c`의 `g_item_table`에 행을 추가합니다.

---

## 등록 아이템 목록

### Health Pot (체력 포션)
| 항목 | 값 |
|------|----|
| **타입** | `ITEM_POTION` |
| **DB ID** | 6 |
| **효과** | HP +20 회복 (최대 HP 초과 불가) |
| **획득 방법** | 상자 오픈, 상점 구매 |

### Elixir (엘릭서)
| 항목 | 값 |
|------|----|
| **타입** | `ITEM_POTION` |
| **DB ID** | 7 |
| **효과** | HP +40 회복 (최대 HP 초과 불가) |
| **획득 방법** | 상자 오픈, 상점 구매 |

---

## 소모형 아이템 규칙

- 인벤토리에 보관하며, 사용 시 즉시 효과 발동 후 제거됨.
- HP 회복은 `max_hp`를 초과하지 않음.
- 상점에서 코인(`SHOP_ITEM_COST = 5`)을 소모하여 무작위 아이템(소모/장비 통합) 구입 가능.

---

## 새 소모형 아이템 추가 방법

1. `include/item.h`에 필요하면 새 필드 또는 상수 추가 (`hp_restore` 계열).
2. `src/logic/item_db.c`의 `g_item_table`에 `{ ITEM_POTION, 0, 0, 회복량, "이름" }` 형식으로 1행 추가.
3. `include/item_db.h`의 `ITEM_DB_COUNT`를 1 증가.
4. 본 문서에 항목 추가.
