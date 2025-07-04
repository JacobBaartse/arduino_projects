void setup() {
  // put your setup code here, to run once:
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  Serial.begin(115200);

}

int val2=0;
int val3=0;
int val4=0;
int p_val2=0;
int p_val3=0;
int p_val4=0;

void print_values(){
  Serial.print(val2);
  Serial.print(val3);
  Serial.println(val4);
}

void loop() {
  // put your main code here, to run repeatedly:
  val2 = digitalRead(2);
  val3 = digitalRead(3);
  val4 = digitalRead(4);
  if (val2 != p_val2) print_values();
  if (val3 != p_val3) print_values();
  if (val4 != p_val4) print_values();
  p_val2 = val2;
  p_val3 = val3;
  p_val4 = val4;
  //delay(20);
}
