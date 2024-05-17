#include <LiquidCrystal.h>

#include <SoftwareSerial.h>

#include <PulseSensorPlayground.h>


String agAdi="arif";
String sifre="arif2323";
const int RS=12,enable=11,d4=7,d5=6,d6=5,d7=4;
LiquidCrystal lcd(RS,enable,d4,d5,d6,d7);
float pulse=0;
float sicaklik=0;
SoftwareSerial ser(2,3);
String apiKey="QLT8Q7N64UWG222P";

//DEĞİŞKENLER
int nabizPin=A0;
int nabizvarPin=8;
int nabizyokPin=13;
int nabizyokoran=0;

volatile int KAH;
volatile int sinyal;
volatile int zamanaralik=600;
volatile boolean Pulse=false;
volatile boolean atis=false;

static boolean seriportgorsel=true;

volatile int zaman[10];
volatile unsigned long orneksayac=0;
volatile unsigned long sonatiszaman=0;
volatile int enyuksek=512;
volatile int endusuk=512;
volatile int anlik=525;
volatile int genlik=100;
volatile boolean ilkkalpatis=true;
volatile boolean ikincikalpatis=false;

void setup() {
  lcd.begin(16,2);
  pinMode(nabizvarPin,OUTPUT);
  pinMode(nabizyokPin,OUTPUT);
  Serial.begin(9600);
  kesmefonksiyonu();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Hasta Takip ");
  lcd.setCursor(0, 1);
  lcd.print("     Sistemi");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Baslatiliyor....");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Veri aliniyor....");
  ser.begin(9600);
  ser.println("AT");
  delay(1000);
  ser.println("AT+GMR");
  delay(1000);
  ser.println("AT+CWMODE=1");
  delay(1000);
  ser.println("AT+CWQAP");
  delay(1000);
  ser.println("AT+RST");
  delay(5000);
  ser.println("At+CIPMUX=1");
  delay(1000);

  //String cmd="AT+CWJAP=\""+agAdi+"\",\""+sifre+"\"";
  ser.println("AT+CWJAP=\""+agAdi+"\",\""+sifre+"\"");
  //String cmd="AT+CWJAP=\"DESKTOP-1T6KK7Q\",\"gnzl3979\"";
  //ser.println(cmd);
  delay(1000);
  ser.println("AT+CIFSR");
  delay(1000);

}

void loop() {
  seriportcikis();

  if(atis==true)
  {
    nabizyokoran=255;
    kalpatisseriportcikis();
    atis=false;
  }
  ledsoldurma();
  delay(20);
  read_sicaklik();
  esp_8266();

}

void kesmefonksiyonu(){   
  TCCR2A=0x02;
  TCCR2B=0x06;
  OCR2A=0x7C;
  TIMSK2=0x02;
  sei();
}

void seriportcikis(){  
  if(seriportgorsel==true){
    arduinoSerialMonitorVisual('-',sinyal);        

  }
  else{
    seriportaveriyolla('S',sinyal);
  }
}

void arduinoSerialMonitorVisual(char sembol,int veri){
  const  int sensorMin=0;
  const int sensorMax=1024;
  int sensorokuma=veri;
  int aralik=map(sensorokuma,sensorMin,sensorMax,0,11);
  switch(aralik){
    case 0:
      Serial.println("");
    break;
    case 1:
      Serial.println("---");
    break;
    case 2:                                                                                    
      Serial.println("------");
    break;
    case 3:
      Serial.println("---------");
    break;
    case 4:
      Serial.println("------------");
    break;
     case 5:
      Serial.println("--------------|-");
    break;
    case 6:
      Serial.println("--------------|---");
    break;
    case 7:
      Serial.println("--------------|------");
    break;
    case 8:
      Serial.println("--------------|---------");
    break;
    case 9:
      Serial.println("--------------|------------");
    break;
    case 10:
      Serial.println("--------------|---------------");
    break;
    case 11:
      Serial.println("--------------|-------------------");
    break;
  }
}

