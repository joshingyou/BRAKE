//Author: Ji Hye Lee 

//Define the analog input pins for sensors 
const int flex_thumb = 12; 
const int flex_index = 13; 
const int flex_middle = 14; 

//Needs more testing to find accurate numbers. 
const int index_straight = 525; 
const int index_curled = 730; 

//For testing 
const boolean debug = true; 

//Variables for holding sensor data 
int thumb_data;   //flex data for the thumb
int index_data;   //flex data for the index 
int middle_data;  //flex data for the middle finger. 

void setup() 
{
  //set Pin Directions
  pinMode (flex_thumb, INPUT); 
  pinMode (flex_index, INPUT);
  pinMode (flex_middle, INPUT); 
  
  //setup serial connection to computer 
  Serial.begin (9600); 
}

void loop()
{
  //read 5V Sensor Values
  analogReference(DEFAULT); 
  thumb_data = analogRead(flex_thumb); 
  index_data = analogRead(flex_index); 
  middle_data = analogRead(flex_middle);  

  //Map values 
  int INDEX = map (index_data, index_straight, index_curled, 0, 255); 

  //debugging information 
  if (debug == true) {
     Serial.print("   "); 
     Serial.print(INDEX); 
     delay(200); 
  }
}



