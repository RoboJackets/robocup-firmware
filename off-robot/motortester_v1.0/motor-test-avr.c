//motor-test-avr.c
#include <motor-test-avr.h>
#include <motor-test-lcd.h>


DebugInfo debugInfo;
#define lcdPort         PORTD
#define lcdDdr          DDRD
#define motorPort       PORTA
#define motorDdr        DDRA


//set pins 34, 35, 36 as outputs
DDRA &= ~(1 << PIND34); // sets port 34 of the motor as an output
DDRA &= ~(1 << PIND35); // sets port 35 of the motor as an output
DDRA &= ~(1 << PIND36); // sets port 36 of the motor as an output

//pins 30, 31, 32 set as inputs by default (along with 33 and 37)

/* test characteristics
Hall sensor abc - Phases UYW
001-10/
101-1/0
100-/10
110-01/
010-0/1
011-/01
*/

long[] phase1 = {5, 0, 2.5}; //the first is high low float across each of the motors three phases
long[] phase5 = {5, 2.5, 0}; //the fifth is high float low
long[] phase4 = {2.5, 5, 0}; //the fourth is float high low
long[] phase6 = {0, 5, 2.5}; //the sixth is low high float
long[] phase2 = {0, 2.5, 5}; //the second is low float high
long[] phase3 = {2.5, 0, 5}; //the third is float low high





int main(void ) {
   
   initMotorTest();
   //iteratively write hall values, and check that voltages at the sensors are correct
   for (int command = 1; command <= 6; command++) {
        //each command is a number, 
        //nextCommand converts that number into binary and writes it to the hall sensors
        nextCommand(command);
   }
   /*
        IODIR = 0x00,
        IPOL = 0x02,
        GPINTEN = 0x04,
        DEFVAL = 0x06,
        INTCON = 0x08,
        IOCON = 0x0A,
        GPPU = 0x0C,
        INTF = 0x0E,
        INTCAP = 0x10,
        GPIO = 0x12,
        OLAT = 0x14
    */
}

void initMotorTest(void) {
    motorDdr |= 0x0E;
}

bool nextCommand(int command) {
    int val = [floor(command / 4), floor((command % 4) / 2), floor((command % 2) / 1)]; //puts the command in binary
    MotorPort = MotorDdr | (val << 4); //writes the command to the hall sensors
    bool * acc= checkInstruction(command); //reads in voltages for the motor and checks that they are correct
    //write LCD
    char[] top_row = ['p', 'h', 's', '1', ' ', 'p,', 'h', 's', '2', ' ', 'p', 'h', 's', '3'];
    char[] t = ['p', 'a', 's', 's'];
    char[] f = ['f', 'a', 'i', 'l'];
    for (int i = 0, i < length(top_row), i++)
    {
        LCD_goto(i, 0);
        LCD_print(top_row[i]);
        LCD_goto(i, 1);
        if (i % 5 == 0)
            acc[i/5] == true ? LCD_print(t) : LCD_print(f);
    }
    free(acc);
    return 1;

}
bool* checkInstruction(int command) {
    int hallVal = convert(halls);
    bool* accuracy = maloc(3 * sizeof(int));
    //note: not sure how to handle checking the float values
    switch(hallVal){
        case 1:
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase1[2] - analog_phases[2]) < 0.5,
                             abs(phase1[1] - analog_phases[1]) < 0.5,
                             abs(phase1[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        case 2: 
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase2[2] - analog_phases[2]) < 0.5,
                             abs(phase2[1] - analog_phases[1]) < 0.5,
                             abs(phase2[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        case 3:
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase3[2] - analog_phases[2]) < 0.5,
                             abs(phase3[1] - analog_phases[1]) < 0.5,
                             abs(phase3[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        case 4:
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase4[2] - analog_phases[2]) < 0.5,
                             abs(phase4[1] - analog_phases[1]) < 0.5,
                             abs(phase4[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        case 5:
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase5[2] - analog_phases[2]) < 0.5,
                             abs(phase5[1] - analog_phases[1]) < 0.5,
                             abs(phase5[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        case 6:
            analog_phases = (motorPort & !motorDdr);
            accuracy = [abs(phase6[2] - analog_phases[2]) < 0.5,
                             abs(phase6[1] - analog_phases[1]) < 0.5,
                             abs(phase6[0] - analog_phases[0]) < 0.5];
            return accuracy;
            break;
        default:
            accuracy = [000];
            return accuracy;
            break;
        }

}


/*
//start the clock
  cli();            //Disable global interrupts
  TCCR1B |= 1<<CS11 | 1<<CS10;  //Divide by 64
  OCR1A = 15624;        //Count 15624 cycles for 1 second interrupt
  TCCR1B |= 1<<WGM12;     //Put Timer/Counter1 in CTC mode
  TIMSK1 |= 1<<OCIE1A;        //enable timer compare interrupt
  sei(); 
  //Setup the I/O for the LED
 
  DDRD |= (1<<0);     //Set PortD Pin0 as an output
  PORTD |= (1<<0);        //Set PortD Pin0 high to turn on LED
 
  while(1) { }          //Loop forever, interrupts do the rest
}
 
ISR(TIMER1_COMPA_vect)      //Interrupt Service Routine
{
  PORTD ^= (1<<0);        //Use xor to toggle the LED
}
  */