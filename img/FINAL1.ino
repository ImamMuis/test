#include <Wire.h>
#include "RTClib.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN A0
#define DHTTYPE DHT11
#define HIGH 0
#define LOW 1
#define relay 2

LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS3231 rtc;
DHT dht (A0, DHT11); 
SoftwareSerial ArduinoUno (5, 6);
                                 
float suhu,kelembabantanah, kelembabanudara;                                                           // Inisialisasi input
float m_suhu, m_kelembabantanah, m_kelembabanudara;                                                    // Inisialisasi derajat keanggotaan
float dingin, sejuk, suhu_sedang, panas, sangat_panas;                                                 // Inisialisasi membership function suhu udara
float sangat_kering, kering, lembab, basah, sangat_basah;                                              // Inisialisasi membership function kelembaban tanah
float udara_sangat_kering, udara_kering, udara_lembab, udara_basah, udara_sangat_basah;                // Inisialisasi membership function kelembaban udara
float dingin_1, sejuk_1, suhu_sedang_1, panas_1, sangat_panas_1;                                       // Inisialisasi membership yang aktif
float sangat_kering_1, kering_1, lembab_1, basah_1, sangat_basah_1;                                    // Inisialisasi membership yang aktif
float udara_sangat_kering_1, udara_kering_1, udara_lembab_1, udara_basah_1, udara_sangat_basah_1;      // Inisialisasi membership yang aktif
float SCmax=0, Cmax=0, Smax=0, Lmax=0, SLmax=0;                                                        // Inisialisasi defuzzification
float fsmax=0;                                                                                         // Inisialisasai nilai max
float momen, LA, output;                                                                               // Inisialisasai defuzzifiication
String cdata;                                                                                          

// --------------------------------- FUZZYFICATION ------------------------------//

float mf_suhu(float a, float b, float c)                     // Mencari derajat keanggotan dari membership function segitiga suhu udara
   {
     if (suhu>=a&&suhu<b) {m_suhu=(suhu-a)/(b-a); }
     if (suhu>=b&&suhu<c) {m_suhu=(c-suhu)/(c-b);} 
   }

float mf_kelembabantanah(float a1, float b1, float c1)       // Mencari derajat keanggotan dari membership function segitiga suhu udara
   {
     if (kelembabantanah>=a1&&kelembabantanah<b1) {m_kelembabantanah=(kelembabantanah-a1)/(b1-a1);}
     if (kelembabantanah>=b1&&kelembabantanah<c1) {m_kelembabantanah=(c1-kelembabantanah)/(c1-b1);}
   }

float mf_kelembabanudara(float a2, float b2, float c2)       // Mencari derajat keanggotan dari membership function segitiga suhu udara
   {
     if (kelembabanudara>=a2&&kelembabanudara<b2) {m_kelembabanudara=(kelembabanudara-a2)/(b2-a2);}
     if (kelembabanudara>=b2&&kelembabanudara<c2) {m_kelembabanudara=(c2-kelembabanudara)/(c2-b2);}
   }
                                                                                                                                              
void setup() 
{ 
  pinMode(relay, OUTPUT);                 
  digitalWrite (relay, LOW);          
  lcd.init();
  lcd.backlight();
  rtc.begin();
  dht.begin();
  Serial.begin(9600);
  Serial.begin(115200);
  ArduinoUno.begin(9600);
}

