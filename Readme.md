# 🎵 Chtice (치지직 실시간 방송 알림 디스코드 봇)

![C++](https://img.shields.io/badge/C++20-%2300599C.svg?style=flat-square&logo=c%2B%2B&logoColor=white)
![MySQL](https://img.shields.io/badge/MySQL-4479A1?style=flat-square&logo=mysql&logoColor=white)
![Fedora](https://img.shields.io/badge/Fedora_40-51A2DA?style=flat-square&logo=fedora&logoColor=white)
![Discord](https://img.shields.io/badge/Discord-5865F2?style=flat-square&logo=discord&logoColor=white)

> 네이버 치지직(Chzzk) OPEN API를 활용하여 스트리머의 방송 상태를 실시간으로 모니터링하고, 지정된 디스코드 채널에 알림을 전송하는 C++ 기반 애플리케이션입니다. 

[🤖 Chtice 디스코드 서버에 초대하기](https://discord.com/oauth2/authorize?client_id=1537339670017351710)

---

## 📂 디렉토리 구조 (Project Structure)

```text
chtice/prj/
│
├── src/                  # 메인 소스 코드
    ├── proc/             # 프로세스 로직
    │   ├── common.h / .cpp       # 공통 유틸리티
    │   ├── dbquery.h / .cpp      # SQL 쿼리 매니저
    │   ├── chtice_daemon.h/.cpp  # Polling Engine 및 상태 감지
    │   └── chtice_bot.h / .cpp   # 디스코드 슬래시 명령어 핸들러
    │
    └── lib/              # 코어 공통 라이브러리
          ├── client.h / .cpp     # TCP Socket 통신 클라이언트
          ├── dbmysql.h / .cpp    # MySQL 커넥터
          ├── server.h / .cpp     # TCP Socket 통신 서버
          ├── https.h / .cpp      # HTTPS 요청 처리 모듈
          └── util.h / .cpp       # 전역 유틸리티 함수
```

---

