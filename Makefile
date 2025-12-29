# 컴파일러 설정
CC = aarch64-linux-gnu-gcc
CLIENT_CC = gcc 

CFLAGS = -Wall -fPIC -I./server/include
LIB_LIBS = -lwiringPi -lpthread
MAIN_LIBS = -lwiringPi -ldl -lpthread -rdynamic

# 디렉토리 설정 
LIB_DIR = server/lib
SRC_DIR = server/src

# 타겟 설정
LIB_TARGET = $(LIB_DIR)/libdevice_control.so
SERVER_TARGET = maintest
CLIENT_TARGET = client_app 

# 소스 파일 설정
LIB_SRCS = $(SRC_DIR)/led_thread_routine.c \
           $(SRC_DIR)/buzzer_thread_routine.c \
           $(SRC_DIR)/sensor_thread_routine.c \
           $(SRC_DIR)/fnd_thread_routine.c \
           $(SRC_DIR)/server_thread.c

MAIN_SRCS = $(SRC_DIR)/main.c
CLIENT_SRCS = client/client.c

all: $(LIB_DIR) $(LIB_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# 라이브러리 빌드 시 소스파일 존재 여부 확인을 위해 앞에 @ls 등을 넣어 디버깅 가능
$(LIB_TARGET): $(LIB_SRCS)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LIB_LIBS)

$(SERVER_TARGET): $(MAIN_SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(MAIN_LIBS)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CLIENT_CC) -Wall -o $@ $^ $(CLIENT_LIBS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	rm -rf $(LIB_DIR)

.PHONY: all clean
