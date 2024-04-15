//This is the include list that enables many of the functions used
#include <Arduino.h>
#include <list>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <esp32-hal-i2c.h>
#include <iostream>
#include <charconv>
#include <ctime>

//Initializing functions that are located below the loop
void master_C();
void newCode(int);

//This is for the <iostream> include
using namespace std;

//This sets the length of each code used
#define Password_Length 11

//Initialization of the pins for the external circuit
int oneLock = 4;
int twoLock = 5;
int threeLock = 6;
int oneServo = 15;
int twoServo = 16;
int threeServo = 17;

//Initialization of the global variables used throughout the code
char Data[Password_Length]; 
char Master[Password_Length] = "1234567890";
char cliPsswd[Password_Length];
char masterPos[2];
byte data_count = 0;
char customKey;
bool filledPos[15];
int currentPos;
bool full = false;

//Setting up the size of the keypad
const byte ROWS = 4;
const byte COLS = 4;

//Setting the pins for the keypad
byte rowPins[ROWS] = {35, 36, 37, 38};
byte colPins[COLS] = {39, 40, 41, 42};

//Setting what each button is equivalient to as a character
char hexaKeys[ROWS][COLS] = 
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Initializing the keypad and the lcd screen
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 20, 4);

//This class stores all the slot/client and each clients unique code 
class Clients {
  public:             // Access specifier
    char passwd[Password_Length];  
};
//Intialization of the slots
Clients client11, client12, client13, client14, client15, client21, client22, client23, client24, client25;
Clients client31, client32, client33, client34, client35;

//This clears what is stored as currently entered data and the data count
void clearData()
{
  while(data_count !=0)
  {
    Data[data_count--] = 0; 
  }
  return;
}

//Setting up the LCD screen and setting the pins as outputs
void setup() 
{
  lcd.init(); 
  lcd.backlight();

  pinMode(oneLock, OUTPUT);
  pinMode(twoLock, OUTPUT);
  pinMode(threeLock, OUTPUT);
  pinMode(oneServo, OUTPUT);
  pinMode(twoServo, OUTPUT);
  pinMode(threeServo, OUTPUT);                                                                                                                                                                                 
}

