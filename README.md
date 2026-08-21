# Prj4 시스템 프로그래밍 & 동적 메모리 할당기 (Dynamic Storage Allocator)

본 저장소는 C 기반 저수준 메모리 관리 및 시스템 콜/프로세스 제어 메커니즘을 학습하고 최적화한 프로젝트입니다.

### Key Implementations & Performance Tuning
* **Segregated Free List 기반 Dynamic Memory Allocator 구현**:
  * 묵시적/명시적 가용 리스트의 $O(N)$ 탐색 병목을 개선하여 **가용 블록 탐색 시간 $O(1)$로 최적화**
  * **Immediate Coalescing & Splitting** 기법을 적용하여 외부 단편화(External Fragmentation) 최소화
  * CSAPP `mdriver` 벤치마크 평가 결과: **메모리 효율 및 처리 속도 점수 55점 -> 92점으로 개선**
* **저수준 파일 I/O 및 프로세스 제어**: UNIX 시스템 콜 기반 I/O 리다이렉션 및 프로세스 라이프사이클 관리

### Tech Stack
* C, GDB, Valgrind, Linux/UNIX Environment
