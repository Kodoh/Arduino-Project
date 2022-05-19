#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield(); 
#define DEBUG
#ifdef DEBUG
#define start main
#else
#define start synchro
#endif
#define emptylist 0
#define UP_ARROW 2
#define DOWN_ARROW 1
#ifdef __arm__
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
const byte numChars = 32;
char fchar1[numChars]; 
struct values {
  char ID;
  String names;
  int value;
  int minVal;
  int maxVal;
};
values channels[24];
int channelNum = 0;
boolean newData = false;
typedef enum state_e { synchro=5,basic, main,select,longText,longText2,maxvalues} phase;
const int one_second = 1000;
const int three_seconds = 3000;
int amount = EEPROM.read(0);
int location = 2;
int current = EEPROM.read(1);
int boundNum = 0;
String mode = "defualt";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(19,2);
  lcd.setCursor(0,0);
  set_colour(main,"defualt");
  byte a [ ] =  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000 };
  lcd.createChar(emptylist,a);
  byte d [ ] =  { B00100, B01110, B11111, B00100, B00100, B00100, B00000, B00000 };
  lcd.createChar(UP_ARROW, d);
  byte c[ ] = { B00000, B00000, B00100, B00100, B00100, B11111, B01110, B00100 };
  lcd.createChar(DOWN_ARROW,c);
  for (int i = 0;i<24;i++) {
    channels[i].minVal = 0;
    channels[i].maxVal = 255;
  }
  for(int i = 0; i<amount;i++) {
    //getString();
  }
}
#define new_state(s) { state = s; set_colour(s,mode); }
void loop() {
  static int last_b = 0;
  static phase state = start;
  static int pointer = 1;
  static int bpressed;
  static bool maxred = false;
  static bool mingreen = false;
  static char currentLetter;
  static int numSize;
  static int numSize2;
  static String FoundChannel;
  static int index;
  static int avg
  static int index2;
  static int getmaxCount = 0;
  static int tempPointer;
  static boolean firstTime;
  static int tempNum;
  static values temp[20];
  static int press_time;
  static String SCROLLmode;
  static String HCImode;
switch (state) {

 case longText: {
    if (SCROLLmode == "top") {
    FoundChannel = channels[pointer-1].names;
    }
    if (SCROLLmode == "bottom") {
      FoundChannel = channels[pointer].names;
    }
  for (int j = 0;j<(FoundChannel.length()-8);j+=2) {
    if (SCROLLmode == "top") {
      lcd.setCursor(index+numSize,0);
    }
    if (SCROLLmode == "bottom") {
      lcd.setCursor(index2+numSize2,1);
    }
    lcd.print(FoundChannel.substring(j));
    Serial.print(FoundChannel.substring(j));
    if (j == (FoundChannel.length()-9) && (FoundChannel.length())%2 == 1) {
      Serial.print("TRUE");
      if (SCROLLmode == "top") {
      lcd.setCursor(15,0);
      }
      if (SCROLLmode == "bottom") {
        lcd.setCursor(15,1);
      }
      lcd.write(' ');
    }
    delay(1000);
  }  
  new_state(main);
  break; 
 }
 case maxvalues: {
  //Serial.print(getmaxCount);
  if (getmaxCount % 2 == 0) {
   getmaxCount++;
  for (int x = 0; x<channelNum;x++) {
    temp[x] = channels[x];
  }
  
  tempNum = channelNum;
  for (int t = 0; t<channelNum;t++) {
    channels[t].names = "";
    channels[t].ID = 0;
    channels[t].value = 0;
    channels[t].maxVal = 0;
    channels[t].minVal = 0;
  }
  channelNum =  0;
  //Serial.print(channels[1].names);
  lcd.clear();
  lcd.setCursor(0,0);
  for(int i = 0; i<tempNum;i++) {
      if (HCImode == "max") {
        if (temp[i].value > temp[i].maxVal) {
          channels[channelNum].ID = temp[i].ID;
          channels[channelNum].value = temp[i].value;
          channels[channelNum].maxVal = temp[i].maxVal;
          channels[channelNum].minVal = temp[i].minVal;
          channels[channelNum].names = temp[i].names;
          channelNum++;
        }
      }
      if (HCImode == "min") {
        if (temp[i].value < temp[i].minVal) {
          channels[channelNum].ID = temp[i].ID;
          channels[channelNum].value = temp[i].value;
          channels[channelNum].maxVal = temp[i].maxVal;
          channels[channelNum].minVal = temp[i].minVal;
          channels[channelNum].names = temp[i].names;
          channelNum++;
      }
    }
  }
  tempPointer = pointer;
  pointer = 1;
  //Serial.print(values[0]);
  new_state(main);
  break;
  }
 else {
  getmaxCount++;
  pointer = tempPointer;
  for (int x = 0; x<tempNum;x++) {
    channels[x] = temp[x];
  }
  channelNum = tempNum;
  new_state(main);
  break;
 }
 }
 case select: {
  if (millis() - press_time >= 1000) {
  press_time = millis();
  lcd.clear();
  lcd.setBacklight(B101);
  lcd.setCursor(0,0);
  lcd.print("F132339");
  lcd.setCursor(0,1);
  lcd.print(freeMemory());
  int b = lcd.readButtons();
  int released = !b & last_b;
   last_b = b; // Save
   if (released & bpressed) {
    new_state(main);
    break;
   }
  }
  break;
 }
    case synchro: {
      char inp;
      Serial.write('Q');
      delay(one_second);
      inp = Serial.read();
      if (inp == 'X') {
        new_state(basic);
      }
      break;
    }
    case basic: {
      Serial.println("UDCHARS,FREERAM,NAMES,EEPROM,HCI"); 
      new_state(main);
      break;
    }
    case main: {
     //Serial.println(values[0]);
     String fchar;
     char* fchar2;
     recvWithEndMarker();
     if (newData == true) {
      fchar = fchar1;
      newData = false;
     }
      if (fchar[0] == 'C') {
        //storeString(fchar);
        for (int i = 0;i<24;i++) {
        if (channels[i].ID == fchar[1]) {
            currentLetter = fchar[1];
            channels[i].ID = fchar[1];
            channels[i].names = fchar.substring(2);
            firstTime = true;
            break;
          }
         if (i == 23) {
            currentLetter = fchar[1];
            channels[channelNum].ID = fchar[1];
            channels[channelNum].names = fchar.substring(2);
            firstTime = true;
            channelNum += 1; 
         }
        }
      }
    
      if (fchar[0] == 'V'){
        int val = fchar.substring(2).toInt();
        for(int j = 0; j<channelNum;j++) {
          if(fchar [1] == channels[j].ID) {
            channels[j].value = val;
          }
        }
      }
        if (fchar[0] == 'X') {
          for(int i = 0; i<channelNum;i++) {
          if (fchar[1] == channels[i].ID) {   
            channels[i].maxVal = fchar.substring(2).toInt();
            storeString(fchar);
              }
            }
          }
        if (fchar[0] == 'N') {
          for(int i = 0; i<channelNum;i++) {
          if (fchar[1] == channels[i].ID) {   
            channels[i].minVal = fchar.substring(2).toInt();
            storeString(fchar);
          }
        }
          }
      lcd.clear(); 
      for(int i = 0; i < channelNum; i++) {
        for(int j = 0; j < channelNum - 1; j++) {
          if (channels[j].ID > channels[j+1].ID){
            values temp = channels[j+1];
            channels[j+1] = channels[j];
            channels[j] = temp;
            
          }    
        }
      }
      int pos = String(channels[pointer-1].value).length();
      int val = channels[pointer-1].value;
      if  (val >= 0) {
        if (pos == 1) {
          index = 4;
        }
        if (pos == 2) {
          index = 3;
        }
        if (pos == 3) {
          index = 2;
        }
      } else {
        if (pos == 2) {
          index = 4;
        }
        if (pos == 3) {
          index = 3;
        }
        if (pos == 4) {
          index = 2;
        }
      }
      int pos2 = String(channels[pointer].value).length();
      int val2 = channels[pointer].value;
      if  (val2 >= 0) {
        if (pos2 == 1) {
          index2 = 4;
        }
        if (pos2 == 2) {
          index2 = 3;
        }
        if (pos2 == 3) {
          index2 = 2;
        }
      } else {
        if (pos2 == 2) {
          index2 = 4;
        }
        if (pos2 == 3) {
          index2 = 3;
        }
        if (pos2 == 4) {
          index2 = 2;
        }
      }
    maxred = false;
    mingreen = false;
      for(int i = 0; i<channelNum;i++) {
          if (channels[i].value > channels[i].maxVal) {
             lcd.setBacklight(B001);
             maxred = true;
          }
          if (channels[i].value < channels[i].minVal) {
            lcd.setBacklight(B010);
            mingreen = true; 
          }
        }
      if (mingreen & maxred) {
        lcd.setBacklight(B011);
      }
      if (!maxred & !mingreen) {
       lcd.setBacklight(B111); 
      }
      int firstVal = channels[pointer-1].value;
      //Serial.println(firstVal);
      if (channels[pointer-1].ID != 0) {
      lcd.setCursor(1,0);
      lcd.print(channels[pointer-1].ID);
      lcd.setCursor(index,0);
      lcd.print(firstVal);
      }
      numSize = String(firstVal).length() + 1;
      char firstchar = channels[pointer-1].ID;
      int count = channels[pointer-1].names.length();
      FoundChannel = channels[pointer-1].names;
        //delay(1000);
        int letterCounter = 16-(index+numSize+count);
        if ((letterCounter<0) && firstTime == true && currentLetter == firstchar) {
          Serial.print("true");
          SCROLLmode = "top";
          new_state(longText);
          firstTime = false;
      } else {
        lcd.setCursor(index+numSize,0);
        lcd.print(channels[pointer-1].names);
      }
      int secondVal = channels[pointer].value;
      if (channels[pointer].ID != 0) {
      lcd.setCursor(1,1);
      lcd.print(channels[pointer].ID);
      lcd.setCursor(index2,1);
      lcd.print(secondVal);
      }
      numSize2 = String(secondVal).length() + 1;
      char secondchar = channels[pointer].ID;
      int count2 = channels[pointer].names.length();
      FoundChannel = channels[pointer].names;
      //delay(1000);
      int letterCounter2 = 16-(index2+numSize2+count2);
      if ((letterCounter2<0)&& firstTime == true && currentLetter == secondchar) {
        SCROLLmode = "bottom";
        new_state(longText2);
        firstTime = false;
    } else {
      lcd.setCursor(index2+numSize2,1);
      lcd.print(channels[pointer].names);
    }
      if (pointer > 1 & channelNum > 0) {
        int b = lcd.readButtons();
        int pressed = b & ~last_b;
        last_b = b;
        lcd.setCursor(0,0);
        lcd.write(UP_ARROW);
        if (pressed & BUTTON_UP) {
          pointer -= 1;
      }
      }
      if (pointer < channelNum) {
        int b = lcd.readButtons();
        int pressed = b & ~last_b;
        last_b = b;
        lcd.setCursor(0,1);
        lcd.write(DOWN_ARROW);
        if (b & BUTTON_DOWN) { 
          pointer += 1;
        }
      }
      delay(150);
      int readbutt = lcd.readButtons();
      int pressed2 = readbutt & ~last_b;
      last_b = readbutt;
      if (pressed2 & BUTTON_SELECT) {
         bpressed = pressed2;
         press_time = millis();
         new_state(select);
         break;
      }
      if (pressed2 & (BUTTON_RIGHT)) {
        bpressed = pressed2;
        HCImode = "max";
        new_state(maxvalues);
        break;
      }
      if (pressed2 & (BUTTON_LEFT)) {
        bpressed = pressed2;
        HCImode = "min";
        new_state(maxvalues);
        break;
      }
}
}
} 
void set_colour(phase t,String mode) {
  //Serial.print(mode);
  switch (t) {
    case select:
      lcd.setBacklight(B111);
      break;
    case synchro:
      lcd.setBacklight(B101);
      break;
    case basic:
      lcd.setBacklight(B111);
      break;
    //case longText:
    case maxvalues:
    case main:
        lcd.setBacklight(B111);                       
      break; 

}
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            fchar1[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            fchar1[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}



void storeString(String str) {
    amount += 1;
    EEPROM.write(0, amount);     //holds number of items
    EEPROM.write(current,str.length());
    for (int i = 0; i < str.length(); i++) {
        EEPROM.write(i + current+1, str[i]);
    }
    current += str.length() + 1;
    EEPROM.write(1,current);
}

void getString() {
    String str = "";
    int len = EEPROM.read(location)+location;
    for (int i = location; i < len; i++) {
        str += (char)EEPROM.read(i + 1);
    }
    location += str.length() + 1;
    if (str[0] == 'C') {
     channels[channelNum].ID = str[1];
     channels[channelNum].names = str.substring(2);
     channelNum += 1;
    } else {
      for (int j = 0;j<channelNum;j++) {
        if (str[1] == channels[j].ID) {
          if (str[0] == 'X') {
            Serial.print("x");
            channels[j].maxVal = str.substring(2).toInt();
          }
          if (str[0] == 'N') {
            channels[j].minVal = str.substring(2).toInt();
          }
        }
      }
    }
}
