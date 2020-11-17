#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "ff.h"
#include "mp3decoder.h"
#include "oled.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "song_handler.h"
#include "task.h"
#include <stdio.h>

/***************************************** MAIN LOOP *********************************************
**************************************************************************************************/
void main(void) { mp3_setup(); }