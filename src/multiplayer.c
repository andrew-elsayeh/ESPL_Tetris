#include "AsyncIO.h"
#include "queue.h"


/** AsyncIO related */
#define UDP_BUFFER_SIZE 1024
#define UDP_RECEIVE_PORT 1234
#define UDP_TRANSMIT_PORT 1235

TaskHandle_t UDPControlTask = NULL;


aIO_handle_t udp_soc_receive = NULL, udp_soc_transmit = NULL;

typedef enum { NONE = 0, INC = 1, DEC = -1 } opponent_cmd_t;

void UDPHandler(size_t read_size, char *buffer, void *args)
{
    opponent_cmd_t next_key = NONE;
    BaseType_t xHigherPriorityTaskWoken1 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken2 = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken3 = pdFALSE;

    if (xSemaphoreTakeFromISR(HandleUDP, &xHigherPriorityTaskWoken1) ==
        pdTRUE) {

        char send_command = 0;
        if (strncmp(buffer, "INC", (read_size < 3) ? read_size : 3) ==
            0) {
            next_key = INC;
            send_command = 1;
        }
        else if (strncmp(buffer, "DEC",
                         (read_size < 3) ? read_size : 3) == 0) {
            next_key = DEC;
            send_command = 1;
        }
        else if (strncmp(buffer, "NONE",
                         (read_size < 4) ? read_size : 4) == 0) {
            next_key = NONE;
            send_command = 1;
        }

        if (NextKeyQueue && send_command) {
            xQueueSendFromISR(NextKeyQueue, (void *)&next_key,
                              &xHigherPriorityTaskWoken2);
        }
        xSemaphoreGiveFromISR(HandleUDP, &xHigherPriorityTaskWoken3);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken1 |
                           xHigherPriorityTaskWoken2 |
                           xHigherPriorityTaskWoken3);
    }
    else {
        fprintf(stderr, "[ERROR] Overlapping UDPHandler call\n");
    }
}

void vUDPControlTask(void *pvParameters)
{
    static char buf[50];            //Buffer to store sth
    char *addr = NULL;              //Address to something
    in_port_t port = UDP_RECEIVE_PORT;  //the UDP Recieve port
    unsigned int ball_y = 0;    
    unsigned int paddle_y = 0;
    char last_difficulty = -1;
    char difficulty = 1;

    udp_soc_receive =
        aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE, UDPHandler, NULL);

    printf("UDP socket opened on port %d\n", port);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(15));
        //Empty Queues?
        while (xQueueReceive(BallYQueue, &ball_y, 0) == pdTRUE) {
        }
        while (xQueueReceive(PaddleYQueue, &paddle_y, 0) == pdTRUE) {
        }
        while (xQueueReceive(DifficultyQueue, &difficulty, 0) == pdTRUE) {
        }

        //Calcuate some number and cast it onto a string
        signed int diff = ball_y - paddle_y;
        if (diff > 0) {
            sprintf(buf, "+%d", diff);
        }
        else {
            sprintf(buf, "-%d", -diff);
        }

        //Send this string over
        aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf));
        
        if (last_difficulty != difficulty) {                            //If difficulty changes

            sprintf(buf, "D%d", difficulty + 1);                         //write command to send 
            aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf, strlen(buf)); //Send it away
            last_difficulty = difficulty;
        }
    }
}