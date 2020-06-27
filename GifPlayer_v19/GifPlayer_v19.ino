/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include "Ticker.h"
#include <Adafruit_Arcada.h>
#include "GifDecoder.h"
#include <ArduinoJson.h>
#include <SdFat.h>
/*********************
 *      DEFINES
 *********************/
#define CANVAS_WIDTH    130
#define CANVAS_HEIGHT   130

#define LVGL_TICK_PERIOD 20

// BUTTONS [ARCADA]

  #define KEY_RIGHT             ARCADA_BUTTONMASK_RIGHT                                  // Link LVGL Key definitions to Arcada buttons from ItsyBitsy header file
  #define KEY_LEFT              ARCADA_BUTTONMASK_LEFT
  #define KEY_UP                ARCADA_BUTTONMASK_UP
  #define KEY_DOWN              ARCADA_BUTTONMASK_DOWN
  #define KEY_ESC               ARCADA_BUTTONMASK_B
  #define KEY_ENTER             ARCADA_BUTTONMASK_A

LV_IMG_DECLARE(blox_gif_bg)                                                             // LVGL image file declarations :: C files [stored in second level LVGL src folder
LV_IMG_DECLARE(blox_games_bg)
LV_IMG_DECLARE(blox_emoji_bg)
LV_IMG_DECLARE(blox_settings_bg)
LV_IMG_DECLARE(color_tri_manichi_vGR)
  
/**********************
 *      TYPEDEFS
 **********************/
 
/* Adafruit Arcada [Buttons + TFT]*/
Adafruit_Arcada arcada;                                                                   // register variable for Arcada library pipleine
uint8_t buttons;                                                                          // Arcada buttons :: included from ItsyBitsy header file || GLOBAL variable
GifDecoder<ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT, 12> decoder;                              // GifDecoder inherits w x h from Arcada TFT :: 240x240
int backlite;                                                                             // TFT backlight variable

/* Filesystem using SDFAT */
File file;                                                                                // file as variable :: SDFAT library used in Arcada pipeline

/**********************
 *  GIF Player INIT 
 **********************/
int gif_player_begin;                                                                     // Launch Gif player app :: 1 ON | 0 OFF
int16_t  gif_offset_x, gif_offset_y;                                                      // x y coordinates                                                     
void gif_player();                                                                        // gif player funcion call
uint16_t y;                                                                               // RANDOMISER ON/OFF [1 IS ON // 2 IS OFF] -- SET VIA MENU SELECTION // JSON CONFIG SAVED
int x;                                                                                    // INT X is delcared as global variable so it can be used as DIRECTORY argument
const char * DIRECTORY[] = {"/gifs", "/favs", "/grmod", "/emojis", "/favourites"};        // DIRECTORY as an ARRAY stores PATH VARIABLES
uint16_t randNumber;                                                                      // random number :: if set, randomises order Gifs are played based on total num of Gifs
uint16_t displayTimeSeconds = 10;                                                         // default Gif display time if no JSON config file contains one
int mode_select = 2;                                                                      // default mode select :: mode 2 == UP / DOWN GESTURES ONLY UNLESS IN EMOJI MODE                                            
int gif_direction;                                                                        // GIF play direction :: 0 == A-Z | 1 == Z-A | Default A-Z
int play_all;                                                                             // Menu :: Showcase Mode :: play_all == 1 [ALL DIRECTRORIES] // default is 0
int pause;                                                                                // pauses Gif
int gif_init = 0;                                                                         // set to first directory on first run after startup - called once
int app_num = -1;                                                                         // tracks folders and calls directory [ x = 0 | 1 | 2 | 3 ]
int last_folder;                                                                          // keeps track of folder number so lvgl group callback highlights last pressed btn

int poll_num = 0;
int dir_num = 0;
int active_dir1;
int active_dir2;
int active_dir3;
int active_dir4;
int base_dir = -1;
int last_dir = 0;

/**********************
 *  JSON 
 **********************/
 // JSON CONFIG STUFF - GENERATES CONFIG FILE IF ONE DOESN'T EXIST
struct Config {
  int _volume;
  int _brightness;
  int _seconds_per_gif;
  int _random;
};

const char *filename = "/arcada_config.json";                                          // JSON CONFIG FILE NAME // Generated if CONFIG file does not exist!
Config config;
StaticJsonDocument<105> configJSON;
SdFat SD;

/**********************
 *  STATIC PROTOTYPES
 **********************/
 
/*INPUT DEVICE :: KEYPAD [GESTURE]*/
void buttons_drv_reg(void);
static bool buttons_read(lv_indev_data_t * data);
static void my_style_mod_cb(lv_group_t* group, lv_style_t* style);
static void general_event_handler(lv_obj_t* obj, lv_event_t event);
/* GROUPS */

/* TEST */
void lv_test_UI(void);

/**********************
 *  STATIC VARIABLES
 **********************/
 
/* CORE :: TFT*/
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

/* store key states */
uint8_t last_key;
uint8_t last_state;

/* GROUPS */
static lv_group_t * g;

/* Keypad driver */
static lv_indev_t * emulated_kp_indev;

/* menu screen */
static lv_obj_t* scr;
static lv_obj_t* scr_gif;
static lv_obj_t* scr_games;
static lv_obj_t* scr_emoji;
static lv_obj_t* scr_settings;
static lv_obj_t* scr_gif_play_time;
static lv_obj_t* scr_random;
static lv_obj_t* scr_brightness;
static lv_obj_t* scr_battery;

/*background image*/
static lv_obj_t* main_bg;

/*main menu image canvas*/
lv_obj_t* canvas;

/* main menu image objects */
static lv_obj_t* icon1b;
static lv_obj_t* icon2b;
static lv_obj_t* icon3b;
static lv_obj_t* icon4b;

/*main menu buttons*/
static lv_obj_t* new_btn1;
static lv_obj_t* new_btn2;
static lv_obj_t* new_btn3;
static lv_obj_t* new_btn4;

/*main menu btn icons*/
lv_obj_t* label_icon1;
lv_obj_t* label_icon2;
lv_obj_t* label_icon3;
lv_obj_t* label_icon4;
lv_obj_t* label_icon5;

/*gif app preload container + page*/
static lv_style_t preload_style;
static lv_style_t opa_0_black;
static lv_style_t opa_1_black;

/*directory*/
lv_obj_t* folder_btn1;
lv_obj_t* folder_btn2;
lv_obj_t* folder_btn3;
lv_obj_t* folder_btn4;
lv_obj_t* folder_btn5;

/*settings menu list*/
lv_obj_t* settings_btn_list;

/*settings menu bttuons*/
lv_obj_t* settings_btn1;
lv_obj_t* settings_btn2;
lv_obj_t* settings_btn3;
lv_obj_t* settings_btn4;

/*gif play time menu page*/
lv_obj_t* gif_time_once_cb;
lv_obj_t* gif_time_10s_cb;
lv_obj_t* gif_time_30s_cb;
lv_obj_t* gif_time_1min_cb;
lv_obj_t* gif_time_4ever_cb;

/*gif shuffle menu page*/
/*switches*/
lv_obj_t* sw1;

/*LEDs*/
lv_obj_t* led1;
lv_obj_t* led2;
lv_obj_t* led3;
/*LED buttons*/
lv_obj_t* btn_led1;
lv_obj_t* btn_led2;
lv_obj_t* btn_led3;

/*button function calls*/
lv_obj_t* back_btn;
lv_obj_t* home_btn;

/*animtation LVGL*/
static lv_anim_t a;
static lv_anim_t a2;
static lv_anim_t a_time;
static lv_anim_t sa;
static lv_anim_t canvas_animation;

/*animated intro images style*/
static lv_style_t img_animation;
static lv_style_t img_opacity_0;
static lv_style_t img_opacity_255;
static lv_style_t bg_style;
static lv_style_t canvas_style;
static lv_style_t block_style;
static lv_style_t block_opa_000;
static lv_style_t block_opa_255;
static lv_style_t focus_style;
static lv_style_t defocus_style;
static lv_style_t info_tab_style;

/*variable logic counters*/
int menu_count = -2;
int icon_count = 0;
int sw_random = 0;
int menu_colour;
int gif_time_cb_tracker;
int LED_counter;
int icon_focus;
int last_icon;
int first_run;
int prev_icon;
/*LVGL object data*/
int16_t user_data;
/*LVGL Task handlers*/
lv_task_t* task;
/*task callbacks*/
static void my_task(lv_task_t* task);
static void anim_button_complete(lv_task_t* task);
/***********************************************************************************/
/***********************************************************************************/

    /*------------------
     * CORE :: DEBUG
     * -----------------*/
     
/* Serial debugging */ 

#if USE_LV_LOG != 0
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

    /*------------------
     * CORE :: TFT
     * -----------------*/
     
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  arcada.display->startWrite(); /* Start new TFT transaction */
  arcada.display->setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      arcada.display->writeColor(c, 1);
      color_p++;
    }
  }
  arcada.display->endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

    /*------------------
     * CORE :: TICKER
     * -----------------*/

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}

Ticker timer(lv_tick_handler, LVGL_TICK_PERIOD, 0, MILLIS); /* interrupt handler timer */ //MICROS or MILLIS

/***********************************************************************************/
/***********************************************************************************/

    /*------------------
     *   INPUT DEVICE
     * -----------------*/

void buttons_drv_reg(void)
{
    lv_indev_drv_t  kp_drv;
    lv_indev_drv_init(&kp_drv);
    kp_drv.type = LV_INDEV_TYPE_KEYPAD;
    kp_drv.read_cb = buttons_read;
    emulated_kp_indev = lv_indev_drv_register(&kp_drv);

    /*Create an object group*/
    g = lv_group_create();

    /*Assig the input device(s) to the created group*/
    lv_indev_set_group(emulated_kp_indev, g);
    lv_group_mod_style;
    lv_group_set_style_mod_cb(g, my_style_mod_cb);
}

/*------------------
 * Keypad
 * -----------------*/

/* Will be called by the library to read the keypad */
static bool buttons_read(lv_indev_drv_t * kp_drv, lv_indev_data_t * data)
{
    bnt_proccess(); // assign last_state and last_button variables

    data->key = last_key;
    data->state = last_state;
    printf("buttons_read:\n");
    printf("last_key: %d\n", last_key);
    printf("last_state: %d\n", last_state);
    return false;
}

/***********************************************************************************/
/* https://github.com/littlevgl/lvgl/issues/527 */ /* MichaelVLV */
    /*--------------------------
     *   Arcada Button READER
     * ------------------------*/
