#include <bcm2835.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>

#define IN 0
#define OUT 1

#define PIN 18
#define VALUE_MAX 40
#define DIRECTION_MAX 40

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

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
int inclination(){
    int f; //기울기
    if(GPIOExport(PIN) == -1){
        printf("eroor\n");
    }
    if(GPIODirection(PIN, IN) == -1){
        printf("eroor\n");
    }
    f = GPIORead(PIN);
    return f;
}
int read_adc(uint8_t channel) {
    char tx[] = { 1, (8 + channel) << 4, 0 };
    char rx[3];
    bcm2835_spi_transfernb(tx, rx, 3);
    printf("%02x %02x %02x", rx[0], rx[1], rx[2]);
    return ((rx[1] & 3) << 8) + rx[2];
}
void* flex_sensor(void *arg) {
    int sock = *(int *)arg;
    int result;
    int a, b, c, d, e, f;
    char bit[6];
    if (!bcm2835_init()) {
        printf("bcm2835 initialization failed\n");
    }

    if (!bcm2835_spi_begin()) {
        printf("bcm2835 SPI begin failed\n");
    }

    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);

    while (1) {
        int flexADC0 = read_adc(0);
        printf("Flex Sensor 1 ADC Value: %d\n", flexADC0);
        if(flexADC0 >= 1000)
            a = 1;
        else
            a = 0;
        int flexADC1 = read_adc(1);
        printf("Flex Sensor 2 ADC Value: %d\n", flexADC1);
        if(flexADC1 >= 1000)
            b = 1;
        else
            b = 0;
        int flexADC2 = read_adc(2);
        printf("Flex Sensor 3 ADC Value: %d\n", flexADC2);
        if(flexADC2 >= 1000)
            c = 1;
        else
            c = 0;
        int flexADC3 = read_adc(3);
        printf("Flex Sensor 4 ADC Value: %d\n", flexADC3);
        if(flexADC3 >= 1000)
            d = 1;
        else
            d = 0;
        int flexADC4 = read_adc(4);
        printf("Flex Sensor 5 ADC Value: %d\n", flexADC4);
        if(flexADC4 >= 1000)
            e = 1;
        else
            e = 0;
        if(a == 0 && b == 0 && c == 1 && d == 1 && e == 1){
          f = inclination();
        }
        else if(a == 1 && b == 0 && c == 0 && d == 1 && e == 1){
          f = inclination();
        }
        else
          f = 0;
        sprintf(bit, "%d%d%d%d%d%d", a, b, c, d, e, f);
        printf("%s\n", bit);
        if(write(sock, bit, sizeof(bit)) == -1){
            printf("write() error");
        }
        bcm2835_delay(500);
    }

    bcm2835_spi_end();
    bcm2835_close();
}

int main(int argc, char **argv){
   int sock = -1;
   struct sockaddr_in serv_addr;
   pthread_t p_thread[2];
   if (argc != 3) {
       printf("Usage : %s <IP> <port>\n", argv[0]);
       exit(1);
   }

   sock = socket(PF_INET, SOCK_STREAM, 0);
   if (sock == -1) error_handling("socket() error");

   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
   serv_addr.sin_port = htons(atoi(argv[2]));
   if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
       error_handling("connect() error");

   printf("Connection established\n");
   if(pthread_create(&p_thread[0], NULL, flex_sensor, (void*)&sock) == -1){
      error_handling("pthread_create() error");
   }
   pthread_join(p_thread[0], NULL);
   close(sock);
   return 0;
}
