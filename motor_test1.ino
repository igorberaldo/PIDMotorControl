#include <DueTimer.h>
#define pwm_pin 2
#define direction_pin 24
#define d2_pin 22
#define command_length 100
#define interruptPinA 20
#define interruptPinB 21

char command[command_length];
int counter, controller;
char c;
int pwm_value = 0;
signed int pos = 0;
long int T = 5e4;
double Tb = 0.05;
double input, Kp1 = 0.0, Kp, Ki1 = 0.0, Ki, Kd1 = 0.0, Kd, y = 0.0, y_ant = 0.0, x = 0.0, x_ant = 0.0, x_2ant = 0.0;
bool motor = LOW;
volatile long int encoderPos = 0;
long int difPulses, encoderPosAnt=0;
volatile bool pulsesA = false;
volatile bool pulsesB = false;
double degree = 0.0;
double speedm1 = 0.0;
double pulses = 0.0;
char type;
bool started = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(pwm_pin, OUTPUT);
  pinMode(d2_pin, OUTPUT);
  pinMode(direction_pin, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  digitalWrite(d2_pin, LOW);
  while(!Serial){}

  pinMode(interruptPinA, INPUT_PULLUP);
  pinMode(interruptPinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinA), interruptA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinB), interruptB, CHANGE);
  Timer1.attachInterrupt(funTimer).setPeriod(T).start();    
}

void loop() {
  // put your main code here, to run repeatedly:

}


void serialEvent()
{
  while(Serial.available())
  {
    c = Serial.read();
    if(c == 13)
    {
      command[counter] = '\0';
      counter = 0;
      analizeString();
    }
    else
    {
      command[counter] = c;
      counter++;
      if(counter == command_length) counter = 0;
    }
  }
}

void analizeString()
{    
   switch (command[0])
   {   
      case 'S': //stop
          Serial.println("The system is off");
          digitalWrite(d2_pin, LOW);
          analogWrite(pwm_pin, 0);
          Timer2.stop();
          started = false;
          break;
      case 'C':
           digitalWrite(direction_pin, HIGH);           
           break;
      case 'A':
           digitalWrite(direction_pin, LOW);  
           break;
      case 'I': // Start
           Serial.println("The system is on");
           digitalWrite(d2_pin, HIGH);           
           analogWrite(pwm_pin, pwm_value);
           started = true;
           break;
      case 'P':
            controller = atof(&command[1]);
            input = atof(&command[3]);       
            type = command[2];
            break;
      case 'G':
           Kp = atof(&command[1]);  
           Ki = atof(&command[2]);
           Kd = atof(&command[3]);
           break;
      case 'L':
           controller = atof(&command[1]);
           switch(controller)
           {
            case 1:
              if(type == 's')
              {
                Kp =  0.0705983413499644;
              } 
              else if(type == 'd')
              {
                Kp = 0.0930679793022759;
              }
              break;
            case 2:
              if(type == 's')
              {
                Kp = 0.0124435163577445; 
                Ki = 0.497740654309778;
              } 
              else if(type == 'd')
              {
                Kp = 0.0883; 
                Ki = 0.0138;
              }
              break;
            case 3:
              if(type == 's')
              {
                Kp = 0.08;
                Kd = -0.00107;
              } 
              else if(type == 'd')
              {
                Kp = 0.104;
                Kd = 0.00503;
              }
              break;
           case 4:
              if(type == 's')
              {
                Kp = 0.0623;
                Ki = 1.24;
                Kd = 1.16e-5;
              } 
              else if(type == 'd')
              {
                Kp = 0.102;
                Ki = 0.0218;
                Kd = 0.00473;
              }
              break;
            default:
              break;
           }  
           break;     
      case 'T':
           pwm_value = atof(&command[1]);
           if(pwm_value >= 0 || pwm_value <= 4095)    
           analogWrite(pwm_pin, pwm_value);
           else
              Serial.println("ERROR");           
           break;      
     /*case 'O': // INPUTS            
           input = atof(&command[1]);       
           type = command[2];           
           break;   */
     case 'R': // RANDOM STEPS           
           input = atof(&command[1]);       
           type = command[2];                     
           digitalWrite(d2_pin, HIGH); 
           pwm_value = (input *4095)/6;            
           if(input > 0.0 )
           {
              digitalWrite(direction_pin, HIGH);             
           }
           else 
           {
            digitalWrite(direction_pin, LOW);       
            pwm_value = pwm_value * -1;     
           }
           analogWrite(pwm_pin, pwm_value);
           break;          
     case 'Z': // ZERO            
           degree = 0.0;
           encoderPos = 0;  
           y = 0.0;
           x = 0.0;
           pwm_value = 0;   
           speedm1 = 0.0;
           break;
     default:       
           controller = 0;               
           break;           
   }
}

void interruptA(){
  pulsesA = digitalRead(interruptPinA) == HIGH;
  if(pulsesA!=pulsesB) 
  {
    encoderPos++;
  }
  else
  {    
    encoderPos--;
  }
}
void interruptB(){
  pulsesB = digitalRead(interruptPinB) == HIGH;
  if(pulsesA==pulsesB) 
  {
    encoderPos++;
  }
  else 
  {
    encoderPos--;  
  }
}

