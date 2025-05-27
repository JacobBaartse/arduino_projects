Servo myservo;

void setup_servo()
{
  
}

void do_servo(int pin, int position)
{
  myservo.attach(pin);
  myservo.write(position);
  delay(100);
  myservo.detach();

}
