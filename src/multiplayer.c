#include "AsyncIO.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** AsyncIO related */
#define UDP_BUFFER_SIZE 1024
#define UDP_RECEIVE_PORT 1234
#define UDP_TRANSMIT_PORT 1235




QueueHandle_t PieceQueue = NULL;
QueueHandle_t ModeQueue = NULL;


SemaphoreHandle_t HandleUDP = NULL;

aIO_handle_t udp_soc_receive = NULL, udp_soc_transmit = NULL;



/*
 * When something is recieved over UDP, this ISR is called. It only reacts to Next because 
 * The game doesn't need to react on the others. 
 */
void UDPHandler(size_t read_size, char *buffer, void *args)
{
    BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken3 = pdFALSE;


    int PieceReceived;
    int GotMode;

    if (xSemaphoreTakeFromISR(HandleUDP, &xHigherPriorityTaskWoken1) == pdTRUE) 
    {
        
        if      (strncmp(buffer, "NEXT=O",
         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 0;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=I",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 1;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=L",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 2;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=J",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 3;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=Z",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 4;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=S",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 5;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "NEXT=T",
                         (read_size < 6) ? read_size : 6) == 0) {
            PieceReceived = 6;
            xQueueSendFromISR(PieceQueue, &PieceReceived, &xHigherPriorityTaskWoken2);
        }
        else if (strncmp(buffer, "MODE=",
                         (read_size < 5) ? read_size : 5) == 0) {
            GotMode = 1;
            xQueueSendFromISR(ModeQueue, &GotMode, &xHigherPriorityTaskWoken2);
        }
        

        xSemaphoreGiveFromISR(HandleUDP, &xHigherPriorityTaskWoken3);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken1 |
                            xHigherPriorityTaskWoken2 |
                            xHigherPriorityTaskWoken3);
    }
}

void resetMultiplayer()
{
    char buf[10];
    sprintf(buf, "RESET");
    aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
}

void endMultiplayer()
{
    aIOCloseConn(udp_soc_receive);
}


void startMultiplayer()
{
    char *addr = NULL; // Loopback
    in_port_t port = UDP_RECEIVE_PORT;

    udp_soc_receive = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE, UDPHandler, NULL);

    HandleUDP = xSemaphoreCreateMutex();
    PieceQueue = xQueueCreate(10, sizeof(int));
    ModeQueue = xQueueCreate(1, sizeof(int));

    printf("UDP socket opened on port %d\n", port);
}

void requestShape(){
    char buf[10];
    sprintf(buf, "NEXT");
    aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
}



void setMode(int Algorithm)
{
    char* mode;
    switch (Algorithm)
    {
    case 0:
        mode = "FAIR";
        break;
    case 1:
        mode = "RANDOM";
        break;
    case 2:
        mode = "EASY";
        break;
    case 3:
        mode = "HARD";
        break;
    case 4:
        mode = "DETERMINISTIC";
        break;
    default:
        break;
    }
    char buf[20];
    sprintf(buf, "MODE=%s", mode);
    aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
}

void requestMode(){
    char buf[10];
    sprintf(buf, "MODE");
    aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
}

void assertBinaryRunning(){
    requestMode();
    int mode;
    if(xQueueReceive(ModeQueue, &mode, 1000) == pdFALSE)
    {
        printf("Please run the Tetris Generator Binary for the Multiplayer Mode to work!\n");
        exit(EXIT_SUCCESS);
    }
}

/**
 * Requests a piece from the binary, waits for an answer and returns it
 */
int getPiecefromUDB()
{
    int pieceNum;
    requestShape();
    xQueueReceive(PieceQueue, &pieceNum, portMAX_DELAY);
    return pieceNum;
}


