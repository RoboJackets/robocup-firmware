// the idea of the code is to get data from the movement of a ball as it passes four sensors. 
//  the code intends to get the time the ball passes by every sensor, including when it comes in and out of it.
//  it also calculates the mean speed, accelaration, and net force acting on it

// max speed the ball can reach is ~6m/s. for an spacing of 5cm between sensors it will take <8.333ms for the ball to reach the next sensor. 
//    with 10μs periods, the percent error would be 0.036.

const int r_sensors = 0.05; //distance between sensors (m)
const int r_beam = 0.000001; //IDK, check the used component data sheet for actual size of light beam ~this value (1μm) is a guess~
const int mass_ball = 45.93*(10^-3); // mass of the golf ball (kg)

const int sensor1 = 1; //TX -> sensor 1
const int sensor2 = 0;//RX -> sensor 2
const int sensor3 = 2; //SDA -> sensor 3
const int sensor4 = 3; //SCL -> sensor 4

unsigned long time_sensor[8]; //time sensor was triggered (μs)
int i = 0;
int k = 0;

float v_rsensor[3];
float v_rbeam[4];
float vmean;
float a;
float fnet_friction;

const int ledPin =  LED_BUILTIN;
const long interval = 0.001;  //interval for led blink (miliseconds)
int ledState = LOW; 

void setup() {  
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(sensor1), interrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensor2), interrupt2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensor3), interrupt3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensor4), interrupt4, CHANGE);
}

// reading sensors and saving times ball passes
void loop() {
  if (ledState == 1){
    digitalWrite(ledPin, ledState);
    delay(100);
    ledState = LOW;
  }
  if (i==8){
    calculations();
   
  }
}
// an interrupt for each sensor
void interrupt1 (){
  time_sensor[i] = micros();
  ledState = HIGH;
  i++;
}
void interrupt2 (){
  time_sensor[i] = micros();
  ledState = HIGH;
  i++;
}
void interrupt3 (){
  time_sensor[i] = micros();
  ledState = HIGH;
  i++;
}
void interrupt4 (){
  time_sensor[i] = micros();
  ledState = HIGH;
  i++;
}

// calculating variables
void calculations(){
  for (int j=0; j<8; j+=2) {
    
    if (k<3){ //v_rsensor has size 3, and v_rbeam size 4, then, in the last loop, we shouldn't record a velocity for v_rsensor.
      
      v_rsensor[k] = r_sensors/((time_sensor[j+2]-time_sensor[j])*10^-6);
      v_rbeam[k] = r_beam/((time_sensor[j+1]-time_sensor[j])*10^-6);
      vmean += v_rsensor[k]+v_rbeam[k];
    } 
    else{
    
      v_rbeam[k] = r_beam/((time_sensor[j+1]-time_sensor[j])*10^-6);
      vmean += v_rbeam[k];
    } 
    
    k++;
  }
  
  vmean = (vmean)/7;
  a = (v_rbeam[3]-v_rsensor[0])/((time_sensor[6]-time_sensor[7])*10^-6);

 Serial.print("Mean speed: ");
 Serial.print(vmean);
 Serial.print("\nMean accelaration: ");
 Serial.print(a);
}
