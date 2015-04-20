//Author: Ji Hye Lee 

//Define the analog input pins for sensors 
const int flex_thumb = A5;
const int flex_index = A4; 
const int flex_middle = A3; 
const int led = 13; 

//**********************************************************************
//needs more testing to find more reliable bias
const int bias = 20; 
//**********************************************************************

//Needs more testing to find more reliable number. 
/*
const int index_straight = 525; 
const int index_curled = 730; 
*/

// Assign the value of a first analog read. 
//resistance is the highest --> the lowest value 
int thumb_curled; 
int index_curled; 
int middle_curled; 

//resistance is the lowest --> the highest value. 
int thumb_straight = 0;
int index_straight = 0; 
int middle_straight = 0; 

//**********************************************************************
//For testing 
const boolean debug = true; 
//**********************************************************************

//Variables for holding sensor data 

int thumb_data;   //flex data for the thumb
int index_data;   //flex data for the index 
int middle_data;  //flex data for the middle finger. 

void setup() 
{
  //setup serial connection to computer 
  Serial.begin (9600); 

  //set Pin Directions
  pinMode (flex_thumb, INPUT); 
  pinMode (flex_index, INPUT);
  pinMode (flex_middle, INPUT); 
  pinMode (led, OUTPUT); 
  
  thumb_curled = analogRead(flex_thumb); 
  index_curled = analogRead(flex_index); 
  middle_curled = analogRead(flex_middle); 

  Serial.println ("Give me a queue :D : "); 
}

void loop()
{
  //read 5V Sensor Values
  analogReference(DEFAULT); 
  thumb_data = analogRead(flex_thumb); 
  index_data = analogRead(flex_index); 
  middle_data = analogRead(flex_middle);  

  //Calibration for (max value) 
  if (thumb_data > thumb_straight)  
    thumb_straight = thumb_data; 
  if (index_data > index_straight)
    index_straight = index_data; 
  if (middle_data > middle_straight) 
    middle_straight = middle_data; 
    
  //Calibration for min value 
  if (thumb_data < thumb_curled)
    thumb_curled = thumb_data; 
  if (index_data < index_curled) 
    index_curled = index_data; 
  if (middle_data < middle_curled) 
    middle_curled = middle_data; 

  //Map values 
  thumb_data = map (thumb_data, thumb_straight, thumb_curled, 0, 100); 
  index_data = map (index_data, index_straight, index_curled, 0, 100); 
  middle_data = map (middle_data, middle_straight, middle_curled, 0, 100); 
 
 //Detect the index curled  
  if (index_data  <= bias && 
      thumb_data  > bias  && 
      middle_data > bias) {

     Serial.print ("Touch Detected");  
     digitalWrite (led, HIGH);   //LED on
     delay (1000);
     digitalWrite (led, LOW);    //LED off 
     delay (1000); 
  }

  //debugging information 
  if (debug == true) {
     Serial.print("thumb: "); 
     Serial.print(thumb_data); 
     Serial.print("index: "); 
     Serial.print(index_data); 
     Serial.print("middle: "); 
     Serial.print(middle_data); 

     delay(200); 
  }
}


//Testing 
/*
void setup() 
{
  //initialize variables. 
  //pin modes. 
  //pinMode (13, OUTPUT); 
  
  //serial baud rate 
  Serial.begin(9600); 
}

void loop() 
{
   digitalWrite(13,HIGH);   //LED on
   delay (1000); 
   digitalWrite(13, LOW);   //LED off
   delay (1000); 
   
   //loops back onto itself.  
   Serial.println("Hello World"); 
   delay(200); 
}
*/