void bnt_proccess()
{ 
  arcada.readButtons();
  buttons = arcada.justPressedButtons();

  // DOWN gesture
  if(buttons & KEY_DOWN)
  {
    printf("DOWN");

    last_key = LV_KEY_NEXT;           // DOWN IS NEXT 
    last_state = LV_INDEV_STATE_PR;

    //buttons.state_DOWN = false;

    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  // UP gesture
  if(buttons & KEY_UP)
  {
    printf("UP");

    last_key = LV_KEY_PREV;           // UP IS PREVIOUS [STARTS AT TOP SO PREVIOUS MEANS MOVING UPWARDS]
    last_state = LV_INDEV_STATE_PR;

    //buttons.state_UP = false;
    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  // RIGHT gesture
  if(buttons & KEY_LEFT)
  {
    printf("buttons.state_ON == true \n");
    last_key = LV_KEY_LEFT;
    last_state = LV_INDEV_STATE_PR;

    //buttons.state_ON = false;
    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  // LEFT gesture
  if(buttons & KEY_RIGHT)
  {
    printf("buttons.state_ON == true \n");
    last_key = LV_KEY_RIGHT;
    last_state = LV_INDEV_STATE_PR;

    //buttons.state_ON = false;
    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  // ENTER button // short press
  if(buttons & KEY_ENTER)
  {
    last_key = LV_KEY_ENTER;
    last_state = LV_INDEV_STATE_PR;

    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  // ESC button // long Press
  if(buttons & KEY_ESC)
  {
    printf("buttons.state_BACK == true \n");

    last_key = LV_KEY_ESC;
    last_state = LV_INDEV_STATE_PR;
    //post_enter = 0;

    //buttons.state_BACK = false;
    goto exit;
  }
  else
  {
    last_state = LV_INDEV_STATE_REL;
  }

  exit: {/* do nothing */} ;
}
/***********************************************************************************/

    /*--------------------
     *     Animations
     * -------------------*/

void anim_button() {
    a.var = new_btn1;
    a.start = lv_obj_get_y(new_btn1);
    a.end = 10;
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
    a.path_cb = lv_anim_path_linear;
    a.ready_cb = NULL;
    a.act_time = -800; /*Delay the animation*/
    a.time = 600;
    a.playback = 0;
    a.playback_pause = 0;
    a.repeat = 0;
    a.repeat_pause = 0;
    lv_anim_create(&a);

    a.var = new_btn2;
    a.start = lv_obj_get_y(new_btn2);
    a.end = 66;
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
    a.path_cb = lv_anim_path_linear;
    a.ready_cb = NULL;
    a.act_time = -600; /*Delay the animation*/
    a.time = 600;
    a.playback = 0;
    a.playback_pause = 0;
    a.repeat = 0;
    a.repeat_pause = 0;
    lv_anim_create(&a);

    a.var = new_btn3;
    a.start = lv_obj_get_y(new_btn3);
    a.end = 122;
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
    a.path_cb = lv_anim_path_linear;
    a.ready_cb = NULL;
    a.act_time = -400; /*Delay the animation*/
    a.time = 600;
    a.playback = 0;
    a.playback_pause = 0;
    a.repeat = 0;
    a.repeat_pause = 0;
    lv_anim_create(&a);

    a.var = new_btn4;
    a.start = lv_obj_get_y(new_btn4);
    a.end = 178;
    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
    a.path_cb = lv_anim_path_linear;
    a.ready_cb = NULL;
    a.act_time = -200; /*Delay the animation*/
    a.time = 600;
    a.playback = 0;
    a.playback_pause = 0;
    a.repeat = 0;
    a.repeat_pause = 0;
    lv_anim_create(&a);

    /*animate canvas on starup*/
    lv_anim_t canvas_animation;
    lv_style_anim_init(&canvas_animation);
    lv_style_anim_set_styles(&canvas_animation, &canvas_style, &img_opacity_0, &img_opacity_255);
    lv_style_anim_set_time(&canvas_animation, 500, 1800);
    lv_style_anim_create(&canvas_animation);

    /*Animate icon image opacity style on startup*/
    lv_anim_t sa;
    lv_style_anim_init(&sa);
    lv_style_anim_set_styles(&sa, &img_animation, &img_opacity_0, &img_opacity_255);
    lv_style_anim_set_time(&sa, 500, 1800);
    lv_style_anim_create(&sa);

    //user_data = 1800;
    task = lv_task_create(anim_button_complete, 2800, LV_TASK_PRIO_MID, &user_data); //2800 //3600 with infotab
    lv_task_once(task);
}

void anim_button_complete(lv_task_t* task) {
    //int16_t* user_data = task->user_data;

    lv_group_add_obj(g, new_btn1);
    lv_group_add_obj(g, new_btn2);
    lv_group_add_obj(g, new_btn3);
    lv_group_add_obj(g, new_btn4);

    if (icon_focus == 0) {
        lv_obj_set_style(label_icon1, &focus_style);
        lv_obj_set_style(label_icon2, &defocus_style);
        lv_obj_set_style(label_icon3, &defocus_style);
        lv_obj_set_style(label_icon4, &defocus_style);
    }
}

void settings_animation() {
    
    /*transition from menu*/

    lv_group_remove_all_objs(g);

    if (first_run == 1) {
        if (last_icon == 1) {
            lv_obj_move_foreground(new_btn1);

            // extend button width
            a.var = new_btn1;
            a.start = lv_obj_get_width(new_btn1);
            a.end = lv_obj_get_width(new_btn1) + 160;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_width;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = -550; /*Delay the animation*/
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
        }
        if (last_icon == 2) {
            lv_obj_move_foreground(new_btn2);

            // extend button width
            a.var = new_btn2;
            a.start = lv_obj_get_width(new_btn2);
            a.end = lv_obj_get_width(new_btn2) + 160;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_width;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = -550; /*Delay the animation*/
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
        }
        if (last_icon == 3) {
            lv_obj_move_foreground(new_btn3);

            // extend button width
            a.var = new_btn3;
            a.start = lv_obj_get_width(new_btn3);
            a.end = lv_obj_get_width(new_btn3) + 160;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_width;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = -550; /*Delay the animation*/
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
        }
        if (last_icon == 4) {
            lv_obj_move_foreground(new_btn4);

            // extend button width
            a.var = new_btn4;
            a.start = lv_obj_get_width(new_btn4);
            a.end = lv_obj_get_width(new_btn4) + 160;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_width;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = -650; /*Delay the animation*/
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
        }

        /*animate canvas on button press*/
        lv_anim_t canvas_animation;
        lv_style_anim_init(&canvas_animation);
        lv_style_anim_set_styles(&canvas_animation, &canvas_style, &img_opacity_255, &img_opacity_0);
        lv_style_anim_set_time(&canvas_animation, 75, 900); //300, 65
        lv_style_anim_create(&canvas_animation);

        // animate image on button press
        lv_anim_t sa;
        lv_style_anim_init(&sa);
        lv_style_anim_set_styles(&sa, &img_animation, &img_opacity_255, &img_opacity_0);
        lv_style_anim_set_time(&sa, 75, 900); //500, 850
        lv_style_anim_create(&sa);

        // animate btn 2 to btn1 y pos
        a2.var = new_btn2;
        a2.start = lv_obj_get_y(new_btn2);
        a2.end = lv_obj_get_y(new_btn1);
        a2.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a2.path_cb = lv_anim_path_linear;
        a2.ready_cb = NULL;
        a2.act_time = -100; /*Delay the animation*/
        a2.time = 200;
        a2.playback = 0;
        a2.playback_pause = 0;
        a2.repeat = 0;
        a2.repeat_pause = 0;
        lv_anim_create(&a2);

        // animate btn 3 to btn1 y pos
        a.var = new_btn3;
        a.start = lv_obj_get_y(new_btn3);
        a.end = lv_obj_get_y(new_btn1);
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = NULL;
        a.act_time = -200; /*Delay the animation*/
        a.time = 200;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_create(&a);

        // animate btn 4 to btn1 y pos
        a.var = new_btn4;
        a.start = lv_obj_get_y(new_btn4);
        a.end = lv_obj_get_y(new_btn1);
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = NULL;
        a.act_time = -300; /*Delay the animation*/
        a.time = 200;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_create(&a);

        // create tasks
        task = lv_task_create(my_task, 900, LV_TASK_PRIO_MID, &user_data);
        lv_task_once(task);
    }

    if (first_run == 2) {
        if (last_icon == 1) {
            // animate button 1 off screen [bottom] after width animation
            a.var = new_btn1;
            a.start = lv_obj_get_y(new_btn1);
            a.end = lv_obj_get_y(new_btn1) + 240;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = 0; /*Delay the animation*/
            a.time = 125;   //250
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);

            /* //disabled for space
            // create preload screen container so can fade screen to black
            lv_obj_t* preloader_screen = lv_cont_create(lv_scr_act(), NULL);
            lv_obj_set_size(preloader_screen, 240, 240);

            // create preload screen colour style
            lv_style_copy(&preload_style, &lv_style_plain_color);
            preload_style.body.main_color = LV_COLOR_BLACK;
            preload_style.body.grad_color = LV_COLOR_BLACK;
            preload_style.body.opa = 0;

            lv_cont_set_style(preloader_screen, LV_CONT_STYLE_MAIN, &preload_style);

            lv_style_copy(&opa_0_black, &preload_style);
            lv_style_copy(&opa_1_black, &preload_style);

            opa_1_black.body.opa = 255;

            // animate opacity
            lv_anim_t sa;
            lv_style_anim_init(&sa);
            lv_style_anim_set_styles(&sa, &preload_style, &opa_0_black, &opa_1_black);
            lv_style_anim_set_time(&sa, 250, 0);
            lv_style_anim_create(&sa);*/

            // set button to top after container has been created
            lv_obj_move_foreground(new_btn1);

            // create task
            task = lv_task_create(my_task, 500, LV_TASK_PRIO_MID, &user_data);
            lv_task_once(task);
        }
        if (last_icon == 2) {
            // animate button 2 off screen [bottom] after width animation
            a.var = new_btn2;
            a.start = lv_obj_get_y(new_btn2);
            a.end = lv_obj_get_y(new_btn2) + 240;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = 0; /*Delay the animation*/
            a.time = 125;   //250
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);

            // create task
            task = lv_task_create(my_task, 500, LV_TASK_PRIO_MID, &user_data);
            lv_task_once(task);
        }
        if (last_icon == 3) {
            // animate button 2 off screen [bottom] after width animation
            a.var = new_btn3;
            a.start = lv_obj_get_y(new_btn3);
            a.end = lv_obj_get_y(new_btn3) + 240;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = 0; /*Delay the animation*/
            a.time = 125;   //250
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);

            // create task
            task = lv_task_create(my_task, 500, LV_TASK_PRIO_MID, &user_data);
            lv_task_once(task);
        }
        if (last_icon == 4) {
            // animate button 2 off screen [bottom] after width animation
            a.var = new_btn4;
            a.start = lv_obj_get_y(new_btn4);
            a.end = lv_obj_get_y(new_btn4) + 240;
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_ease_in_out;
            a.ready_cb = NULL;
            a.act_time = 0; /*Delay the animation*/
            a.time = 125;   //125
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);

            // create task
            task = lv_task_create(my_task, 500, LV_TASK_PRIO_MID, &user_data);
            lv_task_once(task);
        }
    }
}

/* my_task called periodacally - performs commands at set time :: set by above task timer*/
void my_task(lv_task_t* task)
{
    /*Use the user_data*/
    //int16_t* user_data = task->user_data;

    //printf("my_task called with user data: %d\n", *user_data);

    if (first_run == 2 && last_icon == 1) {
        if (lv_obj_get_y(new_btn1) > 10) {
            first_run = 0;                    //ready for new main menu call
            mode_select = 2;                  //showcase mode - up/down only
            play_all = 1;                     //enabled  
            menu_gif();                       //gif player app

            // should disable all gestures - - actionButton only - pause/exit
        }
    }

    if (first_run == 2 && last_icon == 2) {
        if (lv_obj_get_y(new_btn2) > 10) {
            first_run = 0;                    //ready for new main menu call
            menu_games();                     //folder directory
        }
    }

    if (first_run == 2 && last_icon == 3) {
        if (lv_obj_get_y(new_btn3) > 10) {
            first_run = 0;                    //ready for new main menu call  
            //menu_emoji();                   // dummy screen
            mode_select = 4;                  //emoji mode - all four gestures
            play_all = 0;                     //disabled
            menu_gif();                       //gif player app
        }
    }

    if (first_run == 2 && last_icon == 4) {
        if (lv_obj_get_y(new_btn4) > 10) {
            first_run = 0;                    //ready for new main menu call
            menu_settings();                  //settings menu
        }
    }

    if (first_run == 1) {

        // check if width of btn 1 has changed :: 1.4s
        if (lv_obj_get_width(new_btn1) == 220) {

            printf("btn 1 FINISHED ANIMATING\n");

            lv_obj_set_hidden(new_btn1, false);
            lv_obj_set_hidden(new_btn2, true);
            lv_obj_set_hidden(new_btn3, true);
            lv_obj_set_hidden(new_btn4, true);

            first_run = 2;
            settings_animation();
        }

        // check if width of btn 2 has changed :: 1.4s
        if (lv_obj_get_width(new_btn2) == 220) {

            printf("btn 2 FINISHED ANIMATING\n");

            lv_obj_set_hidden(new_btn1, true);
            lv_obj_set_hidden(new_btn2, false);
            lv_obj_set_hidden(new_btn3, true);
            lv_obj_set_hidden(new_btn4, true);

            first_run = 2;
            settings_animation();
        }

        // check if width of btn 3 has changed :: 1.4s
        if (lv_obj_get_width(new_btn3) == 220) {

            printf("btn 3 FINISHED ANIMATING\n");

            lv_obj_set_hidden(new_btn1, true);
            lv_obj_set_hidden(new_btn2, true);
            lv_obj_set_hidden(new_btn3, false);
            lv_obj_set_hidden(new_btn4, true);

            first_run = 2;
            settings_animation();
        }

        // check if width of btn 4 has changed :: 1.4s
        if (lv_obj_get_width(new_btn4) == 220) {

            printf("btn 4 FINISHED ANIMATING\n");

            lv_obj_set_hidden(new_btn1, true);
            lv_obj_set_hidden(new_btn2, true);
            lv_obj_set_hidden(new_btn3, true);
            lv_obj_set_hidden(new_btn4, false);

            first_run = 2;
            settings_animation();
        }
    }
}

    /*-------------------
     *     LVGL MENU
     * ------------------*/

    /*------------------
     *   wallpaper
     * -----------------*/
void background() {
    lv_obj_t* bg_wallpaper = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(bg_wallpaper, &color_tri_manichi_vGR);
}
    /*------------------
     *   back button
     * -----------------*/
void back_button() {

    // button
    back_btn = lv_btn_create(lv_scr_act(), NULL);
    //lv_btn_set_fit(back_btn, LV_FIT_TIGHT);
    lv_obj_set_size(back_btn, 60, 40);
    lv_obj_align(back_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_group_add_obj(g, back_btn);
    lv_obj_set_event_cb(back_btn, general_event_handler);

    // label font size :: icon
    static lv_style_t icon_style;
    lv_style_copy(&icon_style, &lv_style_plain);
    icon_style.text.font = &lv_font_roboto_22;
    icon_style.text.color = LV_COLOR_WHITE;

    lv_obj_t* back_btn_label = lv_label_create(back_btn, NULL);
    lv_label_set_style(back_btn_label, LV_LABEL_STYLE_MAIN, &icon_style);
    lv_label_set_text(back_btn_label, LV_SYMBOL_NEW_LINE);

}
    /*-------------------
     *   Gif player app
     * ------------------*/
void menu_gif() {

    /*unload main menu*/
    if (menu_count == 0) {
      lv_obj_del(scr);
    }
    /*unload directory screen*/
    else {
      lv_obj_del(scr_games);
    }
    
    //screen background wallpaper
    scr_gif = lv_img_create(NULL, NULL);
    lv_img_set_src(scr_gif, &color_tri_manichi_vGR);
    lv_scr_load(scr_gif);

    // background styling
    static lv_style_t bg_style;
    lv_style_copy(&bg_style, &lv_style_plain);
    bg_style.body.opa = 0;

    if (displayTimeSeconds == 3) {
      displayTimeSeconds = 30;
    }
    
    if (displayTimeSeconds == 6) {
      displayTimeSeconds = 60;
    }

    if (displayTimeSeconds != arcada.getGIFtime()) {
      arcada.setGIFtime(displayTimeSeconds, true);   // save to disk
    }

    gif_player_begin = 1;
    menu_count = 1;
}
    /*------------------
     *   games menu
     * -----------------*/
void menu_games() {
  
    /*unload main menu*/
    if (menu_count == 0) {
      lv_obj_del(scr);
    }
    /*unload directory screen*/
    if (menu_count == 1) {
      lv_obj_del(scr_gif);
    }

    /*create games menu*/

    //screen background wallpaper
    scr_games = lv_img_create(NULL, NULL);
    lv_img_set_src(scr_games, &color_tri_manichi_vGR);
    lv_scr_load(scr_games);

    //lv_style_scr.body.main_color = LV_COLOR_PURPLE;
    //lv_style_scr.body.grad_color = LV_COLOR_TEAL;//LV_COLOR_NAVY;//LV_COLOR_MAROON;//LV_COLOR_MAGENTA;//LV_COLOR_GRAY;//LV_COLOR_AQUA;//LV_COLOR_CYAN;

    // background styling
    static lv_style_t bg_style;
    lv_style_copy(&bg_style, &lv_style_plain);
    bg_style.body.opa = 0;
    //bg_style.body.padding.inner = 16;

    lv_obj_t* scroll_page = lv_page_create(scr_games, NULL);
    lv_obj_set_size(scroll_page, 240, 180);
    lv_page_set_style(scroll_page, LV_PAGE_STYLE_BG, &bg_style);
    lv_page_set_style(scroll_page, LV_PAGE_STYLE_SCRL, &bg_style);

       /* settings button lsit style */
    static lv_style_t btn_inactive_style;
    static lv_style_t btn_pressed_style;
    static lv_style_t btn_released_style;

    /* copy default button state styles*/
    lv_style_copy(&btn_inactive_style, &lv_style_btn_ina);
    lv_style_copy(&btn_pressed_style, &lv_style_btn_pr);
    lv_style_copy(&btn_released_style, &lv_style_btn_rel);

    /* modify padding for narrower buttons*/
    btn_inactive_style.body.padding.top = 8;
    btn_pressed_style.body.padding.top = 8;
    btn_released_style.body.padding.top = 8;

    btn_inactive_style.body.padding.bottom = 8;
    btn_pressed_style.body.padding.bottom = 8;
    btn_released_style.body.padding.bottom = 8;

    /*Create a list*/
    settings_btn_list = lv_list_create(scr_games, NULL);
    lv_obj_set_size(settings_btn_list, 180, 240);
    lv_obj_align(settings_btn_list, NULL, LV_ALIGN_CENTER, 0, 10);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_SCRL, &lv_style_transp_fit);

    btn_inactive_style.text.font = &lv_font_roboto_12;
    btn_pressed_style.text.font = &lv_font_roboto_12;
    btn_released_style.text.font = &lv_font_roboto_12;

    /* add styling to list*/
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_INA, &btn_inactive_style);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_PR, &btn_pressed_style);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_REL, &btn_released_style);

    /*Add buttons to the list*/
    folder_btn1 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_DIRECTORY, "Gif Folder 1");
    lv_group_add_obj(g, folder_btn1);
    lv_obj_set_event_cb(folder_btn1, general_event_handler);

    folder_btn2 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_DIRECTORY, "Gif Folder 2");
    lv_group_add_obj(g, folder_btn2);
    lv_obj_set_event_cb(folder_btn2, general_event_handler);

    folder_btn3 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_DIRECTORY, "Gif Folder 3");
    lv_group_add_obj(g, folder_btn3);
    lv_obj_set_event_cb(folder_btn3, general_event_handler);

    folder_btn4 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_DIRECTORY, "Gif Folder 4");
    lv_group_add_obj(g, folder_btn4);
    lv_obj_set_event_cb(folder_btn4, general_event_handler);

    folder_btn5 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_PLAY, "PLAY ALL");
    lv_group_add_obj(g, folder_btn5);
    lv_obj_set_event_cb(folder_btn5, general_event_handler);

    if (last_folder == 0) {
      lv_group_focus_obj(folder_btn1);
    }
    if (last_folder == 1) {
      lv_group_focus_obj(folder_btn2);
    }
    if (last_folder == 2) {
      lv_group_focus_obj(folder_btn3);
    }
    if (last_folder == 3) {
      lv_group_focus_obj(folder_btn4);
    }
    if (last_folder == 4) {
      lv_group_focus_obj(folder_btn5);
    }

    back_button();

    menu_count = 2;
}
    /*------------------
     *   emoji menu
     * -----------------*/
