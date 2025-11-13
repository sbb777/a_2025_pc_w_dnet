# CYCON.net EDS 파일 미매칭 분석 보고서

**작성일시**: 2025-10-29
**문제**: DeviceNet 스캔 시 VAT EDS가 아닌 DNS_V5_SIMPLE_CONFIG_DEMO.EDS로 인식됨
**도구**: CYCON.net DeviceNet Scanner

---

## 📋 문제 요약

CYCON.net에서 DeviceNet 스캔 시:
- **Device Name**: "VAT_V5_SIMPLE_CONFIG_DEMO" (실제 디바이스)
- **DTM Device**: "DNS_V5_SIMPLE_CONFIG_DEMO" (매칭된 EDS)
- **문제**: VAT EDS 파일로 인식되지 않고 Hilscher 기본 EDS로 인식됨

---

## 🔍 CYCON 스캔 결과 상세

### Hardware Device Information (실제 디바이스)
```
Device: VAT_V5_SIMPLE_CONFIG_DEMO
Station Address: 1
Vendor: 283 (0x0000011b) Hilscher GmbH
Device Type ID: 34 (0x00000022)
Sub Device Type: 12 (0x0000000c)
```

### DTM Information (매칭된 EDS)
```
DTM Device: DNS_V5_SIMPLE_CONFIG_DEMO (DNS_V5.SIMPLE_CONFIG.DEMO.EDS)
DTM ProgId: HilscherDnGenSlave, DNGenSlaveDTM, 1
Vendor: Hilscher GmbH
Device Type ID: 34 (0x00000022)
Version: 5.1
Date: 2021-01-01
```

---

## 🔑 근본 원인 분석

### DTM(Device Type Manager) 매칭 메커니즘

CYCON.net과 같은 DeviceNet 설정 도구는 다음 순서로 EDS 파일을 매칭합니다:

**1단계: Vendor Code 매칭**
```
스캔된 디바이스의 Vendor ID = 283 (Hilscher)
  ↓
Vendor ID 283을 가진 모든 EDS 파일 검색
```

**2단계: Product Code 매칭**
```
스캔된 디바이스의 Product Code = 34
  ↓
Vendor 283 + Product Code 34를 가진 EDS 파일 검색
  ↓
DNS_V5_SIMPLE_CONFIG_DEMO.EDS 발견!
```

**3단계: Version 확인 (선택)**
```
Major Revision: 5
Minor Revision: 2 (디바이스) vs 1 (EDS)
  ↓
버전 차이는 경고만 발생, 매칭은 유지
```

**❌ Product Name은 매칭에 사용되지 않음!**

---

## 📊 CIP Identity Object 비교

### 현재 코드 설정

**파일**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c:49-56`

```c
#define VENDOR_ID                            CIP_VENDORID_HILSCHER  // 283
#define DEVICE_TYPE_COMMUNICATION_ADAPTER    0x0C                  // 12
#define PRODUCT_CODE                         34
#define MAJOR_REVISION                       5
#define MINOR_REVISION                       2
char    abProductName[]                      = "VAT_V5_SIMPLE_CONFIG_DEMO";
```

**실제 설정값**:
| 항목 | 값 | 16진수 |
|------|-----|--------|
| Vendor ID | 283 | 0x0000011B |
| Device Type | 12 | 0x0000000C |
| Product Code | **34** | **0x00000022** |
| Major Revision | 5 | - |
| Minor Revision | 2 | - |
| Product Name | "VAT_V5_SIMPLE_CONFIG_DEMO" | - |

---

### DNS_V5_SIMPLE_CONFIG_DEMO.EDS

**파일**: `DNS_V5_SIMPLE_CONFIG_DEMO.EDS:27-36`

```ini
[Device]
        VendCode = 283;                              # Hilscher GmbH
        VendName = "Hilscher GmbH";
        ProdType = 12;                               # Communications Adapter
        ProdTypeStr = "Communications Adapter";
        ProdCode = 34;                               # ← 매칭!
        MajRev = 5;
        MinRev = 1;
        ProdName = "DNS_V5_SIMPLE_CONFIG_DEMO";      # ← 이름은 다름
        Catalog = "0";
```

**매칭 결과**: ✅ **Vendor 283 + Product Code 34 매칭 성공**

---

### 실제 VAT EDS 파일 (476297.eds)

**파일**: `476297.eds:21-31`

```ini
[Device]
        VendCode = 404;                              # VAT Vakuumventile AG
        VendName = "VAT Vakuumventile AG";
        ProdType = 29;                               # Process Control Valve
        ProdTypeStr = "Process Control Valve";
        ProdCode = 650;                              # ← 다름!
        MajRev = 2;
        MinRev = 1;
        ProdName = "VAT Adaptive Pressure Controller";
        Catalog = "650";
```

**매칭 결과**: ❌ **Vendor Code와 Product Code가 모두 다름**

---

## 💡 왜 VAT EDS로 인식되지 않는가?

### 이유 1: Vendor ID 불일치 🔴

```
현재 디바이스: Vendor ID = 283 (Hilscher)
VAT EDS 파일:  VendCode = 404 (VAT Vakuumventile AG)

