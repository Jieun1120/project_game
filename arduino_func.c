void move_player()
{
  interrupts();
  if(game_state == 1)
  {
    fruit_time += 2;
    // 조이스틱 조작
    uint16_t vr_x = analogRead(VR_X);
    uint16_t vr_y = analogRead(VR_Y);
    uint8_t player_move = 1;
    lcd.setCursor(player_pos[0], player_pos[1]);
    lcd.write(uint8_t(0));
    if(vr_x == 0 && vr_y >= 300 && vr_y <= 800)  // 왼쪽
    {
      if(player_pos[0] > 0)
      {
        for(uint8_t i = 0; i < fruit_num; ++i)
        {
          if((player_pos[0] - 1) == rotten_pos[i][0] && player_pos[1] == rotten_pos[i][1])
          {
            player_move = 0;
            break;
          }
        }
        if(player_move)
        {
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.print(" ");
          --player_pos[0];
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.write(uint8_t(0));
        }
      }
    }
    else if(vr_x >= 1020 && vr_x <= 1023 && vr_y >= 300 && vr_y <= 800)  // 오른쪽
    {
      if(player_pos[0] < 15)
      {
        for(uint8_t i = 0; i < fruit_num; ++i)
        {
          if((player_pos[0] + 1) == rotten_pos[i][0] && player_pos[1] == rotten_pos[i][1])
          {
            player_move = 0;
            break;
          }
        }
        if(player_move)
        {
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.print(" ");
          ++player_pos[0];
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.write(uint8_t(0));
        }
      }
    }
    else if(vr_x >= 300 && vr_x <= 800 && vr_y == 0)  // 위
    {
      if(player_pos[1] > 0)
      {
        for(uint8_t i = 0; i < fruit_num; ++i)
        {
          if(player_pos[0] == rotten_pos[i][0] && (player_pos[1] - 1) == rotten_pos[i][1])
          {
            player_move = 0;
            break;
          }
        }
        if(player_move)
        {
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.print(" ");
          --player_pos[1];
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.write(uint8_t(0));
        }
      }
    }
    else if(vr_x >= 300 && vr_x <= 800 && vr_y >= 1020 && vr_y <= 1023)  // 아래
    {
      if(player_pos[1] < 1)
      {
        for(uint8_t i = 0; i < fruit_num; ++i)
        {
          if(player_pos[0] == rotten_pos[i][0] && (player_pos[1] + 1) == rotten_pos[i][1])
          {
            player_move = 0;
            break;
          }
        }
        if(player_move)
        {
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.print(" ");
          ++player_pos[1];
          lcd.setCursor(player_pos[0], player_pos[1]);
          lcd.write(uint8_t(0));
        }
      }
    }

    // 과일 생성
    if(fruit_time == 20) {
      lcd.setCursor(fruit_pos[0], fruit_pos[1]);
      lcd.print(" ");
      rotten_pos[fruit_num][0] = fruit_pos[0];
      rotten_pos[fruit_num][1] = fruit_pos[1];

      // for(uint8_t i = 0; i < fruit_num; ++i)
      // {
      //   if(rotten_pos[i][0] <= 15 && rotten_pos[i][1] <= 1)
      //   {
      //     lcd.setCursor(rotten_pos[i][0], rotten_pos[i][1]);
      //     lcd.write(uint8_t(2));
      //   }
      // }

      make_fruit();
      fruit_time = 0;
      ++fruit_num;
    }
  }
  noInterrupts();
}

// 과일 생성 함수
void make_fruit()
{
  fruit_pos[0] = random(0, 16);
  fruit_pos[1] = random(0, 2);
  lcd.setCursor(fruit_pos[0], fruit_pos[1]);
  lcd.write(uint8_t(1));
}

// FND 점수 출력 함수
void fnd_score(uint8_t score)
{
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(SERIAL_PIN, CLOCK_PIN, LSBFIRST, bit_onoff[score]);
  digitalWrite(LATCH_PIN, HIGH);
}

// reset 함수
void all_clear()
{
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(SERIAL_PIN, CLOCK_PIN, LSBFIRST, 0b00000000);
  digitalWrite(LATCH_PIN, HIGH);
  digitalWrite(LATCH_PIN, LOW);
  player_pos[0] = 0;
  player_pos[1] = 0;
  fruit_pos[0] = 0;
  fruit_pos[1] = 2;
  for(uint8_t i = 0; i < 30; ++i)
  {
    rotten_pos[i][0] = 2;
    rotten_pos[i][1] = 2;
  }
  game_state = 0;
  button_state = 0;
  lcd.clear();
}