//void menu_emoji() {
//    /*unload main menu*/
//    lv_obj_del(scr);

    /*create emoji menu*/

    //screen background wallpaper
//    scr_emoji = lv_img_create(NULL, NULL);
//    lv_img_set_src(scr_emoji, &color_tri_manichi_vGR);
//    lv_scr_load(scr_emoji);

    // background styling
//    static lv_style_t bg_style;
//    lv_style_copy(&bg_style, &lv_style_plain);
//    bg_style.body.opa = 0;
    //bg_style.body.padding.inner = 16;

//    back_button();

//    menu_count = 3;
//}
    /*---------------------------------------------------
     *   function call to track user set Gif play time
     * -------------------------------------------------*/
void checkbox_caller() {
  if (displayTimeSeconds == 0) {
    lv_cb_set_checked(gif_time_once_cb, true);
    lv_cb_set_checked(gif_time_10s_cb, false);
    lv_cb_set_checked(gif_time_30s_cb, false);
    lv_cb_set_checked(gif_time_1min_cb, false);
    lv_cb_set_checked(gif_time_4ever_cb, false);
  }
  if (displayTimeSeconds == 10) {
    lv_cb_set_checked(gif_time_once_cb, false);
    lv_cb_set_checked(gif_time_10s_cb, true);
    lv_cb_set_checked(gif_time_30s_cb, false);
    lv_cb_set_checked(gif_time_1min_cb, false);
    lv_cb_set_checked(gif_time_4ever_cb, false);
  }
  if (displayTimeSeconds == 3) {
    lv_cb_set_checked(gif_time_once_cb, false);
    lv_cb_set_checked(gif_time_10s_cb, false);
    lv_cb_set_checked(gif_time_30s_cb, true);
    lv_cb_set_checked(gif_time_1min_cb, false);
    lv_cb_set_checked(gif_time_4ever_cb, false);
  }
  if (displayTimeSeconds == 6) {
    lv_cb_set_checked(gif_time_once_cb, false);
    lv_cb_set_checked(gif_time_10s_cb, false);
    lv_cb_set_checked(gif_time_30s_cb, false);
    lv_cb_set_checked(gif_time_1min_cb, true);
    lv_cb_set_checked(gif_time_4ever_cb, false);
  }
  if (displayTimeSeconds == 8) {
    lv_cb_set_checked(gif_time_once_cb, false);
    lv_cb_set_checked(gif_time_10s_cb, false);
    lv_cb_set_checked(gif_time_30s_cb, false);
    lv_cb_set_checked(gif_time_1min_cb, false);
    lv_cb_set_checked(gif_time_4ever_cb, true); 
  }
}
    /*------------------
     *   Gif play time
     * -----------------*/
