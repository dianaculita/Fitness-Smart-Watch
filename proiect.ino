// ---------------------------------------------------------------------------------------------------

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> 
#include <Adafruit_ST7789.h> 
#include <SPI.h>
#include <Wire.h>
#include <MechaQMC5883.h>
#include<SoftwareSerial.h>

// ---------------------------------------------------------------------------------------------------

#define DEBUG true

#if defined(ARDUINO_FEATHER_ESP32)
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         8
#endif

// ---------------------------------------------------------------------------------------------------

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
MechaQMC5883 gyro;
SoftwareSerial mySerial(3,2);

const int heartRate = A0;
int val, bpm;
int pressed = 0;

int apasat = 0;

int sec1 = 0;
int min1 = 0;
unsigned long msec = 0;
unsigned long mili = 0;
unsigned long tim = 0;
int pres = 0; // verifica buton apasat pentru cronometru
int fase = 0; // lucreaza cu 3 faze
int start = 0; // incepe cronometrul

unsigned long timp_start = 0;
unsigned long timp_end = 0;
unsigned long timp = 0;
int mil = 0;
int s = 0; 
int minut = 0;
int faza = 0;

int nrB = 3; // nr butoane
int but[3];
boolean varB[3];

int count = 0;
boolean gata = 0;

String APIKEY = "VMQBTPMTVMGQNSVB";
String HOST = "184.106.153.149"; //https://api.thingspeak.com
String PORT = "80";
String field = "field1";
String field2 = "field2";

// ---------------------------------------------------------------------------------------------------

void setup() {  
  Serial.begin(9600);

  // setup butoane
  but[0] = 7; but[1] = 6; but[2] = 5;
  varB[0] = false; varB[1] = false; varB[2] = false; 
  
  pinMode(but[0], INPUT_PULLUP); // BUTON CRONOMETRU -> pin 7
  pinMode(but[1], INPUT_PULLUP); // BUTON PULS -> pin 6
  pinMode(but[2], INPUT_PULLUP); // BUTON ACTIVITATE -> pin 5

  pinMode(heartRate, INPUT);

  // setup lcd
  Serial.print(F("Hello! ST77xx TFT Test"));
  tft.initR(INITR_BLACKTAB);
  Serial.println(F("Initialized"));

  // setup giroscop
  Wire.begin();
  gyro.init();

  // setup comunicatie wifi
   mySerial.begin(9600);
   sendMyData("AT\r\n",2000,DEBUG);
   sendMyData("AT+CWMODE=1\r\n",2000,DEBUG);
   sendMyData("AT+CWJAP=\"NmeWIFI\",\"Parola\"\r\n",3000,DEBUG); // datele pentru NumeWIFI si Parola sunt cele personale
   sendMyData("AT+CIFSR\r\n",2000,DEBUG);
   sendMyData("AT+CIPMUX=0\r\n",2000,DEBUG);

}

// ---------------------------------------------------------------------------------------------------