void loop() {

  int sdata1 = dht.readTemperature();
   suhu = (sdata1);
  int sdata2 = dht.readHumidity();   
   kelembabanudara = (sdata2);
  int sdata3 = (analogRead(A1)/10);  
   kelembabantanah = (sdata3);

// ------------------------------------- MEMBERSHIP FUNCTION --------------------- //

  if(suhu>=25&&suhu<=30)                                                                             //dingin
    { mf_suhu(0, 25, 30); dingin =m_suhu; suhu_sedang=0; panas=0; sangat_panas=0; }            
 if(suhu>=25&&sejuk<=35)                                                                            //  sejuk 
    { mf_suhu(25,30,35); sejuk=m_suhu; panas=0; sangat_panas=0; }                     
 if(suhu>=30&&suhu<=40)                                                                             //  suhu_sedang  
    { mf_suhu(30,35,40); dingin =0;  suhu_sedang=m_suhu;  sangat_panas=0;  }        
 if(suhu>=35&&suhu<=45)                                                                             //  panas
    { mf_suhu(35,40,45); dingin =0;  sejuk=0;  panas=m_suhu;  }                               
 if(suhu>=40&&suhu<=45)                                                                             //  sangat panas
    { mf_suhu(40,45,50); dingin =0;  sejuk=0; suhu_sedang=0;  sangat_panas=m_suhu; } 
              
   //  kelembaban tanah                                                                                                                                    
 if(kelembabantanah>=80&&kelembabantanah<=100)                                                                   // Sangat kering 
    { mf_kelembabantanah(80,100,120); sangat_kering=m_kelembabantanah; lembab=0; basah=0;  sangat_basah=0; } 
 if(kelembabantanah>=50&&kelembabantanah<=90)                                                                  // kering
    { mf_kelembabantanah(50,70,90); kering=m_kelembabantanah;  basah=0; sangat_basah=0; } 
 if(kelembabantanah>=30&&kelembabantanah<70)                                                                   // lembab
    { mf_kelembabantanah(30,50,70); sangat_kering=0; lembab=m_kelembabantanah; sangat_basah=0; } 
 if(kelembabantanah>=10&&kelembabantanah<=50)                                                                  // basah
    { mf_kelembabantanah(10,30,50); sangat_kering=0; kering=0; basah=m_kelembabantanah; } 
 if(kelembabantanah>=0&&kelembabantanah<=20)                                                                 // sangat basah
    { mf_kelembabantanah(-20,0,20); sangat_kering=0; kering=0;  lembab=0; sangat_basah = m_kelembabantanah;}

   //  kelembaban udara                                                                                                                                    
 if(kelembabanudara>=0&&kelembabanudara<=20)                                                                   // Sangat kering 
    { mf_kelembabanudara(-20,0,20); udara_sangat_kering=m_kelembabanudara; udara_lembab=0; udara_basah=0;  udara_sangat_basah=0; } 
 if(kelembabanudara>=10&&kelembabanudara<=50)                                                                  // kering
    { mf_kelembabanudara(10,30,50); udara_kering=m_kelembabanudara;  udara_basah=0; udara_sangat_basah=0; } 
 if(kelembabanudara>=30&&kelembabanudara<70)                                                                   // lembab
    { mf_kelembabanudara(30,50,70); udara_sangat_kering=0; udara_lembab=m_kelembabanudara; udara_sangat_basah=0; } 
 if(kelembabanudara>=50&&kelembabanudara<=90)                                                                  // basah
    { mf_kelembabanudara(50,70,90); udara_sangat_kering=0; udara_kering=0; udara_basah=m_kelembabanudara; } 
 if(kelembabanudara>=80&&kelembabanudara<=100)                                                                 // sangat basah
    { mf_kelembabanudara(80,100,120); udara_sangat_kering=0; udara_kering=0;  udara_lembab=0; udara_sangat_basah = m_kelembabanudara;}
    
//  ------------------ mechanism Inferens  -------------------//
 if (dingin >0){dingin_1 = 1;}    
    else {dingin_1 = 0;}
  if (sejuk >0){sejuk_1 = 1;}            
    else {sejuk_1  = 0;}
  if (suhu_sedang >0){suhu_sedang_1 = 1;}         
    else {suhu_sedang_1 = 0;}
  if (panas >0){panas_1 = 1;}           
    else {panas_1= 0;}
  if (sangat_panas >0){sangat_panas_1 = 1;}     
    else {sangat_panas_1= 0;}

  if (sangat_kering >0){sangat_kering_1 = 1;}
    else {sangat_kering_1 = 0;}
  if (kering >0){kering_1 = 1;}
    else {kering_1 = 0;}
  if (lembab >0){lembab_1 = 1;}
    else {lembab_1 = 0;}
  if (basah >0){basah_1 = 1;}
    else {basah_1 = 0;}
  if (sangat_basah>0){sangat_basah_1 = 1;}
    else {sangat_basah_1 = 0;}

  if (udara_sangat_kering >0){udara_sangat_kering_1 = 1;}
    else {udara_sangat_kering_1 = 0;}
  if (udara_kering >0){udara_kering_1 = 1;}
    else {udara_kering_1 = 0;}
  if (udara_lembab >0){udara_lembab_1 = 1;}
    else {udara_lembab_1 = 0;}
  if (udara_basah >0){udara_basah_1 = 1;}
    else {udara_basah_1 = 0;}
  if (udara_sangat_basah>0){udara_sangat_basah_1 = 1;}
    else {udara_sangat_basah_1 = 0;}
    
  float SC[3]={0,1,2};// sangat cepat
  {
  if (dingin_1==1 && basah_1==1 && udara_sangat_basah_1==1){SC[1]=min(dingin, min (basah,udara_sangat_basah));}
    else {SC[1]=0;}      
  if (dingin_1==1 && sangat_basah_1==1 && udara_sangat_basah_1==1){SC[2]=min(dingin, min (sangat_basah,udara_basah));}           
    else {SC[2]=0;}   
  }
  
  float C[6]={0,1,2,3,4,5};// cepat
  {
  if (dingin_1==1 && sangat_kering_1==1 && udara_sangat_basah_1==1){C[1]=min(dingin, min (sangat_kering,udara_sangat_basah));}  
    else {C[1]=0;}  
  if (dingin_1==1 && kering_1==1 && udara_sangat_basah_1==1){C[2]=min(dingin, min (kering,udara_sangat_basah));}               
    else {C[2]=0;}  
  if (dingin_1==1 && lembab_1==1 && udara_sangat_basah_1==1){C[3]=min(dingin, min (lembab,udara_sangat_basah));}                     
    else {C[3]=0;}  
  if (sejuk_1==1 && sangat_basah_1==1 && udara_basah_1==1){C[4]=min(sejuk, min(sangat_basah,udara_basah));}             
    else {C[4]=0;}
  if (sejuk_1==1 && basah_1==1 && udara_basah_1==1){C[5]=min(sejuk, min(basah,udara_basah));}  
    else {C[5]=0;}         
  }
 
  float S[10]={0,1,2,3,4,5,6,7,8,9};// sedang
  {
  if (sejuk_1==1 && sangat_kering_1==1 && udara_sangat_basah_1==1){S[1]=min(sejuk, min(sangat_kering,udara_sangat_basah));}                 
    else {S[1]=0;} 
  if (sejuk_1==1 && kering_1==1 && udara_basah_1==1){S[2]=min(sejuk, min (kering,udara_basah));}                 
    else {S[2]=0;} 
  if (sejuk_1==1 && lembab_1==1 && udara_basah_1==1){S[3]=min(sejuk, min(lembab,udara_basah));}                 
    else {S[3]=0;}
  if (suhu_sedang_1==1 && lembab_1==1 && udara_lembab_1==1){S[4]=min(suhu_sedang, min(lembab,udara_lembab));}                 
    else {S[4]=0;} 
  if (suhu_sedang_1==1 && sangat_basah_1==1 && udara_lembab_1==1){S[5]=min(suhu_sedang,min(sangat_basah,udara_lembab));}                 
    else {S[5]=0;} 
  if (suhu_sedang_1==1 && basah_1==1 && udara_lembab_1==1){S[6]=min(suhu_sedang, min(basah,udara_lembab));}                 
    else {S[6]=0;}
  if (panas_1==1 && sangat_basah_1==1 && udara_kering_1==1){S[7]=min(panas, min(sangat_basah,udara_kering));}                 
    else {S[7]=0;}
  if (panas_1==1 && basah_1==1 && udara_kering_1==1){S[8]=min(panas, min(basah,udara_kering));}                 
    else {S[8]=0;}
  if (sangat_panas_1==1 && sangat_basah_1==1 && udara_sangat_kering_1==1){S[9]=min(sangat_panas, min(sangat_basah,udara_sangat_kering));}                 
    else {S[9]=0;}
  }
                                                                                                                                                  
  float L[7]={0,1,2,3,4,5,6};// lama
  {
  if (suhu_sedang_1==1 && sangat_kering_1==1 && udara_lembab_1==1){L[1]=min(suhu_sedang, min(sangat_kering,udara_lembab));}        
    else {L[1]=0;} 
  if (panas_1==1 && kering_1==1 && udara_kering_1==1){L[2]=min(panas, min(kering,udara_kering));}           
    else {L[2]=0;}
  if (panas_1==1 && lembab_1==1 && udara_kering_1==1){L[3]=min(panas,min(lembab,udara_kering));}           
    else {L[3]=0;}  
  if (suhu_sedang_1==1 && kering_1==1 && udara_lembab_1==1){L[4]=min(suhu_sedang, min(kering,udara_lembab));}        
    else {L[4]=0;}
  if (sangat_panas_1==1 && sangat_basah_1==1 && udara_sangat_kering_1==1){L[5]=min(sangat_panas, min(sangat_basah,udara_sangat_kering));}        
    else {L[5]=0;} 
  if (sangat_panas_1==1 && basah_1==1 && udara_sangat_kering_1==1){L[6]=min(sangat_panas, min(basah,udara_sangat_kering));}        
    else {L[6]=0;}     
  }
 
  float SL[4]={0,1,2,3};// sangat lama
  {
  if (panas_1==1 && sangat_kering_1==1 && udara_kering_1==1){SL[1]=min(panas,min(sangat_kering,udara_kering));}                    
    else {SL[1]=0;}
  if (sangat_panas_1==1 && sangat_kering_1==1 && udara_sangat_kering_1==1){SL[2]=min(sangat_panas,min(sangat_kering,udara_sangat_kering));}                    
    else {SL[2]=0;}
  if (sangat_panas_1==1 && kering_1==1 && udara_sangat_kering_1==1){SL[4]=min(sangat_panas,min(kering,udara_sangat_kering));}                    
    else {SL[3]=0;}     
  }
                                                                                                                                             
 SCmax=SC[0];  int index1=0;  
    for (int a=1; a<=2; a++){   
      if(SCmax<=SC[a]){ SCmax=SC[a];     index1=a;} 
      }
  Cmax=C[0];    int index2=0;  
    for (int b=1; b<=5; b++)  {
      if(Cmax<=C[b]){ Cmax=C[b];       index2=b;} 
      }
  Smax=S[0];    int index3=0;  
    for (int c=1; c<=9; c++)  {   
      if(Smax<=S[c]){ Smax=S[c];       index3=c;}
      }
  Lmax=L[0];    int index4=0;  
    for (int d=1; d<=6; d++)  { 
      if(Lmax<=L[d]){ Lmax=L[d];       index4=d;} 
      }
  SLmax=SL[0];  int index5=0;  
    for (int e=1; e<=3; e++)  {  
      if(SLmax<=SL[e]){ SLmax=SL[e];     index5=e;} 
      }

// --------------------- END MECHANISM INFERENCE ------------------------- //
// ------------------------ DEFUZZYFICATION ----------------------------- //  
float momen = (SCmax*15) + (Cmax*20) + (Smax*25) + (Lmax*35) + (SLmax*40);
float LA =  SCmax+ Cmax + Smax + Lmax+ SLmax;
float sdata4 = momen/LA;
cdata = cdata + sdata1 +","+ sdata2 +","+ sdata3 +","+ sdata4;
ArduinoUno.println(cdata);
delay(2000);
cdata = "";

// RTC 
 DateTime now = rtc.now();

   Serial.print(now.year(), DEC);
   Serial.print('/');
   Serial.print(now.month(), DEC);
   Serial.print('/');
   Serial.print(now.day(), DEC);
   Serial.print(" ");
   Serial.print(now.hour(), DEC);
   Serial.print(':');
   Serial.print(now.minute(), DEC);
   Serial.print(':');
   Serial.print(now.second(), DEC);
   Serial.println();

  if (now.hour() == 8 & now.minute() == 0 & now.second() == 0)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  else if (now.hour() == 8 & now.minute() == 0 & now.second() == 1)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  if (now.hour() == 8 & now.minute() == 0 & now.second() == 2)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  else if (now.hour() == 8 & now.minute() == 0 & now.second() == 3)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  if (now.hour() == 8 & now.minute() == 0 & now.second() == 4)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  else if (now.hour() == 8 & now.minute() == 0 & now.second() == 5)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  if (now.hour() == 8 & now.minute() == 0 & now.second() == 6)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram  * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  else if (now.hour() == 8 & now.minute() == 0 & now.second() == 7)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  if (now.hour() == 8 & now.minute() == 0 & now.second() == 8)
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  else if (now.hour() == 8 & now.minute() == 0 & now.second() == 9)  
  {
     digitalWrite (relay, HIGH);
     int waktusiram = sdata4;
     waktusiram = waktusiram * 1000;
     delay(waktusiram);
     digitalWrite (relay, LOW);
  }
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SU: ");
  lcd.setCursor(3,0);
  lcd.print(int(sdata1));
  lcd.setCursor(0,1);
  lcd.print("KT: ");
  lcd.setCursor(3,1);
  lcd.print(int(sdata3));
  lcd.setCursor(6,0);
  lcd.print("KU: ");
  lcd.setCursor(10,0);
  lcd.print(int(sdata2));
  lcd.setCursor(6,1);
  lcd.print("OUT: ");
  lcd.setCursor(10,1);
  lcd.print(sdata4);
  delay(5000);
}