void seriportaveriyolla(char sembol,int veri){
    Serial.print(sembol);                                       
    Serial.println(veri);                                                 

}

void kalpatisseriportcikis(){
  if(seriportgorsel==true){
    Serial.print("Kalp atış hızı :");              
    Serial.println(KAH);
  }
  else{
    seriportaveriyolla('B',KAH);
    seriportaveriyolla('Q',zamanaralik);
  }
}

void ledsoldurma(){
  nabizyokoran-=15;
  nabizyokoran=constrain(nabizyokoran,0,255);         
  analogWrite(nabizyokPin,nabizyokoran);

}

void read_sicaklik(){
    int sicaklikdeger=analogRead(A1);
    float milivolt=(sicaklikdeger/1024.0)*5000;
    sicaklik=milivolt/25;
    Serial.print("Sıcaklık :");
    Serial.println(sicaklik);
    lcd.clear();
    lcd.setCursor(0,0);                                                                       
    lcd.print("Kalp atis :");
    lcd.setCursor(11,0);
    lcd.print(KAH);
    lcd.setCursor(0,1);
    lcd.print("Sicak.:");
    lcd.setCursor(8,1);
    lcd.print(sicaklik);
    lcd.setCursor(14,1);
    lcd.print("C");
    delay(2000); 
}

void esp_8266(){
    String cmd = "AT+CIPSTART=4,\"TCP\",\"";
    cmd+="184.106.153.149"; 
    cmd+="\",80";
    ser.println(cmd);
    Serial.println(cmd);
    if(ser.find("Hata")){
      Serial.println("AT+CIPSTART hata");
      return;
    }
    String getStr="GET https://api.thingspeak.com/update?api_key=7M4MHUA4M5N7UVI1&field1=0";                                                   
    getStr+=apiKey;
    getStr+="&field1=";
    getStr+=String(sicaklik);
    getStr+="&field2=";
    getStr+=String(pulse);
    
    getStr+="\r\n\r\n";
    cmd="AT+CIPSEND=4";
    cmd+=String(getStr.length());
    ser.println(cmd);
    Serial.println(cmd);
    delay(1000);
    ser.print(getStr);
    Serial.println(getStr);
    delay(3000);

}

ISR(TIMER2_COMPA_vect){
    cli();
    sinyal=analogRead(nabizPin);
    orneksayac+=2;
    int N=orneksayac-sonatiszaman;

    if(sinyal<anlik && N>(zamanaralik/5)*3){
      if(sinyal<endusuk){
        endusuk=sinyal;
      }
    }
    if(sinyal>anlik && sinyal>enyuksek){
      enyuksek=sinyal;
    }
    if(N>250){
      if((sinyal>anlik) && (Pulse==false) && (N>(zamanaralik/5)*3)){
        Pulse=true;
        digitalWrite(nabizvarPin,HIGH);
        zamanaralik=orneksayac-sonatiszaman;
        sonatiszaman=orneksayac;

        if(ikincikalpatis){
          ikincikalpatis=false;
          for(int i=0;i<=9;i++){
            zaman[i]=zamanaralik;
          }
        }
        if(ilkkalpatis){
          ilkkalpatis=false;
          ikincikalpatis=true;
          sei();
          return;
        }
        word toplanandeger=0;
        for(int i=0;i<=8;i++){
          zaman[i]=zaman[i+1];
          toplanandeger+=zaman[i];
        }
        zaman[9]=zamanaralik;
        toplanandeger+=zaman[9];
        toplanandeger/=10;
        KAH=60000/toplanandeger;
        atis=true;
        pulse=KAH;
      }
    }
    if(sinyal<anlik && Pulse==true){
      digitalWrite(nabizvarPin,LOW);
      Pulse=false;
      genlik=enyuksek-endusuk;
      anlik=genlik/2+endusuk;
      enyuksek=anlik;
      endusuk=anlik;

    }
    if(N>2500){
      anlik=512;
      enyuksek=512;
      endusuk=512;
      sonatiszaman=orneksayac;
      ilkkalpatis=true;
      ikincikalpatis=false;
    }
    sei();
  }



