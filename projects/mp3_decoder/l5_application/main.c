#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "ff.h"
#include "isr.h"
#include "mp3decoder.h"
#include "oled.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "song_handler.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

typedef struct {
  char Tag[3];
  char Title[30];
  char Artist[30];
  char Album[30];
  char Year[4];
  uint8_t genre;
} mp3_meta_data;
/* -------------------------------------------------------------------------- */
/*                          SECTION global variables                          */
/* -------------------------------------------------------------------------- */
volatile bool metamp3 = true;
volatile uint8_t volume_level = 4;
char *song_name_without_dot_mp3;
char list_song_without_mp3[32][128];
uint16_t cursor_main = 0;
volatile bool play_pause = true;
/* -------------------------------------------------------------------------- */
/*                            SECTION global Queue                            */
/* -------------------------------------------------------------------------- */
QueueHandle_t music_Q;
QueueHandle_t songinfo_Q;

/* -------------------------------------------------------------------------- */
/*                          SECTION global Semaphore                          */
/* -------------------------------------------------------------------------- */
SemaphoreHandle_t pause;
SemaphoreHandle_t next;
SemaphoreHandle_t previous;
SemaphoreHandle_t vol_up;
SemaphoreHandle_t vol_down;
SemaphoreHandle_t move_up;
SemaphoreHandle_t move_down;
/* -------------------------------------------------------------------------- */
/*                          SECTION Declaration of function                   */
/* -------------------------------------------------------------------------- */
void reader_task();
void player_task();
void pause_task();
void pause_test();
void get_current_playing_song_name();
void next_song_task();
void previous_song_task();
void read_meta(char *byte_128);
void volume_up_task();
void volume_down_task();
void display_current_volume();
void display_list_of_song();
void move_down_task();
void clear_number_of_page(uint8_t number_of_page);
void populate_song_no_mp3();
/* -------------------------------------------------------------------------- */
/*                     SECTION INTERRUPT SERVICE ROUNTINE                     */
/* -------------------------------------------------------------------------- */

void pause_isr(void);
void next_song_isr(void);
void volume_control_isr(void);
void move_down_isr(void);

/* -------------------------------------------------------------------------- */
/*                            SECTION TASKHANDLE                              */
/* -------------------------------------------------------------------------- */

TaskHandle_t player_handle;
/* -------------------------------------------------------------------------- */
/*                                SECTION MAIN                                */
/* -------------------------------------------------------------------------- */