void loop() {
  
  //tftClear();

  // CRONOMETRU
  if (digitalRead(but[0]) == LOW) { 
    if(pres == 0){
      fase = fase + 1; // se trece la faza urmatoare
      pres = 1; // marcheaza ca apasat
      
      if(fase > 2){ // se reseteaza fazele
        fase = 0;
        pres = 0;
      }
    }
   } else{
    pres = 0;
   }
    
   if(fase == 0){
        //tft.setCursor(2, 10);
        //tftPrintText("start");
        sec1 = 0;
        min1 = 0;
        tim = 0;
        mili = 0;
        msec = 0;
        start = 0;
    }

    if(fase == 1){
       tftClear();
       tft.setCursor(2, 10);
       tftPrintText("started");
       tft.setCursor(2, 40);
       
       if(start == 0){
            start = 1; // incepe
            tim = millis();  
       }
       msec = millis() - tim; 
       min1 = msec / 60000;

       if(msec/1000 > 59){
            sec1 = (msec/1000) - (min1*60);
       }else{
            sec1 = msec / 1000;
       }

       mili = (msec%1000) / 10;
       
       if(min1 <= 9){
            tftPrintText("0"); // afisare si cifra zecilor care e 0 : 0x minute
            tftPrintVal(min1);
       }else{
          tftPrintVal(min1);
       }

       tftPrintText(":");

       if(sec1 <= 9){
            tftPrintText("0");
            tftPrintVal(sec1);
       }else {
            tftPrintVal(sec1);
        }

       tftPrintText(":");

        if(mili <= 9){
            tftPrintText("0");
            tftPrintVal(mili);
        }else {
            tftPrintVal(mili);
        }
    }

    if(fase == 2){
        tft.println();
        tftPrintText("time:");
        tft.setCursor(2, 80);
        
        if(min1 <= 9){
            tftPrintText("0");
            tftPrintVal(min1);
        }else {
            tftPrintVal(min1);
        }

        tftPrintText(":");

        if(sec1 <= 9){
            tftPrintText("0");
            tftPrintVal(sec1);
        }else {
            tftPrintVal(sec1);
        }

        tftPrintText(":");

        if(mili <= 9){
            tftPrintText("0");
            tftPrintVal(mili);
        }else {
            tftPrintVal(mili);
        }
        delay(3000);
        tftClear();
    }

// ---------------------------------------------------------------------------------------------------
  
  // PULS
  if (digitalRead(but[1]) == LOW) { 
    pressed = 1;
    count++; // apas pentru pornire -> count = 1; apas pentru oprire -> count = 2
  }
  if(pressed == 1){
    pressed = 1;
    val = analogRead(heartRate)/10;
    tftPrintPulse(val);
    tft.println();
    tftPrintText("count:");
    tftPrintVal(count);
    delay(1000);

  }

  if(count == 2){ // oprire din a se afisa pulsul
    count = 0;
    pressed = 0;
  }
  
// ---------------------------------------------------------------------------------------------------

  // HOMESCREEN
  if (digitalRead(but[2]) == LOW) { 
    homeScreen();
  }

// ---------------------------------------------------------------------------------------------------

  // TRACKER ACTIVITATE BAZATA PE PULS
  //tftClear();
  
  timp_end = 0;
  val = analogRead(heartRate)/10;

  if(val >= 90 && timp_start == 0 && faza == 0){ // se intra prima data: am puls mare si nu am inceput contorizarea
    tftPrintText("ACTIVITATE");
    timp_start = millis(); // incep contorizare timp
    Serial.println("activitate detectata");
    Serial.print("timp: ");
    Serial.print(timp_start);
    Serial.println();

    sendPuls(val);
    
    faza = 1;
    minut = 0;
    s = 0;
    mil = 0;
  }
  
  if(val >= 90 && faza == 1){ // am puls mare si deja am inceput contorizarea, raman aici pana cand se reduce pulsul
    tftPrintText("ACTIVITATE");
    faza = 1;
    sendPuls(val);
    delay(500);
  } else if(val <= 80 && faza == 1){ // eram in faza 1(activitate) si s-a micsorat pulsul -> opresc
    sendPuls(val);
    timp_end = millis() - timp_start;
    timp_start = 0;
    Serial.print("activitate oprita cu timp_end = ");
    Serial.print(timp_end);
    faza = 2; // pot sa trec la afisare si trimitere date, activitatea s-a incheiat
  }

  if(timp_end != 0 && faza == 2){
    Serial.println("calcul timp activitate...");
    minut = timp_end/60000;
    if(timp_end/1000 > 59){
      s = (timp_end/1000) - (minut*60);
    }else{
      s = timp_end / 1000;
    }

    mil = (timp_end%1000) / 10;

    Serial.print("timp final: ");
    Serial.print("minute: ");
    Serial.print(minut);
    Serial.print(" , secunde: ");
    Serial.print(s);
    
    delay(500);
    tft.println();
    tftPrintVal(minut);
    tftPrintText(":");
    tftPrintVal(s);
    delay(5000);
    tft.println();
    tftPrintText("SLEEP");
    delay(3000);
    //tftClear();
    tft.println();
    tftPrintText("sending...");
    
    // trimitere date catre ThingSpeak -> trimit timpul in secunde sau minute (trimit valoare intreaga)
    if(s != 0){
      if(minut != 0){
        sendTimp(minut);
      } else {
        sendTimp(s);
      }
    } else{
      sendTimp(mil);
    }
    
    faza = 0;
  }
  
}

