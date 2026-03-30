# 프로젝트 이행 계획서 (Roadmap)

클로드 코드는 본 문서의 체크리스트를 기반으로 작업을 수행하며, 각 세부 항목이 끝날 때마다 상태를 업데이트해야 합니다.

# 프로젝트 실행 계획 (Phase 5: 몬스터 AI 업그레이드)

## 1단계: 데이터 모델 확장
- [ ] `monster_def_t`에 `perception_range`(int), `can_fly`(int) 필드 추가
- [ ] `monster_t`에 `state`(MONSTER_STATE_IDLE / MONSTER_STATE_CHASING) 필드 추가
- [ ] `monster.h`에 인지범위 상수 정의 (GOBLIN=6, SLIME=4, BAT=8)
- [ ] `monster_db.c` 테이블에 perception_range, can_fly 값 반영

## 2단계: AI 로직 구현
- [ ] BFS 경로탐색 함수 구현 — 지상 몬스터용 (벽 인식)
- [ ] BFS 경로탐색 함수 구현 — 비행 몬스터용 (벽 통과)
- [ ] 인지범위 판단 로직 구현 (맨해튼 거리 계산, 상태 전환)
- [ ] 배회(IDLE) 랜덤 이동 로직 구현
- [ ] `turn_manager.c`의 `apply_monster_move()` 교체 — 새 AI 적용

## 3단계: 테스트
- [ ] 인지범위 밖 몬스터는 플레이어에게 접근하지 않음을 검증
- [ ] 인지범위 안 몬스터는 BFS 최단경로로 접근함을 검증
- [ ] 지상 몬스터가 벽에 막혀 우회 경로를 선택함을 검증
- [ ] 비행 몬스터가 벽을 통과하여 이동함을 검증
- [ ] IDLE 상태 몬스터가 랜덤하게 이동함을 검증 (이동했음만 확인)
