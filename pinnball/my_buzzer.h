/*
 * Author: Everton Ramires
 * More Tutorial: https://www.youtube.com/channel/UC7zG4YQJc8v-9jNGQknOx5Q
 * 
 * 
 
 CURRENT SOUND OPTIONS: 
 (PT-BR)OPÇÕES ATUAIS DE SONS:
 
 S_CONNECTION   S_DISCONNECTION S_BUTTON_PUSHED   
 S_MODE1        S_MODE2         S_MODE3     
 S_SURPRISE     S_OHOOH         S_OHOOH2    
 S_CUDDLY       S_SLEEPING      S_HAPPY     
 S_SUPER_HAPPY  S_HAPPY_SHORT   S_SAD       
 S_CONFUSED     S_FART1         S_FART2     
 S_FART3        S_JUMP 20

 */

#define BUZZER_PIN 8
int volumePin = A0;  
int volume = 75;

void buzzer_setup() 
{
  cute.init(BUZZER_PIN);
  pinMode(volumePin, OUTPUT);         // stel de volumePin in als uitvoer
  analogWrite(volumePin, volume);     // apply the volume
  cute.play(1);
}

void cute_buzzer(int i){
  cute.play(i);
}

void demo_loop() 
{
  for (int i =0 ; i< 21; i++)
  {
    Serial.println(i);
    cute.play(i);
    delay(2000);
  }
}