void loop()
{
  //Every new client will see this as the first on-screen instructions
  lcd.setCursor(0,0);
  lcd.print("Enter 10-digit Code:");
  lcd.setCursor(0,2);
  lcd.print("Press # for new slot");

  //Collects the currently pressed key on the keypad
  customKey = customKeypad.getKey();

  //The following code is executed if the customkey is detected
  if (customKey)
  {
    //Storing the pressed charcter and increasing the character count
    Data[data_count] = customKey; 
    lcd.setCursor(data_count,1); 
    lcd.print(Data[data_count]); 
    data_count++;

    //If the key pressed was the '#' a different set of code is executed
    //As long as the device is not full
    if(customKey == '#' && full != true)
    {
      //Checks to see how many open spaces are left
      int count = 15;
      for(int i = 0; i < 15; i++)
      {
        if(filledPos[i] == true){
          count--;
        }
      }
      //One there is 9 or less open slot the slots left will display on the screen
      if(count < 10 && count!=0)
      {
        lcd.setCursor(0,3);
        lcd.print("Spots left:");
        lcd.print(count);
        delay(1500);
      }//Once the open slots reaches 0 than the device displays as full
      //It will not generate a new slot and will go back to waiting for a code to be entered
      else if(count == 0)
      {
        lcd.setCursor(0,3);
        lcd.print("Device is full");
        clearData;
        full = true;
        return;
      }
      //Generates a random slot number
      srand((unsigned int)time(NULL));
      int spot = rand() % 15;
      //Checks to see if that slot is in use
      while(filledPos[spot] == true)
      {
        //If in use generates new ones until it finds an empty one
        srand((unsigned int)time(NULL));
        spot = 1 + rand() % 15;
      }

      //Slot number is picks and carried to a new function for a code
      newCode(spot);
      clearData();

      //After code is generated this loop waits for the user to press *
      while(customKeypad.getKey() != '*')
      {
        lcd.setCursor(0,3);
        lcd.print("Select * to exit");
      }
      //This sends the new slot to be opened for the client
      master_C();
      lcd.clear();
    }//If the device is full this is displayed
    else
    {
      lcd.setCursor(0,3);
      lcd.print("Device is full");
    }
  }

  //Once the input count reachs 10 this is executed
  if(data_count == Password_Length - 1)
  {
    lcd.clear();
    //Checks to see if the owner entered the master password
    if(!strcmp(Data, Master))
    {
      //Asks the owner to select a slot to open without a code
      clearData();
      lcd.print("Hello Master");
      lcd.setCursor(0,1);
      lcd.print("Please select a slot");
      
      //Waits for the user to enter 2 characters
      while(data_count < 2)
      {
        customKey = customKeypad.getKey();
        if (customKey)
        {
          Data[data_count] = customKey; 
          lcd.setCursor(data_count,2); 
          lcd.print(Data[data_count]); 
          masterPos[data_count] = customKey; 
          data_count++;
        }
      }
      //Send to a function to turn on the correct external circuitry
      master_C();
    }

    //Checks all the client passwords against what was entered
    if (!strcmp(Data, client11.passwd)){
      lcd.print("Correct");
      masterPos[0] = '1';masterPos[1] = '1';
      filledPos[0] = false;
      master_C();
    }else if (!strcmp(Data, client12.passwd)){
      lcd.print("Correct");
      masterPos[0] = '1';masterPos[1] = '2';
      filledPos[1] = false;
      master_C();
    }else if (!strcmp(Data, client13.passwd)){
      lcd.print("Correct");
      masterPos[0] = '1';masterPos[1] = '3';
      filledPos[2] = false;
      master_C();
    }else if (!strcmp(Data, client14.passwd)){
      lcd.print("Correct");
      masterPos[0] = '1';masterPos[1] = '4';
      filledPos[3] = false;
      master_C();
    }else if (!strcmp(Data, client15.passwd)){
      lcd.print("Correct");
      masterPos[0] = '1';masterPos[1] = '5';
      filledPos[4] = false;
      master_C();
    }else if (!strcmp(Data, client21.passwd)){
      lcd.print("Correct");
      masterPos[0] = '2';masterPos[1] = '1';
      filledPos[5] = false;
      master_C();
    }else if (!strcmp(Data, client22.passwd)){
      lcd.print("Correct");
      masterPos[0] = '2';masterPos[1] = '2';
      filledPos[6] = false;
      master_C();
    }else if (!strcmp(Data, client23.passwd)){
      lcd.print("Correct");
      masterPos[0] = '2';masterPos[1] = '3';
      filledPos[7] = false;
      master_C();
    }else if (!strcmp(Data, client24.passwd)){
      lcd.print("Correct");
      masterPos[0] = '2';masterPos[1] = '4';
      filledPos[8] = false;
      master_C();
    }else if (!strcmp(Data, client25.passwd)){
      lcd.print("Correct");
      masterPos[0] = '2';masterPos[1] = '5';
      filledPos[9] = false;
      master_C();
    }else if (!strcmp(Data, client31.passwd)){
      lcd.print("Correct");
      masterPos[0] = '3';masterPos[1] = '1';
      filledPos[10] = false;
      master_C();
    }else if (!strcmp(Data, client32.passwd)){
      lcd.print("Correct");
      masterPos[0] = '3';masterPos[1] = '2';
      filledPos[11] = false;
      master_C();
    }else if (!strcmp(Data, client33.passwd)){
      lcd.print("Correct");
      masterPos[0] = '3';masterPos[1] = '3';
      filledPos[12] = false;
      master_C();
    }else if (!strcmp(Data, client34.passwd)){
      lcd.print("Correct");
      masterPos[0] = '3';masterPos[1] = '4';
      filledPos[13] = false;
      master_C();
    }else if (!strcmp(Data, client35.passwd)){
      lcd.print("Correct");
      masterPos[0] = '3';masterPos[1] = '5';
      filledPos[14] = false;
      master_C();
    }else{
      lcd.print("Incorrect");
      delay(1000);
      return;}

    //If a key was retrieve by a code the device is then set to not full
    full = false;
    lcd.clear();
    clearData(); 
  }
};