void pg_gif_play_time() {
  // unload settings menu
  lv_obj_del(scr_settings);

  // create new screen container
  scr_gif_play_time = lv_cont_create(NULL, NULL);
  lv_obj_set_size(scr_gif_play_time, 240, 240);
  lv_scr_load(scr_gif_play_time);

  // universal background function
  background();

  // checkbox style
  static lv_style_t checkbox_style_1;
  lv_style_copy(&checkbox_style_1, &lv_style_pretty);
  checkbox_style_1.body.main_color = LV_COLOR_AQUA;
  checkbox_style_1.body.grad_color = LV_COLOR_WHITE;
  checkbox_style_1.body.radius = 10;
  checkbox_style_1.body.opa = 155; //10
  //checkbox_style_1.body.padding.inner = 90;
  checkbox_style_1.body.padding.inner = 15;
  checkbox_style_1.body.padding.left      = 8;
  checkbox_style_1.body.padding.right     = 8;
  checkbox_style_1.body.padding.top       = 8;
  checkbox_style_1.body.padding.bottom    = 8;

  //checkbox object 1
  gif_time_once_cb = lv_cb_create(scr_gif_play_time, NULL);
  lv_cb_set_text(gif_time_once_cb, "ONCE");
  lv_obj_align(gif_time_once_cb, NULL, LV_ALIGN_IN_TOP_MID, -6, 11);
  lv_group_add_obj(g, gif_time_once_cb);
  lv_obj_set_event_cb(gif_time_once_cb, general_event_handler);
  lv_cb_set_style(gif_time_once_cb, LV_CB_STYLE_BG, &checkbox_style_1);
   
  //checkbox object 2
  gif_time_10s_cb = lv_cb_create(scr_gif_play_time, NULL);
  lv_cb_set_text(gif_time_10s_cb, "10s");
  lv_obj_align(gif_time_10s_cb, NULL, LV_ALIGN_IN_TOP_MID, -23, 46);
  lv_group_add_obj(g, gif_time_10s_cb);
  lv_obj_set_event_cb(gif_time_10s_cb, general_event_handler);
  lv_cb_set_style(gif_time_10s_cb, LV_CB_STYLE_BG, &checkbox_style_1);

  //checkbox object 3
  gif_time_30s_cb = lv_cb_create(scr_gif_play_time, NULL);
  lv_cb_set_text(gif_time_30s_cb, "30s");
  lv_obj_align(gif_time_30s_cb, NULL, LV_ALIGN_IN_TOP_MID, -23, 81);
  lv_group_add_obj(g, gif_time_30s_cb);
  lv_obj_set_event_cb(gif_time_30s_cb, general_event_handler);
  lv_cb_set_style(gif_time_30s_cb, LV_CB_STYLE_BG, &checkbox_style_1);

  //checkbox object 4
  gif_time_1min_cb = lv_cb_create(scr_gif_play_time, NULL);
  lv_cb_set_text(gif_time_1min_cb, "60s");
  lv_obj_align(gif_time_1min_cb, NULL, LV_ALIGN_IN_TOP_MID, -9, 116);
  lv_group_add_obj(g, gif_time_1min_cb);
  lv_obj_set_event_cb(gif_time_1min_cb, general_event_handler);
  lv_cb_set_style(gif_time_1min_cb, LV_CB_STYLE_BG, &checkbox_style_1);

  //checkbox object 5
  gif_time_4ever_cb = lv_cb_create(scr_gif_play_time, NULL);
  lv_cb_set_text(gif_time_4ever_cb, "FOREVER !");
  lv_obj_align(gif_time_4ever_cb, NULL, LV_ALIGN_IN_TOP_MID, 0, 151);
  lv_group_add_obj(g, gif_time_4ever_cb);
  lv_obj_set_event_cb(gif_time_4ever_cb, general_event_handler);
  lv_cb_set_style(gif_time_4ever_cb, LV_CB_STYLE_BG, &checkbox_style_1);
  
  checkbox_caller();
  Serial.print("Gif Time: ");
  Serial.println(displayTimeSeconds);

  // universal back button
  back_button();

  menu_count = 41;
}
    /*------------------
     *   Gif shuffle
     * -----------------*/
void pg_gif_shuffle() {
  /*unload settings menu*/
  lv_obj_del(scr_settings);

  /*create gif shuffle menu page*/
  scr_random = lv_cont_create(NULL, NULL);
  lv_obj_set_size(scr_random, 240, 240);
  lv_scr_load(scr_random);

  //screen background wallpaper funcion call
  background();

  //lv_style_scr.body.main_color = LV_COLOR_PURPLE;
  //lv_style_scr.body.grad_color = LV_COLOR_TEAL;//LV_COLOR_NAVY;//LV_COLOR_MAROON;//LV_COLOR_MAGENTA;//LV_COLOR_GRAY;//LV_COLOR_AQUA;//LV_COLOR_CYAN;

  // background styling
  static lv_style_t bg_style;
  lv_style_copy(&bg_style, &lv_style_plain);
  bg_style.body.opa = 0;
  //bg_style.body.padding.inner = 16;

  back_button();

  /*Create styles for the switch*/
  static lv_style_t sw_style;
  static lv_style_t indic_style;
  static lv_style_t knob_on_style;
  static lv_style_t knob_off_style;

  lv_style_copy(&sw_style, &lv_style_pretty);
  sw_style.body.radius = LV_RADIUS_CIRCLE;
  sw_style.body.padding.top = 6;
  sw_style.body.padding.bottom = 6;

  lv_style_copy(&indic_style, &lv_style_pretty_color);
  indic_style.body.radius = LV_RADIUS_CIRCLE;
  indic_style.body.main_color = lv_color_hex(0x9fc8ef);
  indic_style.body.grad_color = lv_color_hex(0x9fc8ef);
  indic_style.body.padding.left = 0;
  indic_style.body.padding.right = 0;
  indic_style.body.padding.top = 0;
  indic_style.body.padding.bottom = 0;

  lv_style_copy(&knob_off_style, &lv_style_pretty);
  knob_off_style.body.radius = LV_RADIUS_CIRCLE;
  knob_off_style.body.shadow.width = 4;
  knob_off_style.body.shadow.type = LV_SHADOW_BOTTOM;

  lv_style_copy(&knob_on_style, &lv_style_pretty_color);
  knob_on_style.body.radius = LV_RADIUS_CIRCLE;
  knob_on_style.body.shadow.width = 4;
  knob_on_style.body.shadow.type = LV_SHADOW_BOTTOM;

  /*Create a switch and apply the styles*/
  sw1 = lv_sw_create(scr_random, NULL);
  lv_sw_set_style(sw1, LV_SW_STYLE_BG, &sw_style);
  lv_sw_set_style(sw1, LV_SW_STYLE_INDIC, &indic_style);
  lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_ON, &knob_on_style);
  lv_sw_set_style(sw1, LV_SW_STYLE_KNOB_OFF, &knob_off_style);
    
  sw_random = y;
  if (sw_random == 1) {

    lv_sw_on(sw1, LV_ANIM_OFF);
  }
  if (sw_random == 2) {

    lv_sw_off(sw1, LV_ANIM_OFF);
  }

  lv_obj_align(sw1, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_group_add_obj(g, sw1);
  lv_obj_set_event_cb(sw1, general_event_handler);

  //back_button();

  menu_count = 42;
}
    /*------------------
     *   TFT brightness
     * -----------------*/
void pg_brightness() {
  lv_obj_del(scr_settings);
    
  // create new screen container
  scr_brightness = lv_cont_create(NULL, NULL);
  lv_obj_set_size(scr_brightness, 240, 240);
  lv_scr_load(scr_brightness);

  // universal background function
  background();

  /*Create a style for the LED*/
  static lv_style_t style_led;
  lv_style_copy(&style_led, &lv_style_pretty_color);
  style_led.body.radius = LV_RADIUS_CIRCLE;
  style_led.body.main_color = LV_COLOR_YELLOW;
  style_led.body.grad_color = LV_COLOR_WHITE;
  style_led.body.border.color = LV_COLOR_MAKE(0xfa, 0x0f, 0x00);
  style_led.body.border.width = 3;
  style_led.body.border.opa = LV_OPA_30;
  style_led.body.shadow.color = LV_COLOR_MAKE(0xb5, 0x0f, 0x04);
  style_led.body.shadow.width = 5;

  /*Create a LED and switch it OFF*/
  led1 = lv_led_create(lv_scr_act(), NULL);
  lv_led_set_style(led1, LV_LED_STYLE_MAIN, &style_led);
  lv_obj_align(led1, NULL, LV_ALIGN_CENTER, -80, 0);
  lv_led_off(led1);

  /*Copy the previous LED and set a brightness*/
  led2 = lv_led_create(lv_scr_act(), led1);
  lv_obj_align(led2, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_led_set_bright(led2, 190);

  /*Copy the previous LED and switch it ON*/
  led3 = lv_led_create(lv_scr_act(), led1);
  lv_obj_align(led3, NULL, LV_ALIGN_CENTER, 80, 0);
  lv_led_on(led2);

  // opaque buttons to turn on LEDs

  static lv_style_t btn_led_style;
  lv_style_copy(&btn_led_style, &lv_style_plain);
  btn_led_style.body.radius = LV_RADIUS_CIRCLE;
  btn_led_style.body.main_color = LV_COLOR_AQUA;
  btn_led_style.body.grad_color = LV_COLOR_WHITE;
  btn_led_style.body.opa = 0;

  btn_led1 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_size(btn_led1, 41, 41);
  lv_obj_align(btn_led1, NULL, LV_ALIGN_CENTER, -80, 0);
  lv_btn_set_style(btn_led1, LV_BTN_STYLE_INA, &btn_led_style);
  lv_btn_set_style(btn_led1, LV_BTN_STYLE_PR, &btn_led_style);
  lv_btn_set_style(btn_led1, LV_BTN_STYLE_REL, &btn_led_style);
  lv_group_add_obj(g, btn_led1);
  lv_obj_set_event_cb(btn_led1, general_event_handler);

  btn_led2 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_size(btn_led2, 41, 41);
  lv_obj_align(btn_led2, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_btn_set_style(btn_led2, LV_BTN_STYLE_INA, &btn_led_style);
  lv_btn_set_style(btn_led2, LV_BTN_STYLE_PR, &btn_led_style);
  lv_btn_set_style(btn_led2, LV_BTN_STYLE_REL, &btn_led_style);
  lv_group_add_obj(g, btn_led2);
  lv_obj_set_event_cb(btn_led2, general_event_handler);

  btn_led3 = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_size(btn_led3, 41, 41);
  lv_obj_align(btn_led3, NULL, LV_ALIGN_CENTER, 80, 0);
  lv_btn_set_style(btn_led3, LV_BTN_STYLE_INA, &btn_led_style);
  lv_btn_set_style(btn_led3, LV_BTN_STYLE_PR, &btn_led_style);
  lv_btn_set_style(btn_led3, LV_BTN_STYLE_REL, &btn_led_style);
  lv_group_add_obj(g, btn_led3);
  lv_obj_set_event_cb(btn_led3, general_event_handler);
    
  LED_counter = arcada.getBacklight();
    
  if (LED_counter == 36) { //36 for filming : was 63
    lv_led_on(led1);
    lv_led_off(led2);
    lv_led_off(led3);
  }
  if (LED_counter == 127) {
    lv_led_off(led1);
    lv_led_on(led2);
    lv_led_off(led3);
  }
  if (LED_counter == 255) {
    lv_led_off(led1);
    lv_led_off(led2);
    lv_led_on(led3);
  }

  // universal back button
  back_button();

  menu_count = 43;
}

void pg_battery() {
  // unload settings menu
  lv_obj_del(scr_settings);

  // create new screen container
  scr_battery = lv_cont_create(NULL, NULL);
  lv_obj_set_size(scr_battery, 240, 240);
  lv_scr_load(scr_battery);

  // universal background function
  background();



  // universal back button
  back_button();

  menu_count = 44;
}
    /*-------------------
     *   settings menu
     * ------------------*/
void menu_settings() {

    /* unload main menu*/
    if (menu_count == 0) {
        lv_obj_del(scr);
    }
    /*unload settings sub pages*/
    if (menu_count == 41) {
        lv_obj_del(scr_gif_play_time);
    }
    if (menu_count == 42) {
        lv_obj_del(scr_random);
    }
    if (menu_count == 43) {
        lv_obj_del(scr_brightness);
    }
    if (menu_count == 44) {
        lv_obj_del(scr_battery);
    }

    scr_settings = lv_cont_create(NULL, NULL);
    lv_obj_set_size(scr_settings, 240, 240);

    /* call background function */
    background();

    /* settings button lsit style */
    static lv_style_t btn_inactive_style;
    static lv_style_t btn_pressed_style;
    static lv_style_t btn_released_style;

    /* copy default button state styles*/
    lv_style_copy(&btn_inactive_style, &lv_style_btn_ina);
    lv_style_copy(&btn_pressed_style, &lv_style_btn_pr);
    lv_style_copy(&btn_released_style, &lv_style_btn_rel);

    /* modify padding for narrower buttons*/
    btn_inactive_style.body.padding.top = 10;
    btn_pressed_style.body.padding.top = 10;
    btn_released_style.body.padding.top = 10;

    btn_inactive_style.body.padding.bottom = 10;
    btn_pressed_style.body.padding.bottom = 10;
    btn_released_style.body.padding.bottom = 10;

    /*Create a list*/
    settings_btn_list = lv_list_create(scr_settings, NULL);
    lv_obj_set_size(settings_btn_list, 180, 240);
    lv_obj_align(settings_btn_list, NULL, LV_ALIGN_CENTER, 0, 10);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BG, &lv_style_transp_fit);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_SCRL, &lv_style_transp_fit);

    /* add styling to list*/
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_INA, &btn_inactive_style);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_PR, &btn_pressed_style);
    lv_list_set_style(settings_btn_list, LV_LIST_STYLE_BTN_REL, &btn_released_style);

    /*Add buttons to the list*/
    settings_btn1 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_REFRESH, "Gif play time");
    lv_group_add_obj(g, settings_btn1);
    lv_obj_set_event_cb(settings_btn1, general_event_handler);

    settings_btn2 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_SHUFFLE, "Gif shuffle");
    lv_group_add_obj(g, settings_btn2);
    lv_obj_set_event_cb(settings_btn2, general_event_handler);

    settings_btn3 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_PLUS, "brightness");
    lv_group_add_obj(g, settings_btn3);
    lv_obj_set_event_cb(settings_btn3, general_event_handler);

    settings_btn4 = lv_list_add_btn(settings_btn_list, LV_SYMBOL_BATTERY_FULL, "battery");
    lv_group_add_obj(g, settings_btn4);
    lv_obj_set_event_cb(settings_btn4, general_event_handler);

    if (menu_count == 41) {
        lv_group_focus_obj(settings_btn1);
    }
    if (menu_count == 42) {
        lv_group_focus_obj(settings_btn2);
    }
    if (menu_count == 43) {
        lv_group_focus_obj(settings_btn3);
    }
    if (menu_count == 44) {
        lv_group_focus_obj(settings_btn4);
    }

    back_button();
    menu_count = 4;
}
    /*------------------
     *   main menu
     * -----------------*/
