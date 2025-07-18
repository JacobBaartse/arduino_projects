Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

void setup_servo()
{
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates
}

void do_servo(int pin, int position) //pin number on pwm extender
{
  pwm.setPWM(pin, 0, (position*2)+90);  // pwm start is 90 pwm end is 
  delay(10);
}