//Used to set and turn on the proper pins for the servos and locks
void master_C()
{
  delay(1000);
  //Checks the slot row
  char checkRow = masterPos[0];
  int selecServo, selecLock;
  //Row one
  if(checkRow == '1')
  {
    selecServo = oneServo;
    selecLock = oneLock;
  } 
  //Row two
  else if(checkRow == '2')
  {
    selecServo = twoServo;
    selecLock = twoLock;
  }
  //Row three
  else if(checkRow == '3')
  {
    selecServo = threeServo;
    selecLock = threeLock;
  }
  //If a wrong slot was selected incorrects slot displayed and returned to main screen
  else 
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Incorrect Slot #");
    delay(2000);
    return;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Opening door...");
  digitalWrite(selecServo, HIGH);
  delay(1000*masterPos[1]);
  digitalWrite(selecServo, LOW);
  delay(500);
  digitalWrite(selecLock, HIGH);
  delay(5000);
  digitalWrite(selecLock, LOW);
  //Need to time how long it takes to make one rotation and subtract how long it was spun 
  //by how long for a full rotation to get it back into its original spot 
  digitalWrite(selecServo, HIGH);
  delay(10000-1000*masterPos[1]);
  lcd.clear();
}

//This function randomly generates codes for each slot
void newCode(int client)
{ 
  lcd.clear();
  //Marks the selected slot as filled
  filledPos[client] = true;
  client++;
  lcd.setCursor(0,0);

  //Checks which slot was selected 
  //Sets its random values
  //And prints the generated code for the user
  switch(client)
  {
    case 1:
      lcd.print("Slot number: 11");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client11.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client11.passwd[i]);
      }
      masterPos[0] = '1';masterPos[1] = '1';
      break;
    case 2:
      lcd.print("Slot number: 12");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client12.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client12.passwd[i]);
      }
      masterPos[0] = '1';masterPos[1] = '2';
      break;
    case 3:
      lcd.print("Slot number: 13");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client13.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client13.passwd[i]);
      }
      masterPos[0] = '1';masterPos[1] = '3';
      break;
    case 4:
      lcd.print("Slot number: 14");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client14.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client14.passwd[i]);
      }
      masterPos[0] = '1';masterPos[1] = '4';
      break;
    case 5:
      lcd.print("Slot number: 15");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client15.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client15.passwd[i]);
      }
      masterPos[0] = '1';masterPos[1] = '5';
      break;
    case 6:
      lcd.print("Slot number: 21");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client21.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client21.passwd[i]);
      }
      masterPos[0] = '2';masterPos[1] = '1';
      break;
    case 7:
      lcd.print("Slot number: 22");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client22.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client22.passwd[i]);
      }
      masterPos[0] = '2';masterPos[1] = '2';
      break;
    case 8:
      lcd.print("Slot number: 23");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client23.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client23.passwd[i]);
      }
      masterPos[0] = '2';masterPos[1] = '3';
      break;
    case 9:
      lcd.print("Slot number: 24");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client24.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client24.passwd[i]);
      }
      masterPos[0] = '2';masterPos[1] = '4';
      break;
    case 10:
      lcd.print("Slot number: 25");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client25.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client25.passwd[i]);
      }
      masterPos[0] = '2';masterPos[1] = '5';
      break;
    case 11:
      lcd.print("Slot number: 31");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client31.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client31.passwd[i]);
      }
      masterPos[0] = '3';masterPos[1] = '1';
      break;
    case 12:
      lcd.print("Slot number: 32");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client32.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client32.passwd[i]);
      }
      masterPos[0] = '3';masterPos[1] = '2';
      break;
    case 13:
      lcd.print("Slot number: 33");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client33.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client33.passwd[i]);
      }
      masterPos[0] = '3';masterPos[1] = '3';
      break;
    case 14:
      lcd.print("Slot number: 34");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client34.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client34.passwd[i]);
      }
      masterPos[0] = '3';masterPos[1] = '4';
      break;
    case 15:
      lcd.print("Slot number: 35");
      for(int i = 0; i < 10; i++)
      {
        srand((unsigned int)time(NULL));
        int generatNum = 48 + rand() % 10;
        client35.passwd[i] = char(generatNum);
        lcd.setCursor(i,2);
        lcd.print(client35.passwd[i]);
      }
      masterPos[0] = '3';masterPos[1] = '5';
      break;
    default:
      lcd.setCursor(0,1);
      lcd.print("Error");
      delay(2000);
  }
  lcd.setCursor(0,1);
  lcd.print("Please remember code");
}