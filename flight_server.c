#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/shm.h>

#define BUFFER_SIZE 1024
#define MSG_KEY 1234
#define SHM_KEY 5678
#define MAX_SEATS 100

// 예약 정보 구조체 정의
struct flight_info {
    int seats[MAX_SEATS];
};

// 메시지 구조체 정의
struct msgbuf {
    long mtype;
    char mtext[BUFFER_SIZE];
};

// 시그널 핸들러
void handle_signal(int signal) {
    printf("Received signal: %d\n", signal);
    // 알림을 로그 파일에 기록 (File I/O)
    FILE *log_file = fopen("server_log.txt", "a");
    if (log_file != NULL) {
        time_t now = time(NULL);
        fprintf(log_file, "Received signal: %d at %s", signal, ctime(&now));
        fclose(log_file);
    }
}

// 클라이언트 요청 처리 함수
void handle_client(int msgid, int shmid) {
    struct msgbuf msg;
    struct flight_info *info = (struct flight_info *)shmat(shmid, NULL, 0);

    if (info == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    while (1) {
        // 클라이언트 요청 받기 (Message Queue)
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // 클라이언트 요청 처리
        if (strncmp(msg.mtext, "BOOK ", 5) == 0) {
            int seat_number;
            sscanf(msg.mtext + 5, "%d", &seat_number);
            if (seat_number >= 0 && seat_number < MAX_SEATS) {
                if (info->seats[seat_number] == 0) {
                    info->seats[seat_number] = 1;
                    strcpy(msg.mtext, "Booking successful");
                } else {
                    strcpy(msg.mtext, "Seat already booked");
                }
            } else {
                strcpy(msg.mtext, "Invalid seat number");
            }
        } else if (strncmp(msg.mtext, "CANCEL ", 7) == 0) {
            int seat_number;
            sscanf(msg.mtext + 7, "%d", &seat_number);
            if (seat_number >= 0 && seat_number < MAX_SEATS) {
                if (info->seats[seat_number] == 1) {
                    info->seats[seat_number] = 0;
                    strcpy(msg.mtext, "Cancellation successful");
                } else {
                    strcpy(msg.mtext, "Seat not booked");
                }
            } else {
                strcpy(msg.mtext, "Invalid seat number");
            }
        } else if (strncmp(msg.mtext, "STATUS", 6) == 0) {
            char status[BUFFER_SIZE] = "Seat status: ";
            for (int i = 0; i < MAX_SEATS; i++) {
                char seat_status[4];
                sprintf(seat_status, "%d ", info->seats[i]);
                strcat(status, seat_status);
            }
            strcpy(msg.mtext, status);
        }

        // 클라이언트에게 응답 전송 (Message Queue)
        msg.mtype = 2;
        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }

    shmdt(info); // 공유 메모리 분리
}

int main() {
    int msgid, shmid;
    key_t key = MSG_KEY;
    key_t shmkey = SHM_KEY;

    // 메시지 큐 생성 (Message Queue)
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    // 공유 메모리 생성 (Shared Memory)
    if ((shmid = shmget(shmkey, sizeof(struct flight_info), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    // 공유 메모리 초기화 (Shared Memory Initialization)
    struct flight_info *info = (struct flight_info *)shmat(shmid, NULL, 0);
    if (info == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    for (int i = 0; i < MAX_SEATS; i++) {
        info->seats[i] = 0;
    }

    shmdt(info); // 공유 메모리 분리

    // 시그널 핸들러 설정 (Signal Handling)
    signal(SIGUSR1, handle_signal);

    while (1) {
        pid_t pid = fork();
        if (pid == 0) { // 자식 프로세스 (Process Creation)
            execl("./helper", "helper", NULL); // exec 함수 사용
            perror("execl");
            exit(1);
        } else if (pid > 0) {
            wait(NULL); // 자식 프로세스 종료 대기
        } else {
            perror("fork");
            exit(1);
        }
    }

    return 0;
}