void funTimer(){
  degree = (double)encoderPos*1.419;  
  pos = (int)degree % 360;
  //degree = pos;
  difPulses = encoderPosAnt - encoderPos;
  encoderPosAnt = encoderPos;
  speedm1 = 0.2365e6*((double)difPulses/T);   
  Serial.print(millis());
  Serial.print(' ');
  Serial.print(degree);
  Serial.print(' ');
  Serial.print(speedm1); 
  Serial.print(' ');
  Serial.print(input);   
  Serial.print(' ');
  Serial.print(x/input);    
  Serial.print(' ');  
  Serial.println(y);
  if(!started)  controlador();
}

void controlador()
{     
  double pwm_value2;
  if(type == 's')
  { 
    switch(controller)
    {      
      case 0://None  
        //digitalWrite(d2_pin, LOW);
        analogWrite(pwm_pin, 0);     
        y = 0;    
        x = 0;
        break;
      case 1://P
        //Kp = 0.0705983413499644;
        x = input - speedm1;
        y = Kp * x;
        digitalWrite(d2_pin, HIGH);            
        break;
      case 2://PI
        //Kp = 0.0124435163577445; 
        //Ki = 0.497740654309778;  
        x = input - speedm1;
        y = Kp * (x - x_ant) + (Ki*Tb)/2 *(x + x_ant) + y_ant;              
        x_ant = x;
        y_ant = y;
        digitalWrite(d2_pin, HIGH); 
        break;
      case 3://PD
        digitalWrite(d2_pin, HIGH); 
        //Kp = 0.08;
        //Kd = -0.00107;        
        x = input - speedm1;
        y = x * (Kp + (Kd/Tb)) - Kd/Tb * x_ant; 
        x_ant = x; 
        break;
      case 4://PID
        digitalWrite(d2_pin, HIGH); 
        //Kp = 0.0623;
        //Ki = 1.24;
        //Kd = 1.16e-5;        
        x = input - speedm1;
        y = y_ant + (Kp + (Ki*Tb)/2 + Kd/Tb) * x - (Kp - (Ki*Tb)/2 + (2*Kd)/Tb) * x_ant + Kd/Tb * x_2ant;
        y_ant = y;      
        x_2ant = x_ant; 
        x_ant = x;
        break;
      default:        
        digitalWrite(d2_pin, LOW);
        break;   
    }
    if(y > -0.6 && y < 0)
    {
      y = -0.6;
    }
    else if( y < 0.6 && y > 0)
    {
      y = 0.6;
    }
    if( y > 6) y = 6;
    if(y < -6) y = -6;
    pwm_value2 = (y *4095)/6;
    if(pwm_value2 > 0.0 )
        {
          digitalWrite(direction_pin, HIGH);   
          if(pwm_value2 > 4095)  pwm_value2 = 4095; 
        }
        else
        {
          digitalWrite(direction_pin, LOW); 
          pwm_value2 = pwm_value2 * -1; 
          if(pwm_value2 > 4095)  pwm_value2 = 4095;
        }    
        analogWrite(pwm_pin, pwm_value2);      
  }
  else if(type == 'd')
  {       
    switch(controller)
    {
      case 0://None
        //digitalWrite(d2_pin, LOW);
        analogWrite(pwm_pin, 0);     
        y = 0;    
        x = 0;
        break;
      case 1://P
        //Kp = 0.0930679793022759;       
        x = input - degree;
        y = Kp * x ;        
        digitalWrite(d2_pin, HIGH);            
        break; 
      case 2://PI
        //Kp = 0.0883; 
        //Ki = 0.0138;  
        x = input - degree;
        y = Kp * (x - x_ant) + (Ki*Tb)/2 *(x + x_ant) + y_ant;              
        x_ant = x;
        y_ant = y;
        digitalWrite(d2_pin, HIGH); 
          break; 
        case 3://PD
        digitalWrite(d2_pin, HIGH); 
        //Kp = 0.104;
        //Kd = 0.00503;        
        x = input - degree;
        y = x * (Kp + (Kd/Tb)) - Kd/Tb * x_ant; 
        x_ant = x; 
          break;
      case 4://PID
        digitalWrite(d2_pin, HIGH);
        //Kp = 0.102;
        //Ki = 0.0218;
        //Kd = 0.00473;        
        x = input - degree;
        y = y_ant + (Kp + (Ki*Tb)/2 + Kd/Tb) * x - (Kp - (Ki*Tb)/2 + (2*Kd)/Tb) * x_ant + Kd/Tb * x_2ant;
        y_ant = y;      
        x_2ant = x_ant; 
        x_ant = x;
        break;
      default:
        break;
    }    
    if(y > -0.6 && y < 0)
    {
      y = -0.6;
    }
    else if( y < 0.6 && y > 0)
    {
      y = 0.6;
    }
    if( y > 6) y = 6;
    if(y < -6) y = -6;
    pwm_value2 = (y * 4095)/6;
    if(pwm_value2 > 0.0 )
        {
          digitalWrite(direction_pin, LOW);   
          if(pwm_value2 > 4095)  pwm_value2 = 4095; 
        }
        else
        {
          digitalWrite(direction_pin, HIGH); 
          pwm_value2 = pwm_value2 * -1; 
          if(pwm_value2 > 4095)  pwm_value2 = 4095;
        }    
        analogWrite(pwm_pin, pwm_value2); 
  } 
}




