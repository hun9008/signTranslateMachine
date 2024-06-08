#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define SOUND 13
#define INPUT 4
#define PIN 20
#define POUT 21
#define PIN2 23
#define POUT2 24
#define VALUE_MAX 40
#define DIRECTION_MAX 40

#define RED 17
#define GREEN 27
#define BLUE 22
static int GPIOExport(int pin) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}
static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 ==
      write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}
static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}
void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
void* get_error_code(void *arg){
  int sock = *(int *)arg;
  int str_len;
  char msg[2];
  char error;
  if (GPIOExport(SOUND) == -1) {
    //return 1;
  }
  if (GPIODirection(SOUND, OUT) == -1) {
    //return 1;
  }
  while(1){
    printf("do\n");
    str_len = read(sock, msg, 1);
    error = msg[0];
    printf("%c\n", error);
    if (str_len == -1) error_handling("read() error");
    if(error == '0'){
      printf("error 1\n");
      GPIOWrite(SOUND, 1);
      usleep(2000 * 1000);
      GPIOWrite(SOUND, 0);
    }
    if(error == '1'){
      printf("error 2\n");
      GPIOWrite(SOUND, 1);
      usleep(1000 * 2000);
      GPIOWrite(SOUND, 0);
      usleep(1000 * 1000);
      GPIOWrite(SOUND, 1);
      usleep(1000 * 2000);
      GPIOWrite(SOUND, 0);            
    }  
  }
}
void* change_place(void *arg){
  int sock = *(int *)arg;
  int constant_ctr = 0;
  int next_ctr = 0;
  int constant_cnt = 0;
  int input_flag = 0;
  char where2put[5];
  char next2put[7] = "3";
  if (GPIOExport(POUT) == -1 || GPIOExport(PIN) == -1) {
    //return 1;
  }
  if (GPIOExport(POUT2) == -1 || GPIOExport(PIN2) == -1) {
    //return 1;
  }
  if (GPIOExport(RED) == -1 || GPIOExport(BLUE) == -1 || GPIOExport(GREEN) == -1) {
    //return 1;
  }
  if (GPIOExport(INPUT) == -1) {
    //return 1;
  }
  if (GPIODirection(POUT, OUT) == -1 || GPIODirection(PIN, IN) == -1) {
    //return 2;
  }
  if (GPIODirection(POUT2, OUT) == -1 || GPIODirection(PIN2, IN) == -1) {
    //return 2;
  }
  if (GPIODirection(RED, OUT) == -1 || GPIODirection(GREEN, OUT) == -1 || GPIODirection(BLUE, OUT) == -1) {
    //return 2;
  }
  if (GPIODirection(INPUT, OUT) == -1) {
    //return 1;
  }
  GPIOWrite(RED, 1);
  while(1){
    if (GPIOWrite(POUT, 1) == -1) {
      //return 3;
    }
    if (GPIOWrite(POUT2, 1) == -1) {
      //return 3;
    }
    constant_ctr = GPIORead(PIN2);
    next_ctr = GPIORead(PIN);
    if(constant_ctr == 0){
        if(constant_cnt % 3 == 0){
            sprintf(where2put, "%d", constant_cnt % 3);
            GPIOWrite(RED, 0);
            GPIOWrite(GREEN, 1);
        }
        else if(constant_cnt % 3 == 1){
            sprintf(where2put, "%d", constant_cnt % 3);
            GPIOWrite(GREEN, 0);
            GPIOWrite(BLUE, 1);
        }
        else if(constant_cnt % 3 == 2){
            sprintf(where2put, "%d", constant_cnt % 3);
            GPIOWrite(BLUE, 0);
            GPIOWrite(RED, 1);
        }
        if(write(sock, where2put, sizeof(where2put)) == -1)
          error_handling("write() error");
        constant_cnt+=1;
    }
    if(next_ctr == 0){
        GPIOWrite(BLUE, 0);
        printf("BLUE LED turned off\n");
        GPIOWrite(GREEN, 0);
        printf("GREEN LED turned off\n");
        GPIOWrite(INPUT, 1);
        printf("INPUT turned on\n");
        GPIOWrite(RED, 1);
        printf("RED LED turned on\n");
        constant_cnt = 0;
        if(write(sock, next2put, sizeof(next2put)) == -1)
          error_handling("write() error");
        usleep(1000 * 1000);
        GPIOWrite(INPUT, 0);
    }
    printf("GPIO20Read: %d from pin %d\n", next_ctr, PIN);
    printf("GPIO23Read: %d from pin %d\n", constant_ctr, PIN2);
    usleep(1000 * 1000);
  }

  if (GPIOUnexport(POUT) == -1 || GPIOUnexport(PIN) == -1) {
    //return 4;
  }
  if (GPIOUnexport(POUT2) == -1 || GPIOUnexport(PIN2) == -1) {
    //return 4;
  }
  if (GPIOUnexport(INPUT) == -1) {
    //return 1;
  }
}
int main(int argc, char **argv) {
   int sock = -1;
   struct sockaddr_in serv_addr;
   pthread_t p_thread[2];

   if (argc != 3) {
       printf("Usage : %s <IP> <port>\n", argv[0]);
       exit(1);
     }

   sock = socket(PF_INET, SOCK_STREAM, 0); // 소켓 생성
   if (sock == -1) error_handling("socket() error");

   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
   serv_addr.sin_port = htons(atoi(argv[2])); // 주소 설정
   if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) // 서버에 연결
       error_handling("connect() error");

   //두 개의 스레드 생성
   printf("Connection established\n");
   if(pthread_create(&p_thread[0], NULL, get_error_code, (void*)&sock) == -1){ // thread_input_to_socket 함수를 실행하는 스레드
      error_handling("pthread_create() error");
   }

  
   if(pthread_create (&p_thread[1], NULL, change_place, (void*)&sock) == -1){ // thread_socket_to_output 함수를 실행하는 스레드
      error_handling("pthread_create() error");
   }
   pthread_join(p_thread[0], NULL);
   pthread_join(p_thread[1], NULL);
   close(sock); // 소켓 닫기
   return 0;
}