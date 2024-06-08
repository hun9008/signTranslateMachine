#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/select.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#define PORT 8080
#define BACKLOG 5
#define BUFFER_SIZE 1024

#define LCD_ADDR 0x27 // I2C 주소 (LCD 모듈의 I2C 주소에 맞게 조정)
#define LCD_CHR  1    // 모드 - 데이터를 보낼 때
#define LCD_CMD  0    // 모드 - 명령을 보낼 때

#define LCD_LINE_1  0x80 // 1행
#define LCD_LINE_2  0xC0 // 2행

#define LCD_BACKLIGHT   0x08  // 백라이트 켜기
#define ENABLE          0b00000100 // Enable 비트

#define LED_PIN 0 // LED가 연결된 핀 번호 (GPIO 17, 물리적 핀 11)



int letter[3] = {0, 0, 0};  // 크기가 3인 int 배열, 초기값 0
int tmp[2] = {-1, -1};        // 입력을 받을 int 배열
int lcd; // LCD 제어를 위한 변수

int client_count = 0;
int client_sockets[3] = {0};

char error0[2] = "0";
char error1[2] = "1";

void lcd_init(int lcd);
void lcd_byte(int lcd, int bits, int mode);
void lcd_toggle_enable(int lcd, int bits);
void lcd_string(int lcd, const char *message);
void lcd_clear(int lcd);
int do_mapping(int num, int where);
void led_blink();

void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);

    int client_mode = ++client_count;

    if (client_mode > 3) {
        printf("Too many clients connected. Connection rejected.\n");
        close(sock);
        pthread_exit(NULL);
    }

    client_sockets[client_mode - 1] = sock;

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    char client_ip[INET_ADDRSTRLEN];
    char buffer[BUFFER_SIZE];

    getpeername(sock, (struct sockaddr*)&client_address, &client_address_len);
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("Connected client IP: %s as Mode %d\n", client_ip, client_mode);

    if (client_count == 3) {
        printf("Connection complete\n");
    }

    fd_set readfds;
    int max_sd = sock;


while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
            break;
        }

       if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
                send(sock, buffer, strlen(buffer), 0);
            }
        }

        if (FD_ISSET(sock, &readfds)) {
            int valread = read(sock, buffer, BUFFER_SIZE - 1);
            if (valread <= 0) {
                printf("Client %s disconnected\n", client_ip);
                break;
            }
            buffer[valread] = '\0';
            
            int data;
            
            if (sscanf(buffer, "%06d", &data) == 1){
                if (client_mode == 1) {
                    if (sscanf(buffer, "%d", &data) == 1 && data >= 0 && data <= 111111) { // 이진수 범위 확인
                        // 이진수 문자열을 정수로 변환
                        data = (int)strtol(buffer, NULL, 2);
                        tmp[0] = data;
                    }
                } else if (client_mode == 2) {
                    int data;
                    if (sscanf(buffer, "%d", &data) == 1 && data >= 0 && data <= 2) {
                        tmp[1] = data;
                    }
                }
            }
            

            printf("[Mode %d] -> %s\n", client_mode, buffer);
            printf("tmp: %d %d\n", tmp[0], tmp[1]);

            if (tmp[0] != -1 && tmp[1] != -1) {
                tmp[0] = do_mapping(tmp[0], tmp[1]);
                letter[tmp[1]] = tmp[0];
                printf("letter: %d %d %d\n", letter[0], letter[1], letter[2]);
                tmp[0] = -1;
                tmp[1] = -1;
            }

            if (client_mode == 2 && data == 3) {
                printf("[Mode 2] -> next\n");

                if (letter[0] == 0 || letter[1] == 0) {
                    printf("Incomplete letter\n");
                    write(client_sockets[1], error0, 1);
                    memset(letter, 0, sizeof(letter));
                    lcd_clear(lcd);
                    lcd_string(lcd, "Incomplete Input");
                    led_blink();
                    lcd_clear(lcd);
                } else if ((letter[0] > 14) || (letter[1] > 14) || (letter[2] > 14)) {
                    printf("Wrong input\n");
                    write(client_sockets[1], error1, 1);
                    memset(letter, 0, sizeof(letter));
                    lcd_clear(lcd);
                    lcd_string(lcd, "Wrong Input");
                    led_blink();
                    lcd_clear(lcd);
                } else {
                    printf("Valid letter\n");
                    if((letter[1] >= 10) && (letter[1] <= 14)){
                        int vowel = letter[1] - 10;
                        letter[1] = vowel*14 + letter[0];
                        letter[0] = 0;
                    }
                    printf("letter: %02d %02d %02d\n", letter[0], letter[1], letter[2]);
                    sprintf(buffer, "%02d%02d%02d", letter[0], letter[1], letter[2]);
                    send(client_sockets[2], buffer, strlen(buffer), 0);
                    memset(letter, 0, sizeof(letter));
                    lcd_clear(lcd);
                    lcd_string(lcd, "Valid Letter");
                    delay(5000);
                    lcd_clear(lcd);
                }
            }
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    wiringPiSetup();
    pinMode(LED_PIN, OUTPUT);

    if (wiringPiSetup() == -1) {
        printf("wiringPi 초기화 실패!\n");
        return 1;
    }

    lcd = wiringPiI2CSetup(LCD_ADDR);
    if (lcd == -1) {
        printf("LCD 초기화 실패!\n");
        return 1;
    }

    lcd_init(lcd);

    while (1) {
        int *new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept failed");
            free(new_socket);
            continue;
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_socket) != 0) {
            perror("pthread_create failed");
            free(new_socket);
            continue;
        }

        pthread_detach(client_thread);
    }

    close(server_fd);
    return 0;
}

void lcd_init(int lcd) {
    lcd_byte(lcd, 0x33, LCD_CMD);
    lcd_byte(lcd, 0x32, LCD_CMD);
    lcd_byte(lcd, 0x06, LCD_CMD);
    lcd_byte(lcd, 0x0C, LCD_CMD);
    lcd_byte(lcd, 0x28, LCD_CMD);
    lcd_byte(lcd, 0x01, LCD_CMD); // 클리어 디스플레이
    usleep(500);
}

void lcd_byte(int lcd, int bits, int mode) {
    int bits_high;
    int bits_low;
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;
    
    wiringPiI2CReadReg8(lcd, bits_high);
    lcd_toggle_enable(lcd, bits_high);
    
    wiringPiI2CReadReg8(lcd, bits_low);
    lcd_toggle_enable(lcd, bits_low);
}

void lcd_toggle_enable(int lcd, int bits) {
    usleep(500);
    wiringPiI2CReadReg8(lcd, (bits | ENABLE));
    usleep(500);
    wiringPiI2CReadReg8(lcd, (bits & ~ENABLE));
    usleep(500);
}

void lcd_string(int lcd, const char *message) {
    while (*message) {
        lcd_byte(lcd, *(message++), LCD_CHR);
    }
}

void lcd_clear(int lcd) {
    lcd_byte(lcd, 0x01, LCD_CMD); // 클리어 디스플레이 명령
    usleep(2000); // 지연 시간
}

int do_mapping(int num, int where){
    int map;
    if((where == 0) || (where == 2)){ //자음
        switch(num){
            case 14:
                map = 1; //ㄱ
                break;
            case 15:
                map = 2; //ㄴ
                break;
            case 39:
                map = 3; //ㄷ
                break;
            case 34:
                map = 4; //ㄹ
                break;
            case 62:
                map = 5; //ㅁ
                break;
            case 32:
                map = 6; //ㅂ
                break;
            case 38:
                map = 7; //ㅅ
                break;
            case 48:
                map = 8; //ㅇ
                break;
            case 6:
                map = 9; //ㅈ
                break;
            case 2:
                map = 10; //ㅊ
                break;        
            case 22:
                map = 11; //ㅋ
                break;  
            case 30:
                map = 14; //ㅎ
                break;                      
            default:
                map = 15; //error
        }
    }
    else{ //모음
        num = num / 2;
        switch(num){
            case 15:
                map = 1; //ㅏ
                break;
            case 19:
                map = 2; //ㅑ
                break;
            case 23:
                map = 3; //ㅓ
                break;
            case 7:
                map = 4; //ㅕ
                break;
            case 30:
                map = 5; //ㅣ
                break;
            case 22:
                map = 6; //ㅐ
                break;
            case 6:
                map = 7; //ㅔ
                break;
            case 20:
                map = 8; //ㅒ
                break;
            case 18:
                map = 9; //ㅖ
                break;
            case 27:
                map = 10; //ㅗ
                break;        
            case 25:
                map = 11; //ㅛ
                break;  
            case 29:
                map = 12; //ㅜ
                break;     
            case 28:
                map = 13; //ㅠ
                break;    
            case 31:
                map = 14; //ㅡ
                break;               
            default:
                map = 15; //error
        }
    }

    return map;
}

void led_blink(){
    for(int i=0; i<5; i++){
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }
}