void main() {

  /* ---------------------- Initialization and set up pins --------------------- */
  populate_list_song();
  gpio1__set_as_input(1, 15);
  gpio1__set_as_input(1, 19);
  gpio__construct_with_function(0, 6, GPIO__FUNCITON_0_IO_PIN);
  gpio__construct_with_function(0, 8, GPIO__FUNCITON_0_IO_PIN);

  fprintf(stderr, "total song: %d \n", total_of_songs());
  mp3_setup();
  oled_print("TIM MP3 PLAYER", 2, init);

  zoom_in_mode();
  horizontal_scrolling(2, 2);
  // blinking_mode();
  delay__ms(6000);
  disable_zoom_in_mode();
  // disable_blinking_mode();
  white_Out(3, 1);
  oled_clear();
  oled_update();
  populate_song_no_mp3();
  display_list_of_song();
  sj2_cli__init();

  /* -------------------------------- Queue -------------------------------*/
  music_Q = xQueueCreate(1, sizeof(trackname_t));
  songinfo_Q = xQueueCreate(1, 512);

  /* ------------------------------ Semaphore -----------------------------*/
  pause = xSemaphoreCreateBinary();
  next = xSemaphoreCreateBinary();
  previous = xSemaphoreCreateBinary();
  vol_up = xSemaphoreCreateBinary();
  vol_down = xSemaphoreCreateBinary();
  move_up = xSemaphoreCreateBinary();
  move_down = xSemaphoreCreateBinary();
  /* -------------------------------- Interrupt ------------------------------- */
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "isr in port 0");
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, pause_isr);
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, next_song_isr);
  gpio0__attach_interrupt(6, GPIO_INTR__FALLING_EDGE, volume_control_isr);
  gpio0__attach_interrupt(8, GPIO_INTR__FALLING_EDGE, move_down_isr);

  /* ------------------------------ Task creation ----------------------------- */

  xTaskCreate(reader_task, "reader", (2024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(player_task, "player", (3096 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, &player_handle);
  xTaskCreate(pause_task, "pause", (1024 * 4) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(previous_song_task, "previous_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(next_song_task, "next_song", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(volume_up_task, "volume up", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(volume_down_task, "volume down", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(move_down_task, "move down", (1024 * 2) / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  vTaskStartScheduler();
}
/* -------------------------------------------------------------------------- */
/*                      INTERRUPT SERVICE ROUTINE SECTION                     */
/* -------------------------------------------------------------------------- */

void pause_isr() { xSemaphoreGiveFromISR(pause, NULL); }
void next_song_isr() { xSemaphoreGiveFromISR(next, NULL); }
void volume_control_isr() { xSemaphoreGiveFromISR(vol_up, NULL); }
void move_down_isr() { xSemaphoreGiveFromISR(move_down, NULL); }

/* -------------------------------------------------------------------------- */
/*                           SECTION DEFINITION FUNC                          */
/* -------------------------------------------------------------------------- */

void reader_task() {

  trackname_t song_name;
  uint8_t byte_512[512];
  UINT br; // binary
  while (1) {
    if (xQueueReceive(music_Q, song_name, portMAX_DELAY)) {
      white_Out(OLED__PAGE0, single_page);
      oled_print("Now Playing:", OLED__PAGE0, ninit);
      /* -------------------------------- OPEN FILE ------------------------------- */
      const char *filename = song_name;
      FIL file; // create object file
      FRESULT result = f_open(&file, filename, (FA_READ));

      /* ----------------------------- READ META_DATA ----------------------------- */
      char byte_128[128];
      f_lseek(&file, f_size(&file) - (sizeof(char) * 128));
      f_read(&file, byte_128, sizeof(byte_128), &br);
      read_meta(byte_128);
      f_lseek(&file, 0);
      /* ----------------------------- READ SONG_DATA ----------------------------- */
      if (FR_OK == result) {
        f_read(&file, byte_512, sizeof(byte_512), &br);
        while (br != 0) {
          f_read(&file, byte_512, sizeof(byte_512), &br);
          xQueueSend(songinfo_Q, byte_512, portMAX_DELAY);
          if (uxQueueMessagesWaiting(music_Q)) {
            printf("New play song request\n");
            break;
          }
        }
        /* --------------------------- Auto play next song -------------------------- */
        if (br == 0) {
          metamp3 = true; // read meta
          xSemaphoreGive(next);
        }
        f_close(&file);
      } else {
        fprintf(stderr, "Failed to open the file");
      }
    }
  }
}

void pause_task() {
  while (1) {
    if (xSemaphoreTake(pause, portMAX_DELAY)) {
      if (play_pause) {
        white_Out(OLED__PAGE0, single_page);
        oled_print("Paused", OLED__PAGE0, ninit);
        vTaskSuspend(player_handle);
        deactivate_horizontal_scrolling();
        play_pause = false;
      } else {
        white_Out(OLED__PAGE0, single_page);
        oled_print("Now Playing:", OLED__PAGE0, ninit);
        vTaskResume(player_handle);
        horizontal_scrolling(OLED__PAGE1, OLED__PAGE1);
        play_pause = true;
      }
    }
  }
}

void player_task() {
  while (1) {
    uint8_t byte_512[512];
    xQueueReceive(songinfo_Q, byte_512, portMAX_DELAY);
    for (int i = 0; i < 512; i++) {
      while (!get_DREQ_HighActive()) {
        vTaskDelay(1); // waiting for DREQ
      }
      mp3_write_sdi(byte_512[i]);
    }
  }
}
volatile bool debounce = true;
void next_song_task() {
  while (1) {
    if (xSemaphoreTake(next, portMAX_DELAY)) {
      if (!gpio1__get_level(1, 15)) {
        metamp3 = true;
        int total = total_of_songs();
        if (cursor_main == total) {
          cursor_main = 0;
        }
        char *song = get_songs_name(cursor_main);
        cursor_main++;
        xQueueSend(music_Q, song, portMAX_DELAY);

        white_Out(OLED__PAGE0, single_page);
        oled_print("Now Playing:", OLED__PAGE0, ninit);
      } else {
        white_Out(OLED__PAGE0, single_page);
        oled_print("Now Playing:", OLED__PAGE0, ninit);
        xSemaphoreGive(previous);
      }
      vTaskDelay(1000);
    }
  }
}

void previous_song_task() {
  while (1) {
    if (xSemaphoreTake(previous, portMAX_DELAY)) {
      metamp3 = true;
      int total = total_of_songs();
      if (cursor_main == 0) {
        cursor_main = total;
      }
      cursor_main--;
      char *song = get_songs_name(cursor_main);
      white_Out(OLED__PAGE0, single_page);
      oled_print("Now Playing:", OLED__PAGE0, ninit);

      xQueueSend(music_Q, song, portMAX_DELAY);
    }
    vTaskDelay(100);
  }
}

void volume_up_task() {
  while (1) {
    if (xSemaphoreTake(vol_up, portMAX_DELAY)) {
      if (!gpio1__get_level(1, 15)) {
        volume_level++;
        if (volume_level == 10) {
          volume_level = 0;
        }
        mp3_adjust_vol(volume_level);
        display_current_volume();
        horizontal_scrolling(OLED__PAGE1, OLED__PAGE1);
      } else {
        xSemaphoreGive(vol_down);
      }
    }
  }
}

void volume_down_task() {
  while (1) {
    if (xSemaphoreTake(vol_down, portMAX_DELAY)) {
      volume_level--;
      if (volume_level == 0) {
        volume_level = 9;
      }
      mp3_adjust_vol(volume_level);
      display_current_volume();
      horizontal_scrolling(OLED__PAGE1, OLED__PAGE1);
    }
  }
}

uint8_t cursor_for_scrolling = 0;
void move_down_task() {
  while (1) {
    if (xSemaphoreTake(move_down, portMAX_DELAY)) {
      // deactivate_horizontal_scrolling();
      if (cursor_for_scrolling == 8) {
        cursor_for_scrolling = 0;
        white_Out(0, all_pages);
        display_list_of_song();
      }
      // blinking_mode();
      if (cursor_for_scrolling != 0) {
        deactivate_horizontal_scrolling();
        white_Out(cursor_for_scrolling - 1, single_page);
        oled_print(list_song_without_mp3[cursor_main - 1], cursor_for_scrolling - 1, ninit);
      }
      // white_Out(0, all_pages);
      // display_list_of_song();
      horizontal_scrolling(cursor_for_scrolling, cursor_for_scrolling);
      if (gpio1__get_level(1, 19)) {
        /**
         * cause this cursor will be move down again when we press the button combination
         * a hacky way is to deduct by 1 to move back to our intended location
         * */
        // cursor_main--;
        char *song = get_songs_name(cursor_main - 1);
        // cursor_main = cursor_for_scrolling;
        xQueueSend(music_Q, song, portMAX_DELAY);
      }

      if (gpio1__get_level(1, 15)) {
        cursor_main = 0;
        cursor_for_scrolling = 0;
        clear_number_of_page(8);
        display_list_of_song();
      }
      /* -------------------------------------------------------------------------- */
      cursor_for_scrolling++;
      cursor_main++;
    }
  }
}
/* -------------------------------------------------------------------------- */
/*                               SECTION UTILITY FUNCTIONS                    */
/* -------------------------------------------------------------------------- */

void get_current_playing_song_name() {
  white_Out(OLED__PAGE7, single_page);
  char *song = get_songs_name(cursor_main);
  song_name_without_dot_mp3 = remove_dot_mp3(song);
  oled_print(song_name_without_dot_mp3, OLED__PAGE7, ninit);
}

void read_meta(char *byte_128) {
  white_Out(OLED__PAGE1, single_page);
  white_Out(OLED__PAGE2, single_page);
  white_Out(OLED__PAGE3, single_page);
  white_Out(OLED__PAGE4, single_page);
  white_Out(OLED__PAGE5, single_page);
  white_Out(OLED__PAGE6, single_page);
  white_Out(OLED__PAGE7, single_page);
  mp3_meta_data meta_data_mp3 = {0};
  for (int i = 0; i < 128; i++) {
    if ((((int)(byte_128[i]) > 47) && ((int)(byte_128[i]) < 58)) ||
        (((int)(byte_128[i]) > 64) && ((int)(byte_128[i]) < 91)) ||
        (((int)(byte_128[i]) > 96) && ((int)(byte_128[i]) < 123)) || ((int)(byte_128[i])) == 32) {
      char c = (int)(byte_128[i]);
      if (i < 3) {
        meta_data_mp3.Tag[i] = c;
      } else if (i > 2 && i < 33) {
        meta_data_mp3.Title[i - 3] = c;
      } else if (i > 32 && i < 63) {
        meta_data_mp3.Artist[i - 33] = c;
      } else if (i > 62 && i < 93) {
        meta_data_mp3.Album[i - 63] = c;
      } else if (i > 92 && i < 97) {
        meta_data_mp3.Year[i - 93] = c;
      } else if (i == 127) {
        meta_data_mp3.genre = (int)(byte_128[i]);
      }
    }
  }
  oled_print(meta_data_mp3.Title, OLED__PAGE1, ninit);
  oled_print(meta_data_mp3.Artist, OLED__PAGE2, ninit);
  oled_print(genre_decoder(meta_data_mp3.genre), OLED__PAGE3, ninit);
  oled_print(meta_data_mp3.Year, OLED__PAGE4, ninit);
  horizontal_scrolling(OLED__PAGE1, OLED__PAGE1);
}

void display_current_volume() {
  char smallest_square = (char)128;
  char small_square = (char)129;
  char medium_square = (char)130;
  char full_square = (char)131;
  white_Out(OLED__PAGE7, single_page);
  oled_print("Vol: ", OLED__PAGE7, ninit);
  for (int i = 0; i < volume_level; i++) {
    oled_CS();
    gpio1__set_low(1, 25);
    gpio1__set_high(1, 25);
    if (i < 3) {
      char_smallest_square();
    } else if (i < 5) {
      char_small_square();
    } else if (i < 7) {
      char_medium_square();
    } else {
      char_full_square();
    }
    oled_DS();
  }
}

void display_list_of_song() {
  uint8_t oled_page_counter = 0;
  for (int i = cursor_main; i < cursor_main + 8; i++) {
    if (i == total_of_songs()) {
      break;
    }
    // sprintf(list_song_without_mp3[i], strcat(" ", list_song_without_mp3[i]));
    char *song = list_song_without_mp3[i];
    // song_name_without_dot_mp3 = remove_dot_mp3(song);
    oled_print(song, oled_page_counter, ninit);
    oled_page_counter++;
  }
}

void populate_song_no_mp3() {
  for (int i = 0; i < total_of_songs(); i++) {
    char *song = get_songs_name(i);
    song_name_without_dot_mp3 = remove_dot_mp3(song);
    strncpy(list_song_without_mp3[i], song_name_without_dot_mp3, 127);
  }
}

void clear_number_of_page(uint8_t number_of_page) {
  for (int i = 0; i < number_of_page; i++) {
    white_Out(i, single_page);
  }
}
