/*
 * 

  This code works with the How Do Sharks Swim workbook and lesson plan
  available from the Microsoft Education Workshop at 
  https://aka.ms/shark-lesson
  http://aka.ms/hackingSTEM   
   
  This projects uses an Arduino UNO microcontroller board. More information can
  be found by visiting the Arduino website: https://www.arduino.cc/en/main/arduinoBoardUno 
  See https://www.arduino.cc/en/Guide/HomePage for board specific details and tutorials.

  This project relies upon the construction of a simple joystick controller illuststrating pitch, roll, and yaw. 
  For each axis there are 2 switches. All switches are set HIGH and there is a common GND. When a switch is
  activated the switch goes LOW. For each pair of switches the code emits 1, 0, or -1 indicating direction. 

  Comments, contributions, suggestions, bug reports, and feature
  requests are welcome! For source code and bug reports see:
  https://github.com/microsoft/hackingstem-bbc-joystick-microbit

  David Myka, 2018 Microsoft Education Workshop
  
 *
 */

// Variables to track the 3 axis
int pitch; 
int yaw; 
int roll;

// SWITCH STATES
// x-axis
int roll_ccw; 
int roll_cw;

// y-axis
int pitch_ccw; 
int pitch_cw;

// z-axis
int yaw_ccw;
int yaw_cw; 
//-------------

// Game variables from Excel
String X;
String Y;
String Z;
String hitTime;
int reset = 0;
//------------------

// This should be 6 for a 3 axis joystick
const int numberOfInputPins = 6;

/*
 * -------------------------------------------------------------------------------------------------------
 * SERIAL DATA VARIABLES----------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------------------------------
 */

 //Incoming Serial Data Array
const byte numberOfChannelsFromExcel = 6;  // ******>>>This must be equal to or greater then the number of channels set in Data Streamer Settings worksheet
String incomingSerialData[numberOfChannelsFromExcel];

const String DELIMITER = ",";           // Cordoba expects a comma delimeted string of data
char DOUBLEQUOTES =   char(34);         // Any fields containing commas are escaped with double quotes
String inputString = "";                // String variable to hold incoming data
boolean stringComplete = false;         // Variable to indicate the string is complete (newline found)
const int serialInterval = 60;          // Interval between serial writes
unsigned long serialPreviousTime;       // Timestamp to track interval



/*
 * ARDUINO INITIALIZATION CODE-----------------------------------------------------
 */

void setup() {
  Serial.begin(9600);  

  // Set all pins up to numberOfPins HIGH starting with startPin
  int startPin = 2;
  for(int i=0; i < numberOfInputPins; i++)
  {
    pinMode((i + startPin), INPUT);
    digitalWrite((i + startPin), HIGH);
  }
}

/*
 * START OF MAIN LOOP -------------------------------------------------------------
 */ 
void loop()
{
  // Process sensors
  processSensors();
 
  // Read Excel commands from serial port
  processIncomingSerial();

  // Test for reset trigger 
  if (reset==1)
  {
    resetGame();    // if we have a trigger then reset the game variables
  }

  // Process and send data to Excel via serial port
  processOutgoingSerial();  
}

/*
 * RESET GAME CODE-----------------------------------------------------------------------------------
 */

void resetGame()
{
  // Reset state variables for shark game in Excel
  X = "";
  Y = "0";
  Z = "-10.5";
  hitTime = "0";
}

/*
 * SENSOR INOUT CODE-----------------------------------------------------------------------------------
 */
void processSensors()
{
  // Read joystick poistions from digital pins
  // x-axis
  roll_ccw   = digitalRead(2);
  roll_cw    = digitalRead(3);

  // y-axis
  pitch_ccw  = digitalRead(4);
  pitch_cw   = digitalRead(5);  

  //z-axis
  yaw_ccw    = digitalRead(6); 
  yaw_cw     = digitalRead(7);  

  //PROCESS SWITCHES
  //roll----------------------------------
  if(roll_ccw==LOW)
  {
    roll = 1;
  } 
  else if(roll_cw==LOW)
  {
    roll = -1;
  }
  else
  {
    roll = 0;
  }
  
  //pitch----------------------------------
  if(pitch_ccw==LOW)
  {
    pitch = 1;
  } 
  else if(pitch_cw==LOW)
  {
    pitch = -1;
  }
  else
  {
    pitch = 0;
  }

  //yaw----------------------------------
  if(yaw_ccw==LOW)
  {
    yaw = 1;
  } 
  else if(yaw_cw==LOW)
  {
    yaw = -1;
  }
  else
  {
    yaw = 0;
  }
}


/*
 * INCOMING SERIAL DATA PROCESSING CODE-------------------------------------------------------------------
 */

void processIncomingSerial()
{
  getSerialData();
  parseSerialData();
}

/*
 * getSerialData()
 * 
 * Gathers bits from serial port to build inputString
 */
void getSerialData()
{
  while (Serial.available()) {
    char inChar = (char)Serial.read();    // get new char
    inputString += inChar;                // add it to input string
    if (inChar == '\n') {                 // if we get a newline... 
      stringComplete = true;              // we have a complete string of data to process
    }
  }
}

/*
 * parseSerialData() 
 * 
 * Parse all program control variables and data from Excel  
 */
void parseSerialData()
{
  if (stringComplete) { // process data from inputString to set program variables. 
    
    //Build an array of values from comma delimited serial string from Data Streamer
    BuildDataArray(inputString);

    //Set values based on array index - Data Out column A5 = 0, B5 = 1, C5 = 2, etc.
    X   = incomingSerialData[0];    // Data Out column A5
    Y   = incomingSerialData[1];    // Data Out column B5
    Z   = incomingSerialData[2];    // Data Out column C5
    hitTime = incomingSerialData[3];    // Data Out column D5
    reset   = incomingSerialData[4].toInt();    // Data Out column E5

    inputString = "";                         // reset inputString
    stringComplete = false;                   // reset stringComplete flag
  }
}

/*
 * BuildDataArray()
 * 
 * Takes the comma delimited string from Data Streamer and splits the fields into an array
 * Follows CSV spec and encodes fields as follows: 
 *    Fields containing commas are wrapped in doublequotes. This allows for comma decimal separators in field values.
 *    Doublequotes in fields are escaped with a second doublequote.
 */
 
void BuildDataArray(String data)
{
  return ParseLine(data);
}

/*
 * ParseLine - parses a single string of comma delimited values terminated by a line ending character
 */
void ParseLine(String data) 
{
    int i = 0;    // tracks the character we are looking at in the data from Excel (data)
    int arrayIndex = 0;

    // For each index array that we are expecting from Excel
    while(arrayIndex < numberOfChannelsFromExcel)
    {
        String field = ParseNextField(data, i);   // parse the next field of data
        incomingSerialData[arrayIndex] = field;   // add field to incomingSerialDataArray
        arrayIndex++;   // increment index
    }
}

/*
 * ParseNextField - parses the next value field in between the comma delimiters
 */
String ParseNextField(String data, int &i)
{
    if (i >= data.length() )    // if character length is greater than data string length
    {
      return ""; //end of data
    }
    
    String field = "";
    bool hitDelimiter = false;    // flag for delimiter detection 
    while (hitDelimiter == false)   // loop through characters until we hit a delimiter
    {
        if (i >= data.length() )    // if character length is greater than data string length
        {
          break; //end of data
        }

        if (String(data[i]) == "\n")    // if character is a line break
        {
          break; // end of data
        }
        
       if(String(data[i]) == DELIMITER)   // if we hit a delimiter
        {
          hitDelimiter = true;  // flag the delimiter hit
          i++; // set iterator after delimiter so we skip the comma
          break;
        }
        else
        {        
          field += data[i];   // add character to field string
          i++;    // increment to next character in data
        }
    }
    return field;
}

/*
 * OUTGOING SERIAL DATA PROCESSING CODE-------------------------------------------------------------------
 */
void processOutgoingSerial()
{
  if((millis() - serialPreviousTime) > serialInterval)  // Enter into this only when interval has elapsed
  {
    serialPreviousTime = millis(); // Reset interval timestamp
    sendDataToSerial(); 
  }
}

void sendDataToSerial()
{
  Serial.print(roll);
 
  Serial.print(DELIMITER);
  Serial.print(pitch);
   
  Serial.print(DELIMITER);
  Serial.print(yaw);
  
  // Excel passthrough variables
  Serial.print(DELIMITER);
  Serial.print(X);

  Serial.print(DELIMITER);
  Serial.print(Y);
  
  Serial.print(DELIMITER);
  Serial.print(Z);
  
  Serial.print(DELIMITER);
  Serial.print(hitTime);
  
  Serial.println(); // Line ending character
}

