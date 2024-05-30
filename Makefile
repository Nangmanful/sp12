# 컴파일러 설정
CXX = g++
CC = gcc

# 컴파일 옵션
CXXFLAGS = -c
CFLAGS = -c

# 대상 파일
TARGET = myprogram

# 소스 파일
C_SOURCES = main.c
CPP_SOURCES = header.cpp

# 객체 파일
C_OBJS = $(C_SOURCES:.c=.o)
CPP_OBJS = $(CPP_SOURCES:.cpp=.o)

# 모든 객체 파일
OBJS = $(C_OBJS) $(CPP_OBJS)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

# C 소스 파일 컴파일
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# C++ 소스 파일 컴파일
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# 클린업
clean:
	rm -f $(OBJS) $(TARGET)
