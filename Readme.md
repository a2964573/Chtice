# 🎵 Chtice (치지직 실시간 방송 알림 디스코드 봇)

![C++](https://img.shields.io/badge/C++20-%2300599C.svg?style=flat-square&logo=c%2B%2B&logoColor=white)
![MySQL](https://img.shields.io/badge/MySQL-4479A1?style=flat-square&logo=mysql&logoColor=white)
![Fedora](https://img.shields.io/badge/Fedora_40-51A2DA?style=flat-square&logo=fedora&logoColor=white)
![Discord](https://img.shields.io/badge/Discord-5865F2?style=flat-square&logo=discord&logoColor=white)

> 네이버 치지직(Chzzk) 오픈 API를 활용하여 스트리머의 방송 켜짐 상태를 실시간으로 모니터링하고, 지정된 디스코드 채널에 알림을 전송하는 C++ 기반 데몬(Daemon) 애플리케이션입니다. 현재 약 185명의 스트리머를 안정적으로 트래킹하고 있습니다.

[🤖 Chtice 디스코드 서버에 초대하기](https://discord.com/oauth2/authorize?client_id=1537339670017351710)

---

## 🏗️ 시스템 아키텍처 (System Architecture)

![Chtice Architecture](image_c0c0a3.png)

1. **상태 모니터링 (Daemon):** 주기적으로 MySQL에서 대상 목록을 조회하고 치지직 API를 호출해 라이브 상태(ON/OFF)를 갱신합니다.
2. **명령어 처리 (Bot):** 디스코드 유저의 `/등록`, `/삭제` 명령을 수신하여 MySQL에 연동 데이터를 관리합니다.
3. **알림 발송 (Bot):** MySQL 조회 시 `방송 ON` 상태이며 알림이 발송되지 않은 데이터를 찾아 디스코드로 최종 메시지를 전송하고 발송 상태를 업데이트합니다.

---

## 🔥 핵심 엔지니어링 및 트러블슈팅 (Key Engineering)

### 1. 웹훅(Webhook) 부재에 따른 Polling Engine 설계 및 Rate Limit 방어
* **문제:** 치지직 오픈 API는 방송 상태 변화를 실시간으로 밀어주는(Push) 공식 웹훅을 지원하지 않아, 클라이언트 측에서 지속적으로 상태를 확인해야 했습니다.
* **해결:** 외부 API 서버에 가해지는 부하와 429(Too Many Requests) 에러를 원천 차단하기 위해 단일 스레드 동기식(Blocking I/O) 호출 방식을 채택했습니다. 185명 규모의 상태 조회 루프가 완료된 후 **고정된 30초 대기(Sleep)를 강제하는 Rate Limit 컨트롤러**를 구현하여 트래픽 밀도를 일정하게 유지하고 안정성을 확보했습니다.

### 2. 마이크로서비스형 구조와 MySQL 기반 비동기 통신
* 데몬(`chtice_daemon`)과 봇(`chtice_bot`) 프로세스를 완전히 분리했습니다.
* 데몬은 상태 갱신만 담당하고 봇은 유저 응대 및 알림만 담당하며, 두 프로세스는 오직 **MySQL**을 매개로만 통신합니다. 이를 통해 한쪽 프로세스에 문제가 생겨도 시스템 전체가 다운되지 않는 낮은 결합도(Decoupling)를 달성했습니다.

### 3. C++20 기반 메모리 최적화 (RAII)
* 수명 주기가 긴 데몬 프로세스의 특성을 고려해 객체의 생성과 소멸을 묶는 RAII(Resource Acquisition Is Initialization) 패턴을 엄격하게 적용했습니다.
* `std::make_unique`, `std::shared_ptr` 등 C++ 스마트 포인터를 적극 활용하여 멀티스레딩 환경에서도 안전한 메모리 참조를 보장하고 누수(Memory Leak)를 차단했습니다.

### 4. Linux 인프라 및 데몬 무중단 자동화
* Fedora 40 환경에서 논리 볼륨 관리자(LVM)를 활용해 파티션을 할당하고, 봇 구동을 위한 전용 사용자 계정(`chtice`)을 생성해 보안 격리 수준을 높였습니다.
* `systemd` 백그라운드 서비스로 등록하는 과정에서 발생한 권한 및 SELinux 문제를 트러블슈팅하여 서버 재부팅 시에도 데몬이 무중단 자동 실행되도록 파이프라인을 구축했습니다.

---

## 📂 디렉토리 구조 (Project Structure)

```text
/home/chtice/prj/
│
├── conf/                 # 환경 설정 (.env, DB/API 타겟 설정)
├── src/                  # 메인 소스 코드
│   ├── proc/             # 프로세스 로직 (데몬 & 봇)
│   │   ├── common.h / .cpp       # 공통 유틸리티
│   │   ├── dbquery.h / .cpp      # SQL 쿼리 매니저
│   │   ├── chtice_daemon.h/.cpp  # Polling Engine 및 상태 감지
│   │   └── chtice_bot.h / .cpp   # 디스코드 슬래시 명령어 핸들러
│   │
│   └── lib/              # 코어 공통 라이브러리
│         ├── client.h / .cpp     # HTTP / Socket 통신 클라이언트
│         ├── dbmysql.h / .cpp    # MySQL 커넥터
│         ├── server.h / .cpp     # 로컬 서버/소켓 통신
│         ├── https.h / .cpp      # HTTPS 요청 처리 모듈
│         └── util.h / .cpp       # 전역 유틸리티 함수
│
├── app/                  # 빌드된 실행 바이너리 파일
├── build/                # 컴파일된 오브젝트(.o) 및 공유 라이브러리(.so)
└── log/                  # 일자별 런타임/에러 로그 (YYYYMMDD)
```

---

## 🚀 빌드 및 실행 (Build & Run)

이 프로젝트는 Linux 환경에서 `GNU Make`를 사용하여 빌드합니다.

### 1. Build
```bash
cd src/
make
```

### 2. Run (Background)
`systemd` 서비스 등록을 권장하며, 수동 실행 시 아래와 같이 데몬과 봇을 각각 백그라운드로 실행합니다.
```bash
cd ../app/
./chtice_daemon &
./chtice_bot &
```

### 3. Logs
로그는 `log/YYYYMMDD/` 디렉토리 하위에 매일 분리되어 쌓입니다.
```bash
tail -f log/20260812/main.20260812.log
```

