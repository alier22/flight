#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MSG_KEY 1234

// 메시지 구조체 정의
struct msgbuf {
	long mtype;
	char mtext[BUFFER_SIZE];
};

void send_request(int msgid, const char *request) {
	struct msgbuf msg;
	msg.mtype = 1;
	strncpy(msg.mtext, request, BUFFER_SIZE);

	if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
		perror("msgsnd");
		exit(1);
	}

	// 서버로부터 응답 수신
	if (msgrcv(msgid, &msg, sizeof(msg.mtext), 2, 0) == -1) {
		perror("msgrcv");
		exit(1);
	}

	printf("Server response: %s\n", msg.mtext);
}

int main() {
	int msgid;
	key_t key = MSG_KEY;

	// 메시지 큐 접근
	if ((msgid = msgget(key, 0666)) == -1) {
		perror("msgget");
		exit(1);
	}

	int choice;
	int seat_number;
	char request[BUFFER_SIZE];

	while (1) {
		printf("1. Book seat\n2. Cancel booking\n3. Check seat status\n4. Exit\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);

		switch (choice) {
			case 1:
				printf("Enter seat number to book: ");
				scanf("%d", &seat_number);
				snprintf(request, BUFFER_SIZE, "BOOK %d", seat_number);
				send_request(msgid, request);
				break;
			case 2:
				printf("Enter seat number to cancel: ");
				scanf("%d", &seat_number);
				snprintf(request, BUFFER_SIZE, "CANCEL %d", seat_number);
				send_request(msgid, request);
				break;
			case 3:
				snprintf(request, BUFFER_SIZE, "STATUS");
				send_request(msgid, request);
				break;
			case 4:
				exit(0);
			default:
				printf("Invalid choice\n");
		}
	}

	return 0;
}