// ---------------------------------------------------------------------------------------------------

// FUNCTII AUXILIARE

// ---------------------------------------------------------------------------------------------------

// TRIMITE DATE DESPRE PULS CATRE THINGSPEAK
void sendPuls(int pulss){
  Serial.print("Heartbeat: ");
  Serial.println(String(pulss));

  String dataSent = "GET /update?api_key=" + APIKEY + "&" + field + "=" + String(pulss);
  sendMyData("AT+CIPSTART=\"TCP\",\"" + HOST + "\"," + PORT + "\r\n", 1000, DEBUG);
  sendMyData("AT+CIPSEND=" + String(dataSent.length()+4)+"\r\n",1000,DEBUG);
  delay(500);
  mySerial.println(dataSent+"\r\n\r\n");
  Serial.print("Values to be sent: heartbeat = ");
  Serial.print(pulss);
}

// ---------------------------------------------------------------------------------------------------

// TRIMITE DATE DESPRE TIMPUL DE ACTIVITATE CATRE THINGSPEAK
void sendTimp(int timpp){
  Serial.print("Time: ");
  Serial.println(String(timpp));

  String dataSent = "GET /update?api_key=" + APIKEY + "&" + field + "=" + String(val) + "&" + field2 + "=" + String(timpp); // trimit si pulsul curent
  sendMyData("AT+CIPSTART=\"TCP\",\"" + HOST + "\"," + PORT + "\r\n", 1000, DEBUG);
  delay(2000);
  sendMyData("AT+CIPSEND=" + String(dataSent.length()+4)+"\r\n",1000,DEBUG);
  delay(2000);
  mySerial.println(dataSent+"\r\n\r\n");
  Serial.print("Values to be sent: time = ");
  Serial.print(timpp);
  
}

// ---------------------------------------------------------------------------------------------------

// AFISEAZA UN ECRAN PRINCIPAL
void homeScreen(){
  tftClear();
  if(digitalRead(5) == LOW)
    apasat = 1;

  if(apasat == 1){
    //apasat = 1;
    tft.setCursor(10, 20);
    tftPrintText("HOME");
    tft.println();
    tft.println("SLEEP"); // default
    tft.println("functions:");
    tft.println("1 - pulse");
    tft.println("2 - timer");
    delay(5000);
    tftClear();
  }
  apasat = 0;
}

// ---------------------------------------------------------------------------------------------------

// TRIMITE DATE
String sendMyData(String command, const int timeout, boolean debug){
  String response = "";
  mySerial.print(command);//trimite comanda
  long int time = millis();
  
  while((time+timeout) > millis()){
    while(mySerial.available()){//daca modulul are date de afisat le punem pe fereastra seriala
       char character  = mySerial.read();//citeste urmatorul caracter
       response += character; //adauga caracterul in continutul raspunsului
    }
  }
  if(debug){
    Serial.print(response);
  }
  return response;
}

// ---------------------------------------------------------------------------------------------------

// AFISEAZA TEXT
void tftPrintText(String text) {
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.print(text);
}

// ---------------------------------------------------------------------------------------------------

// AFISEAZA VALOARE INT
void tftPrintVal(int val) {
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(val);
}

// ---------------------------------------------------------------------------------------------------

// AFISEAZA PULS
void tftPrintPulse(int val) {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.println(" BPM:");
  tft.print(" ");
  tft.print(val);
}

// ---------------------------------------------------------------------------------------------------

// STERGE DATELE RAMASE PE ECRAN
void tftClear(){
  tft.setTextWrap(true);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(2, 10);
  tft.setTextColor(ST77XX_WHITE);
}

// ---------------------------------------------------------------------------------------------------
