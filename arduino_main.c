#include <TimerThree.h>
#include <LiquidCrystal_I2C.h>

// PIN
enum PINS {
  BUTTON_PIN = 8,
  END_BUTTON,
  BUZZER,
  // 삼색 LED
  LED_BLUE = 51,
  LED_GREEN,
  LED_RED,
  // shift register
  SERIAL_PIN = 45,
  LATCH_PIN,
  CLOCK_PIN,
  // joystick
  VR_X = A0,
  VR_Y = A1,
  SW = 7
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

// FND 비트 제어
const uint8_t bit_onoff[] = {
  0b11111100,
  0b11000000,
  0b10110110,
  0b11100110,
  0b11001010,
  0b01101110,
  0b01111110,
  0b11001100,
  0b11111110,
  0b11101110
};

// 도트
uint8_t rabbit[] = {
  0b01010,
  0b01010,
  0b01110,
  0b01110,
  0b00100,
  0b01110,
  0b01110,
  0b01010
};

uint8_t fruit[] = {
  0b00100,
  0b00100,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b01110,
  0b00100
};

uint8_t rotten[] = {
  0b01000,
  0b10010,
  0b01001,
  0b10010,
  0b01001,
  0b00000,
  0b01110,
  0b11111
};

// 변수
uint8_t button_state = 0;
uint8_t game_state = 0;
uint8_t fruit_time = 0;
uint8_t fruit_num = 0;
uint8_t player_pos[] = {0, 0};
uint8_t fruit_pos[] = {0, 2};
uint8_t rotten_pos[30][2] = {2, };

void setup() {
  Serial.begin(9600UL);

  // Timer
  Timer3.initialize(200000);
  Timer3.attachInterrupt(move_player);

  // PIN
  pinMode(BUTTON_PIN, INPUT);
  pinMode(END_BUTTON, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(SERIAL_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(VR_X, INPUT);
  pinMode(VR_Y, INPUT);
  pinMode(SW, INPUT);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.print(F("LCD init..."));

  delay(1000UL);
  lcd.clear();
  lcd.home();
  digitalWrite(LATCH_PIN, LOW);
  
  // 도트 생성
  lcd.createChar(0u, rabbit);
  lcd.createChar(1u, fruit);
  lcd.createChar(2u, rotten);

  // FND 초기화
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(SERIAL_PIN, CLOCK_PIN, LSBFIRST, 0b00000000);
  digitalWrite(LATCH_PIN, HIGH);

  digitalWrite(LATCH_PIN, LOW);
}

void loop() {
  // 타이틀
  lcd.setCursor(2, 0);
  lcd.print("RABBIT FRUIT");
  lcd.setCursor(7, 1);
  lcd.write(uint8_t(0));
  lcd.write(uint8_t(1));
  delay(3000UL);
  lcd.clear();

  // 대기 화면
  for(;;)
  {
    lcd.setCursor(0,0);
    lcd.print("PRESS BUTTON");
    lcd.setCursor(0,1);
    lcd.print("TO START...");
    lcd.write(uint8_t(0));
    lcd.print(" ");
    lcd.write(uint8_t(2));
    lcd.write(uint8_t(1));

    if(digitalRead(BUTTON_PIN) == LOW)
    {
      delay(300UL);
      lcd.clear();
      button_state = 1;
      break;
    }
  }

  // 게임 시작
  if(button_state == 1) {
    Serial.println("NAME_INPUT");
    String str = "0";

    // 이름 입력 대기
    for(;;)
    {
      lcd.setCursor(0, 0);
      lcd.print("PLEASE INPUT");
      lcd.setCursor(0, 1);
      lcd.print("YOUR NAME");

      if(Serial.available() > 0)
      {
        str = Serial.readStringUntil('\n');
      }
      
      if(str.equals("GAME_START"))
      {
        lcd.clear();
        break;
      }
    }

    lcd.setCursor(3,0);
    lcd.print("GAME START");
    lcd.setCursor(7, 1);
    lcd.write(uint8_t(0));
    lcd.write(uint8_t(1));
    delay(1500UL);
    lcd.setCursor(7, 1);
    lcd.print("  ");
    lcd.setCursor(8, 1);
    lcd.write(uint8_t(0));
    delay(1500UL);
    lcd.clear();
    fruit_time = 0;
    fruit_num = 0;
    game_state = 1;
    int start_time = millis()/1000;
    int stop_time = 0;
    uint8_t score = 0;

    while(game_state == 1)
    {
      delay(200UL);

      for(uint8_t i = 0; i < fruit_num; ++i)
      {
        if(rotten_pos[i][0] <= 15 && rotten_pos[i][1] <= 1)
        {
          lcd.setCursor(rotten_pos[i][0], rotten_pos[i][1]);
          lcd.write(uint8_t(2));
        }
      }

      if(player_pos[0] == fruit_pos[0] && player_pos[1] == fruit_pos[1])
      {
        fruit_pos[0] = 0;
        fruit_pos[1] = 2;
        ++score;
        --fruit_num;
      }

      fnd_score(score);

      // 게임 실패
      if(fruit_num == 4)
      {
        game_state = 2;
        stop_time = millis()/1000;
        int time = stop_time - start_time;
        Serial.print(score);
        Serial.print(",");
        Serial.print(fruit_num);
        Serial.print(",");
        Serial.println(time);
      }
      // 게임 성공
      else if(score == 9)
      {
        game_state = 0;
        stop_time = millis()/1000;
        int time = stop_time - start_time;
        Serial.print(score);
        Serial.print(",");
        Serial.print(fruit_num);
        Serial.print(",");
        Serial.println(time);
      }
    }
    
    // 게임 종료
    while(button_state)
    {
      // 게임 실패
      if(game_state == 2)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("YOU LOSE..");
        lcd.write(uint8_t(0));
        lcd.write(uint8_t(2));
        lcd.setCursor(0,1);
        lcd.print("RESTART? BUTTON");

        int tones[] = {391, 329, 261};
        for(int i = 0; i < 3; ++i)
        {
          tone(BUZZER, tones[i]);
          delay(300UL);
        }
        noTone(BUZZER);

        for(;;)
        {
          digitalWrite(LED_RED, HIGH);
          delay(300UL);
          digitalWrite(LED_RED, LOW);
          delay(300UL);
          if(digitalRead(BUTTON_PIN) == LOW)
          {
            all_clear(); 
            Serial.println("CONTINUE");
            break;
          }
          if(digitalRead(END_BUTTON) == LOW)
          {
            all_clear();
            Serial.println("STOP");
            delay(1000UL);
            exit(0);
          }
        }
      } 
      // 게임 성공
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("YOU WIN!!");
        lcd.write(uint8_t(0));
        lcd.write(uint8_t(1));
        lcd.setCursor(0, 1);
        lcd.print("RESTART? BUTTON");
        
        int tones[] = {261, 329, 391};
        for(int i = 0; i < 3; ++i)
        {
          tone(BUZZER, tones[i]);
          delay(300UL);
        }
        noTone(BUZZER);

        for(;;)
        {
          digitalWrite(LED_BLUE, HIGH);
          delay(300UL);
          digitalWrite(LED_BLUE, LOW);
          delay(300UL);
          if(digitalRead(BUTTON_PIN) == LOW)
          {
            all_clear(); 
            Serial.println("CONTINUE");
            break;
          }
          if(digitalRead(END_BUTTON) == LOW)
          {
            all_clear();
            Serial.println("STOP");
            delay(1000UL);
            exit(0);
          }
        }
      }
    }
  }
}