void menu_main() {

    /*unload menu 1*/
    if (menu_count == 1) {
        lv_obj_del(scr_gif);
    }
    /*unload menu 2*/
    if (menu_count == 2) {
        lv_obj_del(scr_games);
        last_folder = 0;
    }
    /*unload menu 3*/
    if (menu_count == 3) {
        lv_obj_del(scr_emoji);
    }
    /*unload menu 4*/
    if (menu_count == 4) {
        lv_obj_del(scr_settings);
    }

    icon_focus = 0;

    /*create main menu*/

    //screen background wallpaper
    scr = lv_cont_create(NULL, NULL);               
    lv_obj_set_size(scr, 240, 240);
    lv_scr_load(scr);
    main_bg = lv_img_create(scr, NULL);
    lv_img_set_src(main_bg, &color_tri_manichi_vGR);

    // background styling
    //static lv_style_t bg_style; // set above as global
    lv_style_copy(&bg_style, &lv_style_plain);
    bg_style.body.opa = 0;
    //bg_style.body.padding.inner = 16;

    /*Create a buffer for the canvas*/
    static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(CANVAS_WIDTH, CANVAS_HEIGHT)];

    /*Create a canvas and initialize its the palette*/
    canvas = lv_canvas_create(scr, NULL);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(canvas, 0, LV_COLOR_TRANSP);
    lv_canvas_set_palette(canvas, 1, LV_COLOR_RED);

    /*Create colors with the indices of the palette*/
    lv_color_t c0;
    lv_color_t c1;

    c0.full = 0;
    c1.full = 1;

    /*Transparent background*/
    lv_canvas_fill_bg(canvas, c1);

    /* set canvas position */
    lv_obj_set_pos(canvas, 90, 55);

    lv_style_copy(&img_animation, &lv_style_plain);
    img_animation.image.opa = 0;

    lv_style_copy(&img_opacity_0, &lv_style_plain);
    img_opacity_0.image.opa = 0;

    lv_style_copy(&img_opacity_255, &lv_style_plain);
    img_opacity_255.image.opa = 255;

    lv_style_copy(&canvas_style, &bg_style);
    lv_canvas_set_style(canvas, LV_CANVAS_STYLE_MAIN, &canvas_style);

    /* gif icon image */
    icon1b = lv_img_create(canvas, NULL);
    lv_img_set_src(icon1b, &blox_gif_bg);
    lv_obj_align(icon1b, NULL, LV_ALIGN_CENTER, 0, 0);
    //lv_obj_set_pos(icon1b, 95, 60);
    lv_img_set_style(icon1b, LV_IMG_STYLE_MAIN, &img_animation);

    /* game icon image */
    icon2b = lv_img_create(canvas, NULL);
    lv_img_set_src(icon2b, &blox_games_bg);
    lv_obj_align(icon2b, NULL, LV_ALIGN_CENTER, 0, 0);
    //lv_obj_set_pos(icon2b, 95, 60);
    lv_img_set_style(icon2b, LV_IMG_STYLE_MAIN, &img_animation);

    /* emoji icon image */
    icon3b = lv_img_create(canvas, NULL);
    lv_img_set_src(icon3b, &blox_emoji_bg);
    lv_obj_align(icon3b, NULL, LV_ALIGN_CENTER, 0, 0);
    //lv_obj_set_pos(icon3b, 95, 60);
    lv_img_set_style(icon3b, LV_IMG_STYLE_MAIN, &img_animation);

    /* settings icon image */
    icon4b = lv_img_create(canvas, NULL);
    lv_img_set_src(icon4b, &blox_settings_bg);
    lv_obj_align(icon4b, NULL, LV_ALIGN_CENTER, 0, 0);
    //lv_obj_set_pos(icon4b, 95, 60);
    lv_img_set_style(icon4b, LV_IMG_STYLE_MAIN, &img_animation);

    // animated buttons
    new_btn1 = lv_btn_create(scr, NULL);
    lv_obj_set_size(new_btn1, 60, 51);
    //lv_group_add_obj(g, new_btn1);
    lv_obj_set_event_cb(new_btn1, general_event_handler);

    new_btn2 = lv_btn_create(scr, NULL);
    lv_obj_set_size(new_btn2, 60, 51);
    //lv_group_add_obj(g, new_btn2);
    lv_obj_set_event_cb(new_btn2, general_event_handler);

    new_btn3 = lv_btn_create(scr, NULL);
    lv_obj_set_size(new_btn3, 60, 51);
    //lv_group_add_obj(g, new_btn3);
    lv_obj_set_event_cb(new_btn3, general_event_handler);

    new_btn4 = lv_btn_create(scr, NULL);
    lv_obj_set_size(new_btn4, 60, 51);
    //lv_group_add_obj(g, new_btn4);
    lv_obj_set_event_cb(new_btn4, general_event_handler);

    lv_style_copy(&focus_style, &lv_style_plain_color);
    focus_style.text.color = LV_COLOR_ORANGE;
    focus_style.text.font = &lv_font_roboto_22;

    lv_style_copy(&defocus_style, &focus_style);
    defocus_style.text.color = LV_COLOR_WHITE;
    defocus_style.text.font = &lv_font_roboto_22;

    static lv_style_t symbol_size;
    lv_style_copy(&symbol_size, &defocus_style);
    symbol_size.text.font = &lv_font_roboto_22;

    // icons for above buttons
    label_icon1 = lv_label_create(new_btn1, NULL);                      
    lv_label_set_text(label_icon1, LV_SYMBOL_VIDEO);                    //LV_SYMBOL_FILE
    lv_obj_align(label_icon1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(label_icon1, &symbol_size);

    label_icon2 = lv_label_create(new_btn2, NULL);  
    lv_label_set_text(label_icon2, LV_SYMBOL_DIRECTORY);                //LV_SYMBOL_SD_CARD
    lv_obj_align(label_icon2, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(label_icon2, &symbol_size);

    label_icon3 = lv_label_create(new_btn3, NULL);
    lv_label_set_text(label_icon3, LV_SYMBOL_IMAGE);                    //LV_SYMBOL_CLOSE
    lv_obj_align(label_icon3, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(label_icon3, &symbol_size);

    label_icon4 = lv_label_create(new_btn4, NULL);
    lv_label_set_text(label_icon4, LV_SYMBOL_SETTINGS);                 //LV_SYMBOL_EDIT
    lv_obj_align(label_icon4, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(label_icon4, &symbol_size);

    if (menu_count < 0) {
        lv_obj_set_pos(new_btn1, 10, -110);
        lv_obj_set_pos(new_btn2, 10, -166);
        lv_obj_set_pos(new_btn3, 10, -222);
        lv_obj_set_pos(new_btn4, 10, -278);
        img_animation.image.opa = 0;

        //lv_obj_align(mbox1, NULL, LV_ALIGN_IN_TOP_MID, 45, 60);
        anim_button();
    }
    if (menu_count >= 0) {
        lv_obj_set_pos(new_btn1, 10, 10);
        lv_obj_set_pos(new_btn2, 10, 66);
        lv_obj_set_pos(new_btn3, 10, 122);
        lv_obj_set_pos(new_btn4, 10, 178);
        img_animation.image.opa = 255;

        lv_group_add_obj(g, new_btn1);
        lv_group_add_obj(g, new_btn2);
        lv_group_add_obj(g, new_btn3);
        lv_group_add_obj(g, new_btn4);

        //lv_obj_align(mbox1, NULL, LV_ALIGN_IN_TOP_MID, 45, 30);
    }

    lv_obj_set_hidden(icon1b, false);
    lv_obj_set_hidden(icon2b, true);
    lv_obj_set_hidden(icon3b, true);
    lv_obj_set_hidden(icon4b, true);

    //lv_obj_set_hidden(icon_1_container, false);
    //lv_obj_set_hidden(icon_2_container, true);
    //lv_obj_set_hidden(icon_3_container, true);
    //lv_obj_set_hidden(icon_4_container, true);

    if (last_icon == 1) {
        lv_group_focus_obj(new_btn1);
    }
    if (last_icon == 2) {
        lv_group_focus_obj(new_btn2);
    }
    if (last_icon == 3) {
        lv_group_focus_obj(new_btn3);
    }
    if (last_icon == 4) {
        lv_group_focus_obj(new_btn4);
    }
    menu_count = 0;
}
    /*-------------------
     *   GUI initaliser
     * ------------------*/
void lv_test_UI(void)
{
    if (menu_count == -2) {
        menu_main();
    }
}
    /*-------------------------------
     *   LVGL group style modifier
     * -----------------------------*/
static void my_style_mod_cb(lv_group_t* group, lv_style_t* style)
{
    /*Make the bodies a little bit orange*/
    style->body.border.opa = LV_OPA_COVER;
    style->body.border.color = LV_COLOR_ORANGE;
    style->body.border.width = LV_DPI / 20;

    //style->body.main_color = lv_color_mix(style->body.main_color, LV_COLOR_ORANGE, LV_OPA_70);
    //style->body.grad_color = lv_color_mix(style->body.grad_color, LV_COLOR_ORANGE, LV_OPA_70);
    //style->body.shadow.color = lv_color_mix(style->body.shadow.color, LV_COLOR_ORANGE, LV_OPA_60);

    /*Recolor text*/
    style->text.color = lv_color_mix(style->text.color, LV_COLOR_ORANGE, LV_OPA_70);

    /*Add some recolor to the images*/
    if (style->image.intense < LV_OPA_MIN) {
        style->image.color = LV_COLOR_ORANGE;
        style->image.intense = LV_OPA_40;
    }
}
    /*-------------------
     *   event handler
     * ------------------*/
static void general_event_handler(lv_obj_t* obj, lv_event_t event)
{
  /* main menu */
  
/*-------- 1. GIF Player Button -------- */
  if (obj == new_btn1) {
    
    if (event == LV_EVENT_FOCUSED) {
      
      lv_obj_set_hidden(icon1b, false);
      lv_obj_set_hidden(icon2b, true);
      lv_obj_set_hidden(icon3b, true);
      lv_obj_set_hidden(icon4b, true);

      icon_focus = 1;
      lv_obj_set_style(label_icon1, &focus_style);
      lv_obj_set_style(label_icon2, &defocus_style);
      lv_obj_set_style(label_icon3, &defocus_style);
      lv_obj_set_style(label_icon4, &defocus_style);
    }
    
    if (event == LV_EVENT_PRESSED) {

      //menu_gif();
      first_run = 1;

      if (last_icon == 2) {
        prev_icon = 2;
      }
      if (last_icon == 3) {
        prev_icon = 3;
      }
      
      last_icon = 1;     
      settings_animation();
      
    }
    
    icon_count = 1;
    
  }
  
/*-------- 2. Directory Button -------- */
  if (obj == new_btn2) {
    
    if (event == LV_EVENT_FOCUSED) {
      
      lv_obj_set_hidden(icon2b, false);
      lv_obj_set_hidden(icon3b, true);
      lv_obj_set_hidden(icon4b, true);
      lv_obj_set_hidden(icon1b, true); 

      icon_focus = 2;
      lv_obj_set_style(label_icon1, &defocus_style);
      lv_obj_set_style(label_icon2, &focus_style);
      lv_obj_set_style(label_icon3, &defocus_style);
      lv_obj_set_style(label_icon4, &defocus_style);
    }
    
    if (event == LV_EVENT_PRESSED) {

      last_icon = 2;
      menu_games();
      
    }
    
    icon_count = 2;
    
  }
  
/*-------- 2. Directory Page -------- */
    if (obj == folder_btn1) {
        if (event == LV_EVENT_PRESSED) {
            // x directory = 0
            // lauch gif app or maybe away to to preview gif and return or gif file list
            // pop up menu [play now? cancel]

          menu_count = 21;

          //x = 0;            // selects first folder from DIRECTORY
          mode_select = 2;    // up/down gestures only [nextGIF/prevGIF]
          play_all = 0;       // only plays GIFs in this directory
          app_num = 0;        // called in main loop - nextGIF if true
          last_folder = 0;    // keep track of folder number for lvgl group highlighter
          menu_gif();         // call gif player function       
        }   
    }

    if (obj == folder_btn2) {
        if (event == LV_EVENT_PRESSED) {
            // x directory = 1

          menu_count = 22;  

          //x = 1;            // selects second folder from DIRECTORY
          mode_select = 2;    // up/down gestures only [nextGIF/prevGIF]
          play_all = 0;       // only plays GIFs in this directory
          app_num = 1;        // called in main loop - nextGIF if true
          last_folder = 1;    // keep track of folder number for lvgl group highlighter
          menu_gif();         // call gif player function                  
        }
    }

    if (obj == folder_btn3) {
        if (event == LV_EVENT_PRESSED) {
            // x directory = 2
            // lauch gif app or maybe away to to preview gif and return or gif file list
            // pop up menu [play now? cancel]
            
          menu_count = 23;
          
          //x = 2;            // selects third folder from DIRECTORY
          mode_select = 2;    // up/down gestures only [nextGIF/prevGIF]
          play_all = 0;       // only plays GIFs in this directory
          app_num = 2;        // called in main loop - nextGIF if true
          last_folder = 2;    // keep track of folder number for lvgl group highlighter
          menu_gif();         // call gif player function                   
        }
    }

    if (obj == folder_btn4) {
        if (event == LV_EVENT_PRESSED) {
            // x directory = 3
            // lauch gif app or maybe away to to preview gif and return or gif file list
            // pop up menu [play now? cancel]

          menu_count = 24;

          //x = 3;            // selects fourth folder from DIRECTORY
          mode_select = 2;    // up/down gestures only [nextGIF/prevGIF]
          play_all = 0;       // only plays GIFs in this directory
          app_num = 3;        // called in main loop - nextGIF if true
          last_folder = 3;    // keep track of folder number for lvgl group highlighter
          menu_gif();         // call gif player function    
        }
    }

    if (obj == folder_btn5) {
        if (event == LV_EVENT_PRESSED) {
            // play all  = 1 :: aka showcase mode :: no gesture | actionButton only
            // lauch gif app or maybe away to to preview gif and return or gif file list
            // pop up menu [play now? cancel]

          menu_count = 25;
          
          mode_select = 2;    // up/down only
          play_all = 1;       // enabled
          app_num = 4;        // called in main loop - nextGIF if true
          last_folder = 4;    // keep track of folder number for lvgl group highlighter
          menu_gif();         // call gif player function  
        }
    }
      
/*-------- 3. Showcase Button -------- */
  if (obj == new_btn3) {
    
    if (event == LV_EVENT_FOCUSED) {
      lv_obj_set_hidden(icon3b, false);
      lv_obj_set_hidden(icon4b, true);
      lv_obj_set_hidden(icon1b, true);
      lv_obj_set_hidden(icon2b, true);

      icon_focus = 3;
      lv_obj_set_style(label_icon1, &defocus_style);
      lv_obj_set_style(label_icon2, &defocus_style);
      lv_obj_set_style(label_icon3, &focus_style);
      lv_obj_set_style(label_icon4, &defocus_style);
    }
    
    if (event == LV_EVENT_PRESSED) {  
                            
      //menu_emoji();
      first_run = 1;

      if (last_icon == 1) {
        prev_icon = 1;
      }
      if (last_icon == 2) {
        prev_icon = 2;
      }
      
      last_icon = 3;
      settings_animation();
      
    }
    
    icon_count = 3;
    
  }

/*-------- 4. Settings Button -------- */
  if (obj == new_btn4) {
    
    if (event == LV_EVENT_FOCUSED) {
      lv_obj_set_hidden(icon4b, false);
      lv_obj_set_hidden(icon1b, true);
      lv_obj_set_hidden(icon2b, true);
      lv_obj_set_hidden(icon3b, true);

      icon_focus = 4;
      lv_obj_set_style(label_icon1, &defocus_style);
      lv_obj_set_style(label_icon2, &defocus_style);
      lv_obj_set_style(label_icon3, &defocus_style);
      lv_obj_set_style(label_icon4, &focus_style);
    }
    
    if (event == LV_EVENT_PRESSED) {

      last_icon = 4;
      menu_settings();
      
    }
        
  }

/*-------- settings menu -------- */
    
  /* gif play time page    :: object event handler*/
  if (obj == settings_btn1 && event == LV_EVENT_PRESSED) {
      pg_gif_play_time();
  }

  /*
  lv_obj_t* gif_time_once_cb;
  lv_obj_t* gif_time_10s_cb;
  lv_obj_t* gif_time_30s_cb;
  lv_obj_t* gif_time_1min_cb;
  lv_obj_t* gif_time_4ever_cb
  */

  // PLAY ONCE
  if (obj == gif_time_once_cb) {
    
    if (event == LV_EVENT_VALUE_CHANGED) {     
      if (lv_cb_is_checked(obj) == 1) {    
        //gif_time_cb_tracker = 1;
        //checkbox_caller();
        
            displayTimeSeconds = 0; // play ONCE
            Serial.printf("New GIF time %d", displayTimeSeconds); 

            checkbox_caller();
      } 
    }
  }

  // PLAY 10s
  if (obj == gif_time_10s_cb) {

    if (event == LV_EVENT_VALUE_CHANGED) {
      if (lv_cb_is_checked(obj) == 1) {
        //gif_time_cb_tracker = 10;
        
            displayTimeSeconds = 10; // 10 seconds
            Serial.printf("New GIF time %d", displayTimeSeconds);
            
            checkbox_caller();
      }
    }
  }

  // PLAY 30s
  if (obj == gif_time_30s_cb) {

    if (event == LV_EVENT_VALUE_CHANGED) {
      if (lv_cb_is_checked(obj) == 1) {
          //gif_time_cb_tracker = 3;

            displayTimeSeconds = 3; // 30 seconds
            Serial.printf("New GIF time %d", displayTimeSeconds);
            
            checkbox_caller();
      }
    }
  }

  // PLAY 60s
  if (obj == gif_time_1min_cb) {

    if (event == LV_EVENT_VALUE_CHANGED) {
      if (lv_cb_is_checked(obj) == 1) {
        //gif_time_cb_tracker = 6;

            displayTimeSeconds = 6; // 1 minute
            Serial.printf("New GIF time %d", displayTimeSeconds);

            checkbox_caller();
      }
    }
  }

  // PLAY FOREVER
  if (obj == gif_time_4ever_cb) {

    if (event == LV_EVENT_VALUE_CHANGED) {
      if (lv_cb_is_checked(obj) == 1) {
        //gif_time_cb_tracker = 8;
                   
            //displayTimeSeconds = arcada.getGIFtime();
            displayTimeSeconds = 8; // 1 day
            Serial.printf("New GIF time %d", displayTimeSeconds);
            //arcada.setGIFtime(displayTimeSeconds, true);   // save to disk

            checkbox_caller();
      }
    }
  }

  /* gif shuffle page      :: object event handler */
  if (obj == settings_btn2 && event == LV_EVENT_PRESSED) {
    pg_gif_shuffle();
  }
  
  /* gif shuffle switch    :: ON/OFF */
  if (obj == sw1) {
    if (event == LV_EVENT_FOCUSED) {
        }

    if (event == LV_EVENT_VALUE_CHANGED) {
      printf("State: %s\n", lv_sw_get_state(obj) ? "On" : "Off");
      if (lv_sw_get_state(obj) == 1) {
        //sw_random = 1;
        
          y = arcada.getRandom();
          y = 1;
          Serial.printf("RANDOMISER ON %d", y);
          arcada.setRandom(y, true);   // save to disk
          
      }
      if (lv_sw_get_state(obj) == 0) {
        //sw_random = 0;
        
          y = arcada.getRandom();
          y = 2;
          Serial.printf("RANDOMISER OFF %d", y);
          arcada.setRandom(y, true);   // save to disk
          
      }
    }

  }

    /*brightness page       :: object event handler*/
    if (obj == settings_btn3 && event == LV_EVENT_PRESSED) {
        pg_brightness();
    }

    if (obj == btn_led1 && event == LV_EVENT_PRESSED) {
        lv_led_on(led1);
        lv_led_off(led2);
        lv_led_off(led3);
        //LED_counter = 1;

            // low brightness >> 63 // 36 for filming
            int16_t newbrightness = arcada.getBacklight();
            newbrightness = 36;
            Serial.printf("New brightness %d", newbrightness);
            arcada.setBacklight(newbrightness, true);   // save to disk
    }
    if (obj == btn_led2 && event == LV_EVENT_PRESSED) {
        lv_led_off(led1);
        lv_led_on(led2);
        lv_led_off(led3);
        //LED_counter = 2;
        
            // medium brightness >> 127
            int16_t newbrightness = arcada.getBacklight();
            newbrightness = 127;
            Serial.printf("New brightness %d", newbrightness);
            arcada.setBacklight(newbrightness, true);   // save to disk
        
    }
    if (obj == btn_led3 && event == LV_EVENT_PRESSED) {
        lv_led_off(led1);
        lv_led_off(led2);
        lv_led_on(led3);
        //LED_counter = 3;

             // high brightness >> 255
            int16_t newbrightness = arcada.getBacklight();
            newbrightness = 255;
            Serial.printf("New brightness %d", newbrightness);
            arcada.setBacklight(newbrightness, true);   // save to disk
    }

  /*battery page          :: object event handler*/
  if (obj == settings_btn4 && event == LV_EVENT_PRESSED) {
    pg_battery();
  }

  /*universal objects*/
  if (obj == back_btn) {
    if (event == LV_EVENT_PRESSED) {
      if (menu_count <= 4) {
        menu_main();
        printf("4\n");
      }
      if (menu_count == 41 || menu_count == 42 || menu_count == 43 || menu_count == 44) {
        menu_settings();
        printf("4.2\n");
      }
    }
  }
  
  /*(void)obj; /*Unused
  
  #if LV_EX_PRINTF
      switch (event) {
      case LV_EVENT_PRESSED:
          printf("Pressed\n");
          break;

      case LV_EVENT_SHORT_CLICKED:
          printf("Short clicked\n");
          break;

      case LV_EVENT_CLICKED:
          printf("Clicked\n");
          break;

      case LV_EVENT_LONG_PRESSED:
          printf("Long press\n");
          break;

      case LV_EVENT_LONG_PRESSED_REPEAT:
          printf("Long press repeat\n");
          break;

      case LV_EVENT_VALUE_CHANGED:
          printf("Value changed: %s\n", lv_event_get_data() ? (const char*)lv_event_get_data() : "");
          break;

      case LV_EVENT_RELEASED:
          printf("Released\n");
          break;

      case LV_EVENT_DRAG_BEGIN:
          printf("Drag begin\n");
          break;

      case LV_EVENT_DRAG_END:
          printf("Drag end\n");
          break;

      case LV_EVENT_DRAG_THROW_BEGIN:
          printf("Drag throw begin\n");
          break;

      case LV_EVENT_FOCUSED:
          printf("Focused\n");
          break;
      case LV_EVENT_DEFOCUSED:
          printf("Defocused\n");
          break;
      default:
          break;
      }
  #endif*/
}

/*********************************************************************************************************************************************************************************/
/*********************************************************************************************************************************************************************************/
    /*-------------------
     *   general setup
     * ------------------*/     
void setup() {
  
  //set to auto randomise Gifs if user JSON file setting not found
  if (y == 0) {
    y = y + 2;
  }
  
  // initialise GifDecoder
  decoder.setScreenClearCallback(screenClearCallback);
  decoder.setUpdateScreenCallback(updateScreenCallback);
  decoder.setDrawPixelCallback(drawPixelCallback);
  decoder.setDrawLineCallback(drawLineCallback);

  decoder.setFileSeekCallback(fileSeekCallback);
  decoder.setFilePositionCallback(filePositionCallback);
  decoder.setFileReadCallback(fileReadCallback);
  decoder.setFileReadBlockCallback(fileReadBlockCallback);

  // initialise Arcada library
  arcada.arcadaBegin();
  arcada.filesysBeginMSD();
  arcada.filesysBegin();

  // Serial Monitor port address
  Serial.begin(115200); /* prepare for possible serial debug */

  // initalise LVGL
  lv_init();

// LVGL Debugger
#if USE_LV_LOG != 0
  lv_log_register_print(my_print); /* register print function for debugging */
#endif

  // Arcada
  arcada.displayBegin();
  
  // TFT backlight
  if (backlite == 0) {
    arcada.setBacklight(255);
  }

    // ---- CREATE FOLDER DIRECTORIES IF THEY DO NOT EXIST IN FILESYSTEM ---- \\
    
  if (! arcada.exists("/gifs")) { //make directory if it doesn't exist in filesystem [good so user knows which files to use and doesn't have to create them]
    arcada.mkdir("/gifs");
  }  

  if (! arcada.exists("/favs")) { //make directory if it doesn't exist in filesystem [good so user knows which files to use and doesn't have to create them]
    arcada.mkdir("/favs");
  }

    if (! arcada.exists("/grmod")) { //make directory if it doesn't exist in filesystem [good so user knows which files to use and doesn't have to create them]
    arcada.mkdir("/grmod");
  }
  
    if (! arcada.exists("/emojis")) { //make directory if it doesn't exist in filesystem [good so user knows which files to use and doesn't have to create them]
    arcada.mkdir("/emojis");
  }

    if (! arcada.exists("/favourites")) { //make directory if it doesn't exist in filesystem [good so user knows which files to use and doesn't have to create them]
    arcada.mkdir("/favourites");
  }
  
// ---- FILESYSTEM CREATION COMPLETE ---- \\


//------------------CREATE JSON CONFIG FILE IF IT DOES NOT EXIST-------------------\\

          /*-JSON CONFIG FILE-*/
  if (! arcada.loadConfigurationFile()) {

    //StaticJsonDocument<105> configJSON; // create JSON CONFIG file if doesn't exist

    // Add values to arcada_config JSON file
    configJSON["volume"] = 255;                         /* configJSON["happy"] = */
    configJSON["brightness"] = 255;
    configJSON["seconds_per_gif"] = 10;
    configJSON["random"] = 2;
    
    // generate minified arcada_config JSON file
    serializeJson(configJSON, Serial);
    // Serial should print: >> // {"volume":255,"brightness":255,"seconds_per_gif":10,"random":1}
    Serial.println();
    // Generate the prettified JSON and send it to the Serial port
    serializeJsonPretty(configJSON, Serial);

    File config_file = arcada.open(filename, FILE_WRITE);
    if (!config_file) {
      Serial.println(F("Failed to create file"));
      return;
    }

    // Copy values from the JsonDocument to the Config
    config._volume = configJSON["volume"] | 255;
    config._brightness = configJSON["brightness"] | 255;
    config._seconds_per_gif = configJSON["seconds_per_gif"] | 10;
    config._random = configJSON["random"] | 2;

    // Serialize JSON to file
    if (serializeJson(configJSON, config_file) == 0) {
      Serial.println(F("Failed to write to file"));
    }

    config_file.close();

    //saveConfiguration(filename, config);
    Serial.println(F("Print config file..."));
    Serial.printf(filename);   
  } 
//----------------------JSON FILE CREATION COMPLETE--------------------------------\\
  
  else if (! arcada.configJSON.containsKey("seconds_per_gif")) {
    
    Serial.println("Found config but no key");
    //arcada.infoBox("Configuration doesn't contain 'seconds_per_gif', using default 10 seconds per GIF");
    //arcada.display->fillScreen(ARCADA_BLUE);       
  } 
  
  else { 
    displayTimeSeconds = arcada.configJSON["seconds_per_gif"];
    y = arcada.configJSON["random"];  
  }
  
  Serial.printf("Playing GIFs for at least %d seconds per\n", displayTimeSeconds);
  stat;

  // LVGL DISPLAY SETUP :: LVGL.conf
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the graphics library's tick*/
  timer.start();

  // function calls
  buttons_drv_reg();
  lv_test_UI();
}

    /*---------------------------
     *   GifDecoder variables
     * -------------------------*/   
     
uint32_t fileStartTime = displayTimeSeconds * -1001;
uint32_t cycle_start = 0L;
int file_index = -1;
int8_t nextGIF = 0;       // 0 for no change, +1 for next gif, -1 for previous gif

    /*------------------
     * CORE :: LOOP
     * -----------------*/ 
        
void loop() {
  
  while (gif_player_begin == 0) {

    if (file.isOpen()) {
      uint32_t pos = 0;
      file.seekSet(pos);
    }
    
  // LVGL App
    timer.update();
    lv_task_handler(); /* let the GUI do its work */

    if (buttons > 0) {
      Serial.print("buttons_output: "); Serial.println(buttons);
    }
    delay(5);
    
    if (gif_player_begin == 1) {
      break;
    }
  }

  if (gif_player_begin == 1) {
    
    //x = 0;
    //nextGIF = 1;
    gif_player_begin = 2;
    Serial.println("R WE STILL IN THE MATRIX???");
    //continue;
    poll_num = 0;
  }  

  /*// mystery gif game
  if (gif_player_begin = 1) {
    //x = 0;
    nextGIF = 1;
    gif_player_begin = 2;
    Serial.println("R WE STILL IN THE MATRIX???");
    //continue;
  }*/
  
  // Gif Player App
  if (gif_player_begin == 2) {     
    
    if (arcada.recentUSB()) { 
      nextGIF = 1;  // restart when we get back
      return;       // prioritize USB over GIF decoding
    }

    // Check button presses
    arcada.readButtons();
    uint8_t buttons = arcada.justPressedButtons();
    if (buttons & KEY_LEFT) {                                                             // SHOWCASE MODE: ONLY ONE DIRECTORY IS CALLED - LEFT + RIGHT ENABLED
      //x = 0;                            // DIRECTORY == "/gifs"     // LEFT             // EMOJI MODE: Each direction UP DOWN LEFT RIGHT triggers different DIRECTORY
      if (mode_select == 4) {
        x = 2;//x=0
        nextGIF = 1;   // back
      }
    }
    
    if (buttons & KEY_RIGHT) {
      //x = 1;                            // DIRECTORY == "/favs"     // RIGHT
       if (mode_select == 4) {
          x = 3;//x=1
          nextGIF = 1; // --------------- // forward [A-Z PLAY MODE]
        }
    }
    
    if (buttons & KEY_UP) {
      //x = 1;                          // DIRECTORY == "/grmod"       // UP
        if (mode_select == 4) {
          x = 0; //x=2
          nextGIF = 1;    // forward
        } 
        if (mode_select == 2) {
          nextGIF = 1;
      }
    }
    
    if (buttons & KEY_DOWN) {
      //x = 0;                            // DIRECTORY == "/emojis"      // DOWN
        if (mode_select == 4) {
          x = 1;//x=3                     // ELSE [IF MODE SELECT NOT EQUAL TO 2] MODE SELECT = EMOJI MODE
          nextGIF = 1;                    // MOVE GIF < | > AND CAN CHNAGE DIRECTORY
        }      

       // A-Z :: GIF PLAY DIRECTION
        if (mode_select == 2) {           // MODE SELECT 2 == SHOWCASE MODE
          nextGIF = -1;                   // MOVE GIF < | > BUT CANNOT CHANGE DIRECTORY
        } 
    }
    
    if (buttons & KEY_ESC) {
      gif_player_begin = 0;
      if(last_icon == 2) {
        menu_games();
      } else {
        menu_main();
      }
    }
    
    if (buttons & KEY_ENTER) {
      pause = 1;
      pauseGIF(); 
    }

    if (poll_num == 0) {
      last_dir = 0; // last_dir = 0 | polled below to check how many active folders there are
      x = last_dir;
      Serial.print("last directory #: "); Serial.println(x);
      poll_num = 1;
    }
    if (poll_num == 1) {
      x = last_dir;
    }
    
    uint32_t now = millis();
    if (displayTimeSeconds != 8) {                                                    // dictates how long GIF plays if gif play time == 1, 10, 30 or 60
        if( ((now - fileStartTime) > (displayTimeSeconds * 1000)) &&
            (decoder.getCycleNo() > 1)) // at least one 'cycle' elapsed
        {
            nextGIF = 1;                                                              // cycle to next GIF if above displayTimeSeconds                               
        }
    }

     /* int gif_init = 0 :: initial startup :: if nextGIF is already set to 1, skip >>  */
    if (gif_init == 0) {
      if (nextGIF != 1) {
        //x = 0;
        nextGIF = 1;
      }
      Serial.print("DecoderCycle #: "); Serial.println(decoder.getCycleNo());
    }
    
    // nextGIF = 1 // on its own makes GIFs wiz by // display time has to be set to 8 [infinite]
    //if (gif_init == 0) {
      //Serial.println("WTF IS GOING ON???");
      //nextGIF = 1;
    //  gif_init = 1;
    //}

    if (prev_icon == 1 || prev_icon == 2 || prev_icon == 3) {
      Serial.print("FIRST prev icon: "); Serial.println(prev_icon);
      if (x !=0);
      x = 0;
      nextGIF = 1;
    }    

    /*changes directory and moves to nextGIF if app_num is triggered from directory menu :: does not move Gif on if picked directory is the same as current one*/
    /*BEGIN*/
    //directory 1
    Serial.print("app_num: "); Serial.println(app_num);
    if (app_num == 0) {
      if (active_dir1 == 1) {
        x = 0;                // sets new directory to match app number if new dir and current dir not the same
        nextGIF = 1;          // moves to nextGIF if folder number and directory different
        Serial.println("GIF folder 1");
      }
    }
    //directory 2
    if (app_num == 1) {
      if (active_dir2 == 1) {
        x = 1;
        nextGIF = 1;
        Serial.println("GIF folder 2");
      }
    }
    //directory 3
    if (app_num == 2) {
      if (active_dir3 == 1) {
        x = 2;
        nextGIF = 1;
        Serial.println("GIF folder 3");
      }
    }
    //directory 4
    if (app_num == 3) {
      if (active_dir4 == 1) {
        x = 3;
        nextGIF = 1;
        Serial.println("GIF folder 4");
      }
    }
    //play-all
    if (app_num == 4) {
      gif_init = 0;
      app_num = -1;
      Serial.println("play all");
    }
    /*END*/

    if (nextGIF != 0) {
      Serial.print("nextGIF: "); Serial.println(nextGIF);
      // Print the stats for this GIF      
      char buf[80];
      int32_t frames       = decoder.getFrameCount();
      int32_t cycle_design = decoder.getCycleTime();  // Intended duration
      int32_t cycle_actual = now - cycle_start;       // Actual duration
      int32_t percent = 100 * cycle_design / cycle_actual;
      sprintf(buf, "[%ld frames = %ldms] actual: %ldms speed: %ld%%",
              frames, cycle_design, cycle_actual, percent);
      Serial.println(buf);
    }

    if (nextGIF != 0) {
      cycle_start = now;
      if (! arcada.chdir(DIRECTORY[x])) {
        //arcada.errorBox("No '" DIRECTORY[x] "' directory found!\nPlease create it & continue");
        x = 0;
        //return;
      }
      
      int num_files = arcada.filesysListFiles(DIRECTORY[x], "GIF"); 
      if (y == 1) {
        randNumber = random(num_files); // linked to menu :: enables RANDOMISER
      }
      if (y == 2) { // linked to menu function :: turns RANDOMISER OFF
        randNumber = file_index;
      }
      //if (num_files == 0) {
      //  //arcada.errorBox("No GIF files found! Please add some & press (A) to continue");
      //  return;
      //}
      
      if (poll_num == 1) {
        
        // poll directory 1 // run once
        if (last_dir == 0 && num_files > 0) {
          active_dir1 = 1; //Serial.println("Folder 1 ACTIVE");
          dir_num = dir_num + 1;
          last_dir = 1;
          return;
        }
        if (last_dir == 0 && num_files == 0) {
          active_dir1 = 0; //Serial.println("Folder 1 INACTIVE");
          last_dir = 1;
          return;
        }

        // poll directory 2 // run once
        if (last_dir == 1 && num_files > 0) {
          active_dir2 = 1; //Serial.println("Folder 2 ACTIVE");
          dir_num = dir_num + 1;
          last_dir = 2;
          return;
        }
        if (last_dir == 1 && num_files == 0) {
          active_dir2 = 0; //Serial.println("Folder 2 INACTIVE");
          last_dir = 2;
          return;
        }

        // poll directory 3 // run once
        if (last_dir == 2 && num_files > 0) {
          active_dir3 = 1; //Serial.println("Folder 3 ACTIVE");
          dir_num = dir_num + 1;
          last_dir = 3;
          return;
        }
        if (last_dir == 2 && num_files == 0) {
          active_dir3 = 0; //Serial.println("Folder 3 INACTIVE");
          last_dir = 3;
          return;
        }

        // poll directory 4 // run once
        if (last_dir == 3 && num_files > 0) {
          active_dir4 = 1; //Serial.println("Folder 4 ACTIVE");
          dir_num = dir_num + 1;
          // no return here as we are done checking folders
        }
        if (last_dir == 3 && num_files == 0) {
          active_dir4 = 0; //Serial.println("Folder 4 INACTIVE");
          // no return here as we are done checking folders
        }
      }

      // # of folders
      Serial.print("Total # of folders: "); Serial.println(dir_num);

      // set base directory
      if (active_dir1 == 1) {
        base_dir = 0;
      }
      if (active_dir1 == 0 && active_dir2 == 1) {
        base_dir = 1;
      }
      if (active_dir1 == 0 && active_dir2 == 0 && active_dir3 == 1) {
        base_dir = 2;
      }
      if (active_dir1 == 0 && active_dir2 == 0 && active_dir3 == 0 && active_dir4 == 1) {
        base_dir = 3;
      }
      
      poll_num = 2;
      Serial.print("base directory: "); Serial.println(base_dir);
      
      if (dir_num == 0) {
        Serial.println("no GIFs found, please add some");
        Serial.print("number of folders: "); Serial.println(dir_num);
        return;
      }
      //return;

      // Determine how many animated GIF files exist
      Serial.print("Animated GIF files Found: ");  Serial.println(num_files);
      file_index += nextGIF;
      if (file_index >= num_files) {
        file_index = 0;                // wrap around to first file
        if (x < 4 && play_all == 1) {  // moves to next DIRECTORY [0-4] if all GIFs have been played
          
          if (prev_icon > 1) {
            x = base_dir;
          }
          
          else {
            x = x + 1;
            
            // directory 1 if no other directories active
            if (x == 1 && active_dir1 == 1 && active_dir2 == 0 && active_dir3 == 0 && active_dir4 == 0) {
              x = 4;
            }
            
            // directory 2 if no other directories active
            if (x == 2 && active_dir1 == 0 && active_dir2 == 1 && active_dir3 == 0 && active_dir4 == 0) {
              x = 4;
            }

            // directory 3 if no other directories active
            if (x == 3 && active_dir1 == 0 && active_dir2 == 0 && active_dir3 == 1 && active_dir4 == 0) {
              x = 4;
            }           
            
            // directory 2
            if (x == 1 && active_dir2 == 1) {
              x = 1;
            }
            if (x == 1 && active_dir2 == 0 && active_dir3 == 1) {
              x = 2;
            }
            if (x == 1 && active_dir2 == 0 && active_dir3 == 0 && active_dir4 == 1) {
              x = 3;
            }
            if (x == 1 && active_dir2 == 0 && active_dir3 == 0 && active_dir4 == 0) {
              x = 4;
            }

            // directory 2
            if (x == 2 && active_dir3 == 1) {
              x = 2;
            }
            if (x == 2 && active_dir3 == 0 && active_dir4 == 1) {
              x = 3;
            }
            if (x == 2 && active_dir3 == 0 && active_dir4 == 0) {
              x = 4;
            }

            // directory 3
            if (x == 3 && active_dir4 == 1) {
              x = 3;
            }
            if (x == 3 && active_dir4 == 0) {
              x = 4;
            }
          }
        }
        
        if (x == 4 && play_all == 1) {
          x = base_dir;
        }
        
        /* gif_init = 0 :: initial startup :: set directory to 0 [first folder] and sets gif_init to 0 so above gif_init query is not called again */ /* return for playall???*/
        if (gif_init == 0 && app_num == -1) {

          //set to first directory if avaialable
          if (active_dir1 == 1) {
            x = base_dir;
          }
          // second
          if (active_dir1 == 0 && active_dir2 == 1) {
            x = 1;
          }
          // third
          if (active_dir1 == 0 && active_dir2 == 0 && active_dir3 == 1) {
            x = 2;
          }
          // fourth
          if (active_dir1 == 0 && active_dir2 == 0 && active_dir3 == 0) {
            x = 3;
          }
        
          //x = 0;                        // set to first directory if array initially
          gif_init = 1;
          app_num = -1;
        }
        if (gif_init == 0 && app_num > -1) {
          gif_init = 1;                // gif_init set to outside range so not called again unless device restarted
          app_num = -1;
        }
        if (app_num > -1) {
          app_num = -1;                // returns to -1 so call does not repeat + reset for next menu trigger   
        }
        if (prev_icon == 1 || prev_icon == 2 || prev_icon == 3) {
          prev_icon = 0;
          Serial.print("LAST prev icon: "); Serial.println(prev_icon);
          Serial.print("DIRECTORY: "); Serial.println(x);
        }
      }
      
      if (file_index < 0) {
        file_index = num_files-1;      // wrap around to last file
      }
      nextGIF = 0; // and we're done moving between GIFs

      file = arcada.openFileByIndex(DIRECTORY[x], randNumber, O_READ, "GIF");         // randNumber picks random file based on how many files in directory :: file_index

      if (!file) {
        return;
      }

      arcada.display->dmaWait();
      arcada.display->endWrite();   // End transaction from any prior callback
      arcada.display->fillScreen(ARCADA_BLACK);
      decoder.startDecoding();

      // Center the GIF
      uint16_t w, h;
      decoder.getSize(&w, &h);
      Serial.print("Width: "); Serial.print(w); Serial.print(" height: "); Serial.println(h);
      if (w < arcada.display->width()) {
        gif_offset_x = (arcada.display->width() - w) / 2;
      } else {
        gif_offset_x = 0;
      }
      if (h < arcada.display->height()) {
        gif_offset_y = (arcada.display->height() - h) / 2;
      } else {
        gif_offset_y = 0;
      }

      // Note current time for terminating animation later
      fileStartTime = millis();
    }

  decoder.decodeFrame();
  }
}
    /*--------------------------
     *  Ticker library counter
     * -------------------------*/
void printCounter() {
  Serial.print("Counter ");
  Serial.println(timer.counter());
}
/***********************************************************************************/
    /*------------------
     * pause GIF
     * -----------------*/
void pauseGIF() {
  while (pause == 1) {
    arcada.readButtons();
    uint8_t buttons = arcada.justPressedButtons();
    if (buttons & KEY_ENTER) {
      pause = 0;
      break;
    }
  }
}
    /*------------------
     * Gif Player call
     * -----------------*/  
void gif_player() {

}
    /*----------------------------
     * Gif Player function calls
     * ---------------------------*/
void updateScreenCallback(void) {  }

void screenClearCallback(void) {  }

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
    arcada.display->drawPixel(x, y, arcada.display->color565(red, green, blue));
}

void drawLineCallback(int16_t x, int16_t y, uint8_t *buf, int16_t w, uint16_t *palette, int16_t skip) {
  uint16_t maxline = arcada.display->width();
  bool splitdisplay = false;
    
  uint8_t pixel;
  //uint32_t t = millis();
  x += gif_offset_x;
  y += gif_offset_y;
  if (y >= arcada.display->height() || x >= maxline ) {
    return;
  }
  if (w <= 0) return;

  //Serial.printf("Line (%d, %d) %d pixels skipping %d\n", x, y, w, skip);

  uint16_t buf565[2][w];
  bool first = true; // First write op on this line?
  uint8_t bufidx = 0;
  uint16_t *ptr;

  for (int i = 0; i < w; ) {
    int n = 0, startColumn = i;
    ptr = &buf565[bufidx][0];
    // Handle opaque span of pixels (stop at end of line or first transparent pixel)
    while((i < w) && ((pixel = buf[i++]) != skip)) {
      ptr[n++] = palette[pixel];
    }
    if (n) {
      arcada.display->dmaWait(); // Wait for prior DMA transfer to complete
      if (first) {
        arcada.display->endWrite();   // End transaction from prior callback
        arcada.display->startWrite(); // Start new display transaction
        first = false;
      }
      arcada.display->setAddrWindow(x + startColumn, y, min(maxline, n), 1);
      arcada.display->writePixels(ptr, min(maxline, n), false, true);
      bufidx = 1 - bufidx;
    }
  }
  arcada.display->dmaWait(); // Wait for last DMA transfer to complete
}

bool fileSeekCallback(unsigned long position) {
  return file.seek(position);
}

unsigned long filePositionCallback(void) { 
  return file.position(); 
}

int fileReadCallback(void) {
    return file.read(); 
}

int fileReadBlockCallback(void * buffer, int numberOfBytes) {
    return file.read((uint8_t*)buffer, numberOfBytes); //.kbv
}
