#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAXBUFFSIZE 1024

#define MSGQKEY 777

/* Must be follow this struct! */
typedef struct
{
	long lMessageQueueType;
	char acMessageBuffer[MAXBUFFSIZE];
}TMessageQueueInfo;

void ErrorMessage(char *pcMessageBuffer)
{
	perror(pcMessageBuffer);
}

void *ReadMessageQueue(void)
{
	int iMessageQueueID; 	/* Message Queue ID */
	key_t tMessageQueueKey; /* Message Queue Key */
	TMessageQueueInfo tMessageQueueInfo; /* Message Queue information */
	
	/* Message Queue R/W permission */
	int iMessageQueueFlagRW = IPC_CREAT | 0666;

	/* Setting the message queue type */
	tMessageQueueInfo.lMessageQueueType = 1;

	/* Setting the message queue key, must be the same as main thread's
	   message queue key */	
	tMessageQueueKey = MSGQKEY;

	/* Get the message queue ID */
	if ((iMessageQueueID = msgget(tMessageQueueKey, iMessageQueueFlagRW)) < 0)
	{
		ErrorMessage("Failed to get	message queue ID");
		pthread_exit(NULL);
	}

	fprintf(stdout, "Wiat the message\n");

	while (1)
	{
		/* Receive the tMessageQueueInfo data from type 1 */
		if (msgrcv(iMessageQueueID, &tMessageQueueInfo, MAXBUFFSIZE,
				   tMessageQueueInfo.lMessageQueueType, /*IPC_NOWAIT*/0) < 0)
		{
			ErrorMessage("Failed to receive data from message queue");
			//pthread_exit(NULL);
		}

		fprintf(stdout, "******************\nReceive:\n%s******************\n",
				tMessageQueueInfo.acMessageBuffer);
	}
		
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	pthread_t tReadMessageQueue; /* Read MessageQueue thread ID */
	
	int iMessageQueueID; 	/* Message Queue ID */
	key_t tMessageQueueKey; /* Message Queue Key */
	unsigned int uiMessageBufferLen; /* Message buffer lenth */

	/* Message Queue R/W permission */
	int iMessageQueueFlagRW = IPC_CREAT | 0666;
	TMessageQueueInfo tMessageQueueInfo; /* Message Queue information */
	
	/* Get the message queue Key */
	tMessageQueueKey = MSGQKEY;
	
	/* Get the message queue ID */
	if ((iMessageQueueID = msgget(tMessageQueueKey,
								  iMessageQueueFlagRW)) < 0)
	{
		ErrorMessage("Failed to get message queue ID");
		return -1;
	}

	/* Creat the ReadMessageQueue thread */
	pthread_create(&tReadMessageQueue, NULL, (void*)ReadMessageQueue, NULL);

	fprintf(stdout, "Writing some message\n");

	while (1)
	{
		/* Setting the message type is 1 */
		tMessageQueueInfo.lMessageQueueType = 1;

		if (fgets(tMessageQueueInfo.acMessageBuffer, MAXBUFFSIZE, stdin) == NULL)
		{
			ErrorMessage("Failed to get input string");
		}

		uiMessageBufferLen = strlen(tMessageQueueInfo.acMessageBuffer);

		/* Send the message by message queue */
		/* uiMessageBufferLen + 1, let '\0' also sent to other programs */
		if (msgsnd(iMessageQueueID, &tMessageQueueInfo, uiMessageBufferLen + 1, IPC_NOWAIT) < 0)
		{
			ErrorMessage("Failed to send by message queue");
			return -1;
		}

		fprintf(stdout, "Send OK\n");
	}
	
	pthread_join(tReadMessageQueue, NULL);

	return 0;
}
