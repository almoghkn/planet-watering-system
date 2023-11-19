int low=208; // 100% moisture
int up=973; // 0% moisture

int red_led = 13;
bool flag;

void watering();
String serialBuff ;
int low_thr_precent; // low thr in %
int up_thr_precent; // up thr in %
int low_thr_sensor; // low thr sensor 0-1023
int up_thr_sensor; // up thr sensor 0-1023
int SensorValue;

// Motor A connections
int enA = 9;
int in1 = 8;
int in2 = 7;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  
  // Set all the motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(red_led, OUTPUT);
  
  // Turn off motors - Initial state
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  low_thr_precent=2000;
  up_thr_precent=2000;
  up_thr_sensor=2000;
  flag= true;

  // TIMER 1 for interrupt frequency 0.5 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 0.5 Hz increments
  OCR1A = 31249; // = 16000000 / (1024 * 0.5) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts             

  while (!Serial);
}

void loop() {
  if(SensorValue>up_thr_sensor){ // check if need to water the planet
    watering();
    Serial.println(100-(((float(SensorValue)-low)/(up-low))*100)); // send moisture to raspberry
  }
  else if(SensorValue<low_thr_sensor && flag ){ // check if moisture > up thr
    digitalWrite(red_led, HIGH); // turn on red led
    flag=false;  
  }
  else{ // moisture between thr
    flag=true;
    digitalWrite(red_led, LOW); // turn off red led
  }
}

ISR(TIMER1_COMPA_vect){ 
  static int counter=0;
  counter+=1;
  if(counter==300) // count to reach 10 min
  { 
    counter=0;
    SensorValue = analogRead(A0); // get moisture sensor value 
    Serial.println(100-(((float(SensorValue)-low)/(up-low))*100)); // send moisture to raspberry
    //Serial.println(100-(float(SensorValue)/1023)*100);
    //Serial.println(SensorValue);
    TCCR1A = 0; // set entire TCCR1A register to 0
    TCCR1B = 0; // same for TCCR1B
    TCNT1  = 0; // initialize counter value to 0
    // set compare match register for 0.5 Hz increments
    OCR1A = 31249; // = 16000000 / (1024 * 0.5) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS12, CS11 and CS10 bits for 1024 prescaler
    TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
  }
}

void watering(){
  cli();
  int avg_moisture = ((up_thr_sensor-low_thr_sensor)/2)+low_thr_sensor; // calc avg thr moisture
  SensorValue = analogRead(A0);
  while(SensorValue > avg_moisture){ // watering until avg moisture
    digitalWrite(in1, LOW); // open solenoid
    digitalWrite(in2, HIGH); // open solenoid
    analogWrite(enA, 1); // PWM = 1
    SensorValue = analogRead(A0);
  }
  digitalWrite(in1, LOW); // stop watering close solenoid
  digitalWrite(in2, LOW); // stop watering close solenoid
  sei();
}

void serialEvent() {
  while (Serial.available()) { // check for serial data
    char in_char = Serial.read(); // receive data
    serialBuff += String(in_char); // save data in string
    
    if (serialBuff.endsWith("\r")) {
      if(serialBuff.startsWith("low")) // set low thr moisture
      {
        low_thr_precent=serialBuff.substring(3).toInt();
        up_thr_sensor=((float(100-low_thr_precent)/100)*(up-low))+low;
        
      }
      if(serialBuff.startsWith("up")) // set up moisture
      {
        up_thr_precent=serialBuff.substring(2).toInt();
        low_thr_sensor=((float(100-up_thr_precent)/100)*(up-low))+low; 
      }
      serialBuff = ""; 
    }
  }
}