→ Vendor ID가 다르므로 매칭 불가!
```

### 이유 2: Product Code 불일치 🔴

```
현재 디바이스: Product Code = 34
VAT EDS 파일:  ProdCode = 650

→ Product Code가 다르므로 매칭 불가!
```

### 이유 3: Product Name은 매칭 기준이 아님ℹ️

```
현재 디바이스: Product Name = "VAT_V5_SIMPLE_CONFIG_DEMO"
VAT EDS 파일:  ProdName = "VAT Adaptive Pressure Controller"

→ Product Name은 DTM 매칭에 사용되지 않음!
→ Vendor ID + Product Code만 사용됨!
```

---

## 📈 DTM 매칭 우선순위

DTM/EDS 매칭은 다음 순서로 이루어집니다:

```
1. Vendor Code (필수) ⭐⭐⭐
2. Product Code (필수) ⭐⭐⭐
3. Major Revision (선택) ⭐
4. Minor Revision (선택) ⭐
5. Product Name (표시용) ℹ️
```

**Product Name은 매칭에 사용되지 않고 정보 표시용으로만 사용됩니다!**

---

## ✅ 해결 방안

### 방안 1: VAT Vendor ID 및 Product Code 사용 (권장)

실제 VAT 제품으로 인식되도록 CIP Identity를 변경합니다.

**파일**: `Hil_DemoAppDNS\Sources\AppDNS_DemoApplicationFunctions.c`

**변경 전**:
```c
#define VENDOR_ID                            283        // Hilscher
#define DEVICE_TYPE_COMMUNICATION_ADAPTER    0x0C       // Communications Adapter
#define PRODUCT_CODE                         34         // Hilscher Generic
#define MAJOR_REVISION                       5
#define MINOR_REVISION                       2
char    abProductName[]                      = "VAT_V5_SIMPLE_CONFIG_DEMO";
```

**변경 후**:
```c
#define VENDOR_ID                            404        // VAT Vakuumventile AG
#define DEVICE_TYPE_PROCESS_CONTROL_VALVE    29         // Process Control Valve
#define PRODUCT_CODE                         650        // VAT Adaptive Pressure Controller
#define MAJOR_REVISION                       2
#define MINOR_REVISION                       1
char    abProductName[]                      = "VAT Adaptive Pressure Controller";
```

**효과**:
- ✅ CYCON이 476297.eds (VAT EDS) 파일로 매칭
- ✅ DTM Device가 "VAT Adaptive Pressure Controller"로 표시
- ✅ VAT 전용 파라미터 및 설정 사용 가능

**주의사항**:
- Assembly Instance를 VAT EDS 파일에 맞게 조정해야 함
- IO 데이터 크기 및 구조 변경 필요

---

### 방안 2: 새로운 EDS 파일 생성

현재 설정(Vendor 283, Product 34)을 유지하면서 Product Name만 변경한 새 EDS 파일을 생성합니다.

**새 EDS 파일**: `VAT_V5_SIMPLE_CONFIG_DEMO.EDS`

**필수 설정**:
```ini
[Device]
        VendCode = 283;                              # Hilscher GmbH (현재 코드와 일치)
        VendName = "Hilscher GmbH";
        ProdType = 12;                               # Communications Adapter
        ProdCode = 34;                               # (현재 코드와 일치)
        MajRev = 5;
        MinRev = 2;                                  # (현재 코드와 일치)
        ProdName = "VAT_V5_SIMPLE_CONFIG_DEMO";      # (현재 코드와 일치)
```

**작업 순서**:
1. `DNS_V5_SIMPLE_CONFIG_DEMO.EDS` 복사
2. `VAT_V5_SIMPLE_CONFIG_DEMO.EDS`로 이름 변경
3. `ProdName` 값을 "VAT_V5_SIMPLE_CONFIG_DEMO"로 변경
4. `MinRev` 값을 2로 변경
5. CYCON.net 카탈로그에 등록

**효과**:
- ✅ 현재 코드 설정 유지
- ✅ Product Name이 "VAT_V5_SIMPLE_CONFIG_DEMO"로 표시
- ⚠️ CYCON 카탈로그에 수동 등록 필요

**단점**:
- CYCON.net에 EDS 파일을 수동으로 추가해야 함
- 실제 VAT 제품이 아니므로 VAT 특화 기능 사용 불가

---

### 방안 3: Product Code 변경

Hilscher Vendor ID를 유지하면서 새로운 Product Code를 할당합니다.

**변경**:
```c
#define VENDOR_ID        283        // Hilscher (유지)
#define PRODUCT_CODE     9999       // 새로운 코드 (예시)
```

**새 EDS 파일 생성**:
```ini
[Device]
        VendCode = 283;
        ProdCode = 9999;                             # 새 코드
        ProdName = "VAT_V5_SIMPLE_CONFIG_DEMO";
