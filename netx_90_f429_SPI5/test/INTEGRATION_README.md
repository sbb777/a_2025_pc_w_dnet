# main.c VAT 테스트 통합 - Quick Start

**최소 변경으로 VAT DeviceNet 테스트 통합하기**

---

## 🚀 30초 통합

### 1. 파일 백업
```bash
cp Core/Src/main.c Core/Src/main.c.backup
```

### 2. 파일 복사
```bash
# VAT 테스트 파일 복사
cp test/vat_devicenet_test.h Hil_DemoApp/Includes/
cp test/vat_devicenet_test.c Hil_DemoApp/Sources/

# 수정된 main.c로 교체
cp test/main_vat_test.c Core/Src/main.c
```

### 3. 빌드 및 실행
```
Project → Build All
Run → Debug
```

**완료!** VAT 테스트가 자동 실행됩니다.

---

## 📋 변경 사항

### 코드 추가 (4곳)

| 위치 | 추가 내용 | 줄 수 |
|------|-----------|------|
| Include | `#include "vat_devicenet_test.h"` | 1 |
| 전역변수 | `g_tVatContext`, `g_VatTestMode`, `g_bEnableVatTest` | 15 |
| 함수 | `VAT_InitializeTest()`, `VAT_RunTest()` | 120 |
| main() | if-else 분기 추가 | 15 |

**총 추가**: ~150 줄
**기존 코드 유지**: 99%

---

## 🎮 테스트 모드 선택

### 디버거에서 변경 (추천)

Live Expression 창에서:
```c
g_VatTestMode = 1   // Basic Pressure Control (기본)
g_VatTestMode = 2   // Full Calibration
g_VatTestMode = 3   // Continuous Monitoring
```

### 코드에서 변경

`main.c` Line 96:
```c
volatile uint8_t g_VatTestMode = 2;  // 1→2로 변경
```

---

## 📊 3가지 테스트 시나리오

### Mode 1: Basic Pressure Control ⭐ 기본
- 압력 설정값 500
- 10회 통신
- 실행 시간: ~1초

### Mode 2: Full Calibration
- 자동 학습 모드
- 보정 기능 테스트
- 5회 통신

### Mode 3: Continuous Monitoring
- 100회 연속 통신
- 10초간 모니터링
- 실시간 상태 출력

---

## 🔍 Live Expression 모니터링

디버거 Live Expression 창에 추가:

```c
// 기본 정보
g_VatTestMode
g_tVatContext.stats.total_read_count
g_tVatContext.stats.total_write_count

// 센서 데이터
g_tVatContext.input_asm100.pressure
g_tVatContext.input_asm100.position
g_tVatContext.input_asm100.device_status

// 제어 데이터
g_tVatContext.output_asm8.control_setpoint
g_tVatContext.output_asm8.control_mode
```

---

## 🔄 기존 App으로 복귀

### 임시 복귀
디버거에서:
```c
g_bEnableVatTest = false
```
리셋 후 재실행

### 영구 복귀
`main.c` Line 98:
```c
volatile bool g_bEnableVatTest = false;  // true→false
```
재빌드

또는 원본 복원:
```bash
cp Core/Src/main.c.backup Core/Src/main.c
```

---

## ⚠️ 트러블슈팅

### 컴파일 에러: "vat_devicenet_test.h not found"
```bash
# 파일 위치 확인
ls Hil_DemoApp/Includes/vat_devicenet_test.h
ls Hil_DemoApp/Sources/vat_devicenet_test.c

# 없으면 복사
cp test/vat_devicenet_test.* Hil_DemoApp/Includes/
```

### 링크 에러: "undefined reference to VAT_Test_*"
1. Project Explorer에서 `vat_devicenet_test.c` 찾기
2. 회색이면 → 우클릭 → Exclude from Build 체크 해제
3. 재빌드

### 통신 타임아웃
`VAT_InitializeTest()` 함수에서:
```c
tConfig.node_address = 10;     // VAT 장치 주소 확인
tConfig.baud_rate = 250000;    // 또는 125000, 500000
```

---

## 📖 상세 문서

- **통합 가이드**: [20251027_Main_Integration_Guide.md](./20251027_Main_Integration_Guide.md)
- **VAT 테스트 가이드**: [20251027_VAT_DeviceNet_Test_Guide.md](./20251027_VAT_DeviceNet_Test_Guide.md)
- **API 레퍼런스**: [README.md](./README.md)

---

## ✅ 체크리스트

### 통합 전
- [ ] 원본 main.c 백업
- [ ] test/ 폴더 파일 확인

### 통합 후
- [ ] 컴파일 성공
- [ ] 링크 성공
- [ ] 플래그 변수 확인

### 실행 전
- [ ] netX90 전원 ON
- [ ] SPI 연결 확인
- [ ] VAT DeviceNet 연결

### 실행 후
- [ ] 초기화 메시지 출력
- [ ] 테스트 실행 확인
- [ ] 통계 출력 확인

---

## 💡 팁

### 성능 최적화
```c
// VAT_InitializeTest() 함수에서
tConfig.enable_logging = false;  // 로깅 비활성화 → 30% 빠름
tConfig.epr_ms = 50;             // 통신 주기 단축 → 2배 빠름
```

### 커스텀 테스트 추가
`VAT_RunTest()` 함수에 case 추가:
```c
case 4:  /* My Custom Test */
    // 커스텀 로직
    break;
```

### 빌드 시점 모드 선택
```c
#define VAT_TEST_MODE 2  // 컴파일 시점 고정

// main.c에서
volatile uint8_t g_VatTestMode = VAT_TEST_MODE;
```

---

**작성일**: 2025-10-27
**버전**: 1.0
