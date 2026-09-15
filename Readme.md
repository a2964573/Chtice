# 🎵 Chtice (치지직 실시간 방송 알림 디스코드 봇)

![C++](https://img.shields.io/badge/c++20-%2300599C.svg?style=flat-square&logo=c%2B%2B&logoColor=white)
![MariaDB](https://img.shields.io/badge/MariaDB-003545?style=flat-square&logo=mariadb&logoColor=white)
![Fedora](https://img.shields.io/badge/Fedora_40-51A2DA?style=flat-square&logo=fedora&logoColor=white)
![Discord](https://img.shields.io/badge/Discord-5865F2?style=flat-square&logo=discord&logoColor=white)

> 네이버 치지직(Chzzk) 오픈 API를 활용하여 스트리머의 방송 켜짐 상태를 실시간으로 모니터링하고, 지정된 디스코드 채널에 알림을 전송하는 C++ 기반 데몬(Daemon) 애플리케이션입니다.

[🤖 Chtice 디스코드 서버에 초대하기](https://discord.com/oauth2/authorize?client_id=1537339670017351710)

## 📌 주요 특징 (Key Features)

- **API 스로틀링 및 Rate Limit 방어:** 치지직 서버 부하 및 429 에러를 방지하기 위해 185명 규모의 단일 스레드 동기식(Blocking I/O) 호출과 고정된 30초 대기(Sleep) 컨트롤러 적용.
- **마이크로서비스형 아키텍처:** 데몬(`chtice_daemon`)과 봇(`chtice_bot`) 프로세스를 분리하고, MariaDB를 통해서만 비동기적으로 통신하여 시스템 결합도를 낮추고 안정성 극대화.
- **메모리 안정성:** 장기 구동되는 데몬의 특성상 C++20 RAII 패턴과 스마트 포인터를 엄격히 적용하여 메모리 누수 원천 차단.

---

## 📂 디렉토리 구조 (Project Structure)

프로젝트는 백그라운드 상태 감지(Daemon), 디스코드 명령어 처리(Bot), 그리고 공통 라이브러리(Lib)로 구성되어 있습니다.

```text
/home/chtice/prj/
│
├── conf/                 # 설정 파일 (.env, DB/API 대상 설정)
├── src/                  # 소스 코드
│   ├── proc/             # 메인 프로세스 로직 (데몬 & 봇)
│   │   ├── common.h / .cpp       # 공통 유틸리티
│   │   ├── dbquery.h / .cpp      # SQL 쿼리 매니저
│   │   ├── chtice_daemon.h/.cpp  # 상태 감지 엔진 및 폴링
│   │   └── chtice_bot.h / .cpp   # 디스코드 명령어(/등록, /삭제) 처리
│   │
│   └── lib/              # 코어 라이브러리
│         ├── client.h / .cpp     # HTTP / Socket 클라이언트
│         ├── dbmysql.h / .cpp    # MariaDB 커넥터
│         ├── server.h / .cpp     # 로컬 서버 통신
│         ├── https.h / .cpp      # HTTPS 요청 처리
│         └── util.h / .cpp       # 전역 유틸리티 함수
│
├── app/                  # 빌드된 실행 파일 (Target)
├── build/                # 컴파일된 오브젝트(.o) 및 라이브러리(.so)
└── log/                  # 일자별 런타임/에러 로그 (YYYYMMDD)