```

**효과**:
- ✅ Hilscher Vendor 유지
- ✅ 고유한 Product Code
- ⚠️ 새 EDS 파일 생성 및 등록 필요

---

## 📝 권장 조치

### 즉시 조치 (Priority 1)

**목적**: 실제 VAT 제품으로 인식되도록 설정

**작업**:
1. **방안 1 적용**: Vendor ID 404, Product Code 650으로 변경
2. Assembly Instance를 VAT EDS에 맞게 조정
3. 재빌드 및 테스트
4. CYCON 스캔으로 VAT EDS 매칭 확인

---

### 중기 조치 (Priority 2)

**목적**: VAT 특화 기능 구현

**작업**:
1. VAT EDS 파일의 Assembly 정의 확인
2. Input/Output Assembly를 VAT 사양에 맞게 구현
3. VAT 특화 CIP Object 구현 (필요 시)
4. 전체 기능 테스트

---

### 장기 개선 (Priority 3)

**목적**: 유지보수성 향상

**작업**:
1. Identity Object 설정을 헤더 파일로 분리
2. 빌드 시 EDS 파일 자동 업데이트 스크립트 작성
3. 버전 관리 자동화

---

## 🔍 CYCON.net DTM 매칭 로직

### 매칭 알고리즘

```
1. 스캔된 디바이스에서 CIP Identity Object 읽기
   - Vendor ID
   - Device Type
   - Product Code
   - Major/Minor Revision
   - Product Name

2. EDS 카탈로그에서 검색
   FOR EACH EDS in Catalog
       IF (EDS.VendCode == Device.VendorID)
           AND (EDS.ProdCode == Device.ProductCode)
       THEN
           매칭 성공!
           IF (EDS.MajRev != Device.MajorRev) OR (EDS.MinRev != Device.MinorRev)
               경고 메시지 표시
           END IF
           RETURN EDS
       END IF
   END FOR

3. 매칭된 EDS가 없으면
   - "Generic found" 표시
   - Vendor의 Generic DTM 사용
```

**핵심**: Product Name은 검색 조건이 아님!

---

## 📊 비교표

| 항목 | 현재 디바이스 | DNS EDS (매칭됨) | VAT EDS (미매칭) |
|------|--------------|-----------------|-----------------|
| **Vendor ID** | 283 (Hilscher) ✅ | 283 ✅ | 404 ❌ |
| **Product Code** | 34 ✅ | 34 ✅ | 650 ❌ |
| **Device Type** | 12 | 12 ✅ | 29 ❌ |
| **Major Rev** | 5 | 5 ✅ | 2 ⚠️ |
| **Minor Rev** | 2 | 1 ⚠️ | 1 ⚠️ |
| **Product Name** | "VAT_V5_SIMPLE_CONFIG_DEMO" | "DNS_V5_SIMPLE_CONFIG_DEMO" | "VAT Adaptive Pressure Controller" |
| **매칭 결과** | - | ✅ 매칭 | ❌ 미매칭 |

**매칭 기준**: Vendor ID + Product Code
**Product Name**: 표시용, 매칭 기준 아님

---

## 🎯 결론

### 문제 원인

1. **DTM 매칭은 Vendor ID + Product Code로 이루어짐**
2. **현재 디바이스는 Hilscher(283) + 34로 설정되어 있음**
3. **VAT EDS는 VAT(404) + 650으로 설정되어 있음**
4. **Product Name을 변경해도 DTM 매칭에는 영향 없음**

### 해결책

**실제 VAT 제품으로 인식되려면**:
- Vendor ID를 404로 변경
- Product Code를 650으로 변경
- 또는 새 EDS 파일 생성 (Vendor 283, Product 34, Product Name "VAT_V5_SIMPLE_CONFIG_DEMO")

### 권장사항

**방안 1 (VAT Identity 사용)** 권장:
- 실제 VAT 제품으로 완전히 동작
- CYCON에서 VAT EDS 자동 매칭
- VAT 특화 기능 사용 가능

---

## 📚 참고 자료

### CIP Identity Object Attributes

| Attribute | ID | 설명 | 매칭 사용 |
|-----------|-----|------|----------|
| Vendor ID | 1 | 제조사 코드 | ✅ 필수 |
| Device Type | 2 | 디바이스 종류 | ⚠️ 선택 |
| Product Code | 3 | 제품 코드 | ✅ 필수 |
| Revision | 4 | Major.Minor 버전 | ⚠️ 경고 |
| Serial Number | 6 | 시리얼 번호 | ❌ |
| Product Name | 7 | 제품 이름 | ❌ 표시만 |

### DeviceNet EDS 파일 구조

```ini
[File]           # 파일 정보
[Device]         # 디바이스 Identity ← DTM 매칭에 사용
[IO_Info]        # IO Assembly 정의
[Parameters]     # 설정 파라미터
```

---

**분석 완료**: 2025-10-29
**분석자**: Claude AI Assistant
**도구**: Code Analysis, EDS File Parsing, DeviceNet Protocol Analysis
