/*  
    P1X by Krzysztof Jankowski
    TinyRPG

    abstract: Experimental rpg game for 2x16 LCD display
    created: 14-08-2014
    license: do what you want and dont bother me

    webpage: http://p1x.in
    twitter: @w84death
*/
#include <ShiftLCD.h>
#define btn_left 0
#define btn_right 1
#define pin_data 3
#define pin_clock 2
#define pin_latch 4

#define screen_delay 100
#define screen_max 16
#define screen_min 0
#define screen_gui_row 0
#define screen_game_row 1
#define monolog_delay 1500

ShiftLCD lcd(pin_data, pin_clock, pin_latch);

unsigned long current_time;
long refresh_time = 0;
byte game_state = 0;

typedef struct {
  byte dot;
} Symbols;
Symbols symbols = {B10100101};

byte world_map[8][8];

typedef struct {
  byte life;
  byte sprite;
  byte pos_map;
  byte pos_screen;
} Entity;
Entity player = {1,2,0,2};

// SPRITES DATA
byte sprites_data[][8] = {
  { // P logo
    B11111,
    B10001,
    B10101,
    B10001,
    B10111,
    B10111,
    B10111,
    B11111,
  },{ // 1 logo
    B11111,
    B11011,
    B10011,
    B11011,
    B11011,
    B11011,
    B10001,
    B11111,
  },{ // X logo
    B11111,
    B10101,
    B10101,
    B11011,
    B10101,
    B10101,
    B10101,
    B11111,
  },{ // ornament 1
    B00000,
    B00100,
    B01010,
    B10010,
    B10000,
    B01100,
    B00011,
    B00000,
  },{ // ornament 2
    B00000,
    B00000,
    B00000,
    B00000,
    B00110,
    B01001,
    B10000,
    B00000,
  },{ // ornament 3
    B00000,
    B00000,
    B00000,
    B00000,
    B10110,
    B01001,
    B00000,
    B00000,
  },{ // ornament 4
    B00000,
    B00000,
    B00000,
    B00000,
    B01100,
    B10010,
    B00001,
    B00000,
  },{ // ornament 5
    B00000,
    B00100,
    B01010,
    B00001,
    B00001,
    B00110,
    B11000,
    B00000,
  },{ // heart
    B00000,
    B00000,
    B01010,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
  },{ // player
    B01110,
    B01010,
    B00100,
    B01110,
    B10101,
    B01110,
    B01010,
    B01010,      
  },{ // grass1
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B10110,
    B11111,
  },{ // grass2
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B01001,
    B11111,
  },{ // tree1
    B01110,
    B11111,
    B11111,
    B01110,
    B00100,
    B00100,
    B00100,
    B11111,
  },{ // tree2
    B00000,
    B00100,
    B01100,
    B01110,
    B11111,
    B11111,
    B00100,
    B11111,
  },{ // tree3
    B01001,
    B11110,
    B01101,
    B11110,
    B01111,
    B00100,
    B00100,
    B11111,
  },{ // high grass
    B00000,
    B00000,
    B00000,
    B00000,
    B00101,
    B10010,
    B01010,
    B11111,
  },{ // clear
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
  }
};

void setup()
{   
  pinMode(btn_left,INPUT);
  pinMode(btn_right,INPUT); 
 
  byte select[8] = {0,1,2,3,4,5,6,7};
  load_sprites(select);
  
  lcd.begin(16, 2);
  
  print_intro();
}
  
void load_sprites(byte select[])
{  
  for(byte i=0; i<8; i++){
    lcd.createChar(i, sprites_data[select[i]]);
  }
}

void print_intro()
{
  // TinyRPG by P1X
  lcd.setCursor(1, 0);
  lcd.print("TinyRPG");
  lcd.print(" by ");
  lcd.write(0);
  lcd.write(1);
  lcd.write(2);
  
  // ornaments
  lcd.setCursor(0, 1);
  lcd.write(3);
  lcd.write(4);
  for(byte i=0; i<12; i++){
      lcd.write(5);
  }
  lcd.write(6);
  lcd.write(7);
}

void blinking_press_btn(){
  lcd.setCursor(2, 1);
  if(current_time - screen_delay > refresh_time){
    if((current_time/1000) % 2 == 0){
      lcd.print("press");
      lcd.write(5);
      lcd.print("button");
    }else{
      for(byte i=0; i<12; i++){
        lcd.write(5);
      }
    }
   refresh_time = millis();
  }
}

void print_monolog(String txt_lines[])
{
  lcd.clear();
  for(byte i=0; i<sizeof(txt_lines); i++){
    lcd.setCursor(0,0);
    lcd.print(txt_lines[i]);
    if(sizeof(txt_lines) > i){
      lcd.setCursor(0,1);
      lcd.print(txt_lines[i+1]);
    }
    delay(monolog_delay);
  }
  lcd.clear();
}

void init_game(){
  byte select[8] = {8,9,10,11,12,13,14,15};
  load_sprites(select);
  
  String monolog[] = {
    "Welcome, hero!  ",
    "Go to the east. ",
    "You will find   ",
    "a willage.      ",
    "Ask there for ..",
    "..the quest!    ",
  };
  print_monolog(monolog);
   
  generate_map_screen();
  game_state = 1;
}

void print_gui()
{
  lcd.setCursor(0,screen_gui_row);
  lcd.print("[");
  lcd.write(0);
  if(player.life>1){
    lcd.write(0);
  }else{
    lcd.write(symbols.dot);
  }
  if(player.life>2){
    lcd.write(0);
  }else{
    lcd.write(symbols.dot);
  }
  lcd.print("]");
  lcd.setCursor(8,screen_gui_row);
  lcd.print("[");
  lcd.print(player.pos_map);
  lcd.print("/");
  lcd.print(player.pos_screen);
  lcd.print("]  ");
}

void generate_map_screen(){
  for(byte i=screen_min; i<screen_max; i++){
    world_map[player.pos_map][i] = random(2,7);
  }
}

void print_background()
{
  lcd.setCursor(0,screen_game_row);
  for(byte i=screen_min; i<screen_max; i++){
    lcd.write(world_map[player.pos_map][i]);
  }
}

void print_forground()
{
  // print player
  lcd.setCursor(player.pos_screen,screen_game_row);
  lcd.write(player.sprite);
  // print enemies
  // ...
}

void move_player(boolean go_left, boolean go_right)
{
  if(go_left && player.pos_screen > screen_min){
    player.pos_screen--;
  }
  if(go_right && player.pos_screen < screen_max){
    player.pos_screen++;
  }
}

void loop()
{
  current_time = millis();
  boolean refresh_screen = true;
  
  // intro
  if(game_state == 0){
     blinking_press_btn();     
     if(digitalRead(btn_left) || digitalRead(btn_right)){
       delay(150);
       init_game();
     }
  }
  
  if(game_state == 1){
    if(refresh_screen){
      print_gui();
      print_background();
      print_forground();
      refresh_screen = false;
    }
    if(digitalRead(btn_left) || digitalRead(btn_right)){
      delay(150);
      move_player(digitalRead(btn_left), digitalRead(btn_right));
      refresh_screen = true;
    }
  }
}
