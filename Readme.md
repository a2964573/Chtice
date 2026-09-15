🎵 Chtice - 치지직(Chzzk) 실시간 방송 알림 디스코드 봇
Chtice는 네이버 치지직(Chzzk) 오픈 API를 활용하여 스트리머의 방송 켜짐 상태를 실시간으로 모니터링하고, 지정된 디스코드 채널에 알림을 전송하는 C++20 기반의 데몬(Daemon) 애플리케이션입니다.

현재 약 185명의 스트리머 상태를 추적하고 있으며, API 트래픽 제어(Rate Limit)와 안정적인 메모리 관리에 중점을 두고 설계되었습니다.

🤖 Chtice 디스코드 서버에 초대하기

🛠 Tech Stack
Language: C++20

Library/API: DPP (Discord C++), Naver Chzzk Open API

Database: MariaDB 10.6

OS / Infra: Fedora 40 Linux, systemd, LVM

Build: GNU Make

📂 Project Structure
프로젝트는 크게 백그라운드에서 상태를 감지하는 Daemon과 사용자의 명령어를 처리하는 Bot 프로세스로 분리되어 있으며, 공통 모듈은 lib/에 구현되어 있습니다.

Plaintext
/home/chtice/prj/
│
├── conf/                 # 설정 파일 (DB 접속 정보, API 키 등)
│   ├── .env
│   ├── interface.ini
│   └── targets.json
├── src/                  # 소스 코드
│   ├── makefile
│   ├── proc/             # 메인 프로세스 로직
│   │   ├── makefile
│   │   ├── common.h / .cpp       # 공통 유틸리티
│   │   ├── dbquery.h / .cpp      # SQL 쿼리 매니저
│   │   ├── chtice_daemon.h / .cpp # 치지직 API 폴링 및 상태 감지 엔진
│   │   └── chtice_bot.h / .cpp    # 디스코드 슬래시 명령어 처리 및 알림 발송
│   │
│   └── lib/              # 코어 라이브러리 (네트워크, DB 등)
│         ├── makefile
│         ├── client.h / .cpp     # HTTP/소켓 클라이언트 로직
│         ├── dbmysql.h / .cpp    # MariaDB 커넥터
│         ├── server.h / .cpp     # 로컬 서버/소켓 통신
│         ├── https.h / .cpp      # HTTPS 요청 처리
│         └── util.h / .cpp       # 전역 유틸리티 함수 모음
│
├── app/                  # 빌드된 실행 파일(Target) 위치
│   ├── chtice_daemon
│   └── chtice_bot
│
├── build/                # 컴파일된 오브젝트(.o) 및 공유 라이브러리(.so) 파일
│
└── log/                  # 일자별 런타임 로그 및 에러 로그 관리
    └── YYYYMMDD/
       ├── main.YYYYMMDD.log
       └── error.YYYYMMDD.log
⚙️ Core Architecture & Features
1. Polling Engine & Rate Limit Control (chtice_daemon)
치지직 서버 부하 및 429(Too Many Requests) 에러를 원천 차단하기 위해 단일 스레드 동기식(Blocking I/O) API 호출 채택.

약 185명의 스트리머 상태 조회 루프 완료 후 고정된 30초 대기(Sleep) 컨트롤러를 적용하여 안정적인 트래픽 조율.

2. Separation of Concerns (마이크로서비스형 아키텍처)
Daemon (chtice_daemon): 치지직 API와 통신하며 라이브 상태 변화(OFF -> ON)를 감지하고 DB를 갱신합니다.

Bot (chtice_bot): 사용자의 디스코드 명령어(/등록, /삭제 등)를 처리하고, 상태가 ON인 스트리머에 한해 최종 디스코드 알림을 발송합니다.

두 프로세스는 독립적으로 구동되며, MariaDB를 매개로만 비동기적으로 통신하여 시스템 안정성을 높였습니다.

3. Resource & Memory Management (lib/)
C++20의 스마트 포인터(RAII 패턴)를 엄격하게 적용하여 장기 구동되는 데몬 프로세스의 메모리 누수 방지.

dbmysql, https 모듈 등을 분리하여 코드 재사용성과 유지보수성을 극대화.

🚀 Build & Run
해당 프로젝트는 Linux 환경에서 make 명령어를 통해 손쉽게 빌드할 수 있습니다.

Bash
# 전체 프로젝트 빌드
cd src/
make

# 데몬 및 봇 실행 (보통 systemd 서비스로 자동 관리됨)
cd ../app/
./chtice_daemon &
./chtice_bot &

