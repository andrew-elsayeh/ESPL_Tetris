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

TaskHandle_t UDPControlTask = NULL;
SemaphoreHandle_t HandleUDP = NULL;


aIO_handle_t udp_soc_receive = NULL, udp_soc_transmit = NULL;



void UDPHandler(size_t read_size, char *buffer, void *args)
{
    BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken3 = pdFALSE;


    int buffer2;

    if (xSemaphoreTakeFromISR(HandleUDP, &xHigherPriorityTaskWoken1) ==
        pdTRUE) {
        if (strncmp(buffer, "NEXT=", 
                    (read_size < 5) ? read_size : 5) == 0) {
            if(read_size > 5) {
                switch (buffer[5]) {
                    case 'O': buffer2 = 0; break;
                    case 'I': buffer2 = 1; break;
                    case 'L': buffer2 = 2; break;
                    case 'J': buffer2 = 3; break;
                    case 'Z': buffer2 = 4; break;
                    case 'S': buffer2 = 5; break;
                    case 'T': buffer2 = 6; break;
                }

                xQueueSendFromISR(PieceQueue, &buffer2, &xHigherPriorityTaskWoken2);
            }
        }


        xSemaphoreGiveFromISR(HandleUDP, &xHigherPriorityTaskWoken3);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken1 |
                           xHigherPriorityTaskWoken2 |
                           xHigherPriorityTaskWoken3);
    }
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

void requestShape(){
    char buf[10];
    sprintf(buf, "NEXT");
    aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
}

void startMultiplayer()
{
    char *addr = NULL; // Loopback
    in_port_t port = UDP_RECEIVE_PORT;

    udp_soc_receive = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE, UDPHandler, NULL);

    HandleUDP = xSemaphoreCreateMutex();


    PieceQueue = xQueueCreate(10, sizeof(int));

    printf("UDP socket opened on port %d\n", port);
}