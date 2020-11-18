#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  OLED__PAGE0 = 0x00,
  OLED__PAGE1 = 0x01,
  OLED__PAGE2 = 0x02,
  OLED__PAGE3 = 0x03,
  OLED__PAGE4 = 0x04,
  OLED__PAGE5 = 0x05,
  OLED__PAGE6 = 0x06,
  OLED__PAGE7 = 0x07,
} oled_page;

typedef enum {
  OLED__COLUMN0 = 0x00,
  OLED__COLUMN1 = 0x01,
  OLED__COLUMN2 = 0x02,
  OLED__COLUMN3 = 0x03,
  OLED__COLUMN4 = 0x04,
  OLED__COLUMN5 = 0x05,
  OLED__COLUMN6 = 0x06,
  OLED__COLUMN7 = 0x07,
  OLED__COLUMN8 = 0x08,
  OLED__COLUMN9 = 0x09,
  OLED__COLUMN10 = 0x0A,
  OLED__COLUMN11 = 0x0B,
  OLED__COLUMN12 = 0x0C,
  OLED__COLUMN13 = 0x0D,
  OLED__COLUMN14 = 0x0E,
  OLED__COLUMN15 = 0x0F,
} oled_column;

typedef enum {
  page_0,
  page_1,
  page_2,
  page_3,
  page_4,
  page_5,
  page_6,
  page_7,
} page_start;

typedef enum {
  ninit,
  init,
} lines;

typedef enum { single_page = 0, all_pages } white_out_page;
/* -------------------------------------------------------------------------- */
/* ------------------------- Declaration + Power on ------------------------- */

/* Oled Pixel for single Page (8)H x (128)W*/
uint8_t bitmap_[8][128];

/* Output [P1_22] ( ON<--(LOW)|(HIGH) -->OFF ) */
void oled_CS();
void oled_DS();

/* SSP1_I/O Function PIN P0_7 | P0_9 | P1_25 */
void config_oled_pin();

/* Command Buss P1_25(LOW) | Data Buss P1_25(HIGH) */
void oled_setC_bus();
void oled_setD_bus();

/* -------------------------------------------------------------------------- */
/* ------------------------ Initialization + Testing ------------------------ */

/* SPI_oled Initial */
void SPI_oled_initialization();

/* Oled (SPI1) Transfer Byte  */
void oled__transfer_byte(uint8_t data_transfer);

/* Initialize the Sequence of OP-CODE for OLED */
void panel_init();

/* Test <-> Turn LCD ON  --> Print ("CMPE") */
void turn_on_lcd();

/* -------------------------------------------------------------------------- */
/* ---------------------------- Set page + column --------------------------- */

void set_column_start(oled_column column_number);
void set_page_start(oled_page page_number_oled);
/* -------------------------------------------------------------------------- */
/* ----------------------- Clear + Fill Update Screen ----------------------- */

/* Set Bit Map with 0x00  */
void oled_clear();

/* Set Bit Map with 0xFF  */
void oled_fill();

/* Update BitMap to Oled */
void oled_update();

/* -------------------------------------------------------------------------- */
/* ------------------------------ Control Usage ----------------------------- */

/* Horizontal Address Mode */
void horizontal_addr_mode();

/*Scolling in Horizontal*/
void horizontal_scrolling(page_start begin_page, page_start end_page);

/* ------------- Generate new line when it finish the paragraph ------------- */

void new_line(uint8_t line_address);

/*------------------------------Print onto lcd--------------------------------*/
void oled_print(char *message, page_start begin_page, lines init_verify);

/* ---------------------------- White out screen ---------------------------- */
void white_Out(page_start page_num, white_out_page single_or_all);

/* -------------------------- Deactivate Scrolling -------------------------- */

void deactivate_horizontal_scrolling();

/* -------------------------------------------------------------------------- */
/* --------------------------- LOOK UP char Array --------------------------- */

/* Call Back Array for char */
typedef void (*function_pointer_char)(void);

/*
 * Casting to get the ASCII value of the char
 * ---> Assign ASCII value to index of Call Back Array
 */
void char_array_table();

/* Use Lookup Table to search char and Display */
void display_char(char *string);
void display(char *str);
/* ----------------------------- Covert to Pixel ---------------------------- */

void char_A();
void char_B();
void char_C();
void char_D();
void char_E();
void char_F();
void char_G();
void char_H();
void char_I();
void char_J();
void char_K();
void char_L();
void char_M();
void char_N();
void char_O();
void char_P();
void char_Q();
void char_R();
void char_S();
void char_T();
void char_U();
void char_V();
void char_W();
void char_X();
void char_Y();
void char_Z();
/*-----------------------------------------------------------*/
void char_a();
void char_b();
void char_c();
void char_d();
void char_e();
void char_f();
void char_g();
void char_h();
void char_i();
void char_j();
void char_k();
void char_l();
void char_m();
void char_n();
void char_o();
void char_p();
void char_q();
void char_r();
void char_s();
void char_t();
void char_u();
void char_v();
void char_w();
void char_x();
void char_y();
void char_z();

/* --------------------------------- Number --------------------------------- */
void char_0();
void char_1();
void char_2();
void char_3();
void char_4();
void char_5();
void char_6();
void char_7();
void char_8();
void char_9();

/* ------------------------------ Special Char ------------------------------ */
void char_dquote();
void char_squote();
void char_comma();
void char_qmark();
void char_excl();
void char_at();
void char_undersc();
void char_star();
void char_hash();
void char_percent();
void char_amper();
void char_parenthL();
void char_parenthR();
void char_plus();
void char_minus();
void char_div();
void char_colon();
void char_scolon();
void char_less();
void char_greater();
void char_equal();
void char_bracketL();
void char_backslash();
void char_bracketR();
void char_caret();
void char_bquote();
void char_braceL();
void char_braceR();
void char_bar();
void char_tilde();
void char_space();
void char_period();
void char_dollar();
void char_smallest_square();
void char_small_square();
void char_medium_square();
void char_full_square();