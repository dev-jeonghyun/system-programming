# 컴파일러 및 옵션
CC = gcc
CFLAGS = -Wall -Wextra -g

# 오브젝트 파일 목록
OBJS = main.o list.o hash.o bitmap.o debug.o hex_dump.o

# 최종 실행 파일 이름
TARGET = testlib

# 기본 타겟: 컴파일
all: $(TARGET)

# 링크: 실행 파일 생성
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# 개별 소스 파일 컴파일 규칙
main.o: main.c list.h hash.h bitmap.h
	$(CC) $(CFLAGS) -c main.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c

bitmap.o: bitmap.c bitmap.h
	$(CC) $(CFLAGS) -c bitmap.c

debug.o: debug.c debug.h
	$(CC) $(CFLAGS) -c debug.c

hex_dump.o: hex_dump.c hex_dump.h
	$(CC) $(CFLAGS) -c hex_dump.c

# 클린 타겟: 생성된 파일 삭제
clean:
	rm -f *.o $(TARGET) *.out *.output *.result my_output.txt
