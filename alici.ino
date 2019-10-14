#include <SoftwareSerial.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//pinler
//#define pin_sayac_clock 8
//#define pin_sayac_reset 9

SoftwareSerial xbee(5, 4);


int snk_led = 0; //- senkron:true
int blink_led = 0; //- senkrons:true


unsigned int gelen_veri;
unsigned int eski_veri;
int gercek_Deger=0;

void setup() {
  Serial.begin(9600);
  xbee.begin(9600);
  //sayac_goster(0);
  delay(1000);
  //senkron ve reset
  pinMode(2, INPUT); //senkron
  pinMode(3, INPUT); //reset
  pinMode(6, OUTPUT); //senkron ledi
  pinMode(7, OUTPUT); //blink ledi
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  EIMSK |= 0x03;//START butonu için kesme aktif, STOP butonu START gerçekleştiğinde aktif edilecek
  EICRA |= 0x0F;//INT0 ve INT1 rising(yükselen kenar) tetikleme moduna ayarlandı


  //-
  //timer blink için
  TCNT1 = 61600; // 1 saniye 15625 için >> 65535-15625, 61600 ~2,5 saniye
  TCCR1A = 0x00;//Normal mode
  TCCR1B &= (0 << CS11); // 1024 prescler
  TCCR1B |= (1 << CS10); // 1024 prescler
  TCCR1B |= (1 << CS12); // 1024 prescler
  TIMSK1 |= (1 << TOIE1); // Timer1 taşma kesmesi aktif

  //timer blink için
  TCNT0 = 0; // 1 saniye 15625 için >> 65535-15625, 61600 ~2,5 saniye
  TCCR2A = 0x00;//Normal mode
  TCCR2B &= (0 << CS21); // 1024 prescler
  //TCCR2B |= (1 << CS10); // 1024 prescler
  TCCR2B |= (1 << CS12); // 1024 prescler
  //TIMSK2 |= (1 << TOIE1); // Timer1 taşma kesmesi aktif

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  /*digitalWrite(pin_sayac_reset, LOW);
  delay(500);
  digitalWrite(pin_sayac_reset, HIGH);
  delay(500);
  digitalWrite(pin_sayac_reset, LOW);*/

  

  //-

  sei();//Kesme aktifliği(Global kesme bayrağı),
}

void loop() {

  if (xbee.available()) {
    Serial.println("loop");
    gelen_veri = xbee.read();
    gercek_Deger=gelen_veri*6;
    if (gelen_veri != eski_veri)
    {
      Serial.println("");
      Serial.println("");
      Serial.print(" if gelen veri : ");
      Serial.println(gelen_veri);
      Serial.print(" Gelen gerçek değer : ");
      Serial.println(gercek_Deger);
      Serial.print("if eski  : :");
      Serial.println(eski_veri);
      //-
      //timer 0 eklenecek
      if ((gelen_veri != 0) && (eski_veri<200))
      {            
        Serial.print("yeni veri");
        blink_led = 0;
        digitalWrite(7, HIGH); //- blink led hıgh
        //- led blink timer start
        TCNT2 = 0;// 65535-7800(1024 prescaler)=~500 ms
        TIFR2 |= (1 << TOV1) ;//timer1 taşma bayragı sıfırlanır.
        TIMSK2 |= (1 << TOIE1) ;// Timer1 taşma kesmesi aktif
      }
      
      eski_veri = gelen_veri;
      //-
      veri_Al();

    }
  }
}

void veri_Al() {

  while (1) {
    Serial.println("");
    Serial.print("gelen veri while : ");
    Serial.println(gelen_veri);
    Serial.print("eski veri while : :");
    Serial.println(eski_veri);
    unsigned int ters = 255 - gelen_veri;
    unsigned long ilk = millis();
    xbee.write(ters);
    Serial.print("ters  while : :");
    Serial.println(ters);
    if ((millis() - ilk) < 100) {
      Serial.println ("Millis 100 ms");
      if (xbee.available()) {
        digitalWrite(6, HIGH); //-
        gelen_veri = xbee.read();
        Serial.print("2. onay :");
        Serial.println(gelen_veri);
        Serial.print("2. eski : :");
        Serial.println(eski_veri);
        if (gelen_veri == eski_veri) {
          Serial.println("veri transferei tamam");
          digitalWrite(6, HIGH);
          
          //-
          snk_led = 0;
          TCNT1 = 0;// 65535-7800(1024 prescaler)=~500 ms
          //-
          
          //sayac_goster(eski_veri);
          break;
        } 
        
        else {
          digitalWrite(6, LOW); //-
          if ((gelen_veri != 0) && (eski_veri<100))
          {            
            Serial.print("yeni veri");
            //blink_led=0;
            digitalWrite(7, HIGH); //- blink led hıgh
            //- led blink timer start
            TCNT2 = 0;// 65535-7800(1024 prescaler)=~500 ms
            TIFR2 |= (1 << TOV2) ;//timer1 taşma bayragı sıfırlanır.
            TIMSK2 |= (1 << TOIE2) ;// Timer1 taşma kesmesi aktif
          }

          Serial.println("veri transferei yeni veri");
          eski_veri = gelen_veri;
          veri_Al();
          break;
        }

      }
    } else {
      digitalWrite(6, LOW); //-
      Serial.println("2. onay gelmedi!!!");
      veri_Al();
      break;
    }
  }
}

unsigned int giden_istek = 202;
ISR(INT0_vect)//senkron
{
  Serial.println("int0");
  digitalWrite(6, LOW);
  giden_istek = 202;
  xbee.write(giden_istek);
  eski_veri = 204;
}

ISR(INT1_vect)//reset
{
  Serial.println("int1");
  digitalWrite(6, LOW);
  giden_istek = 201;
  xbee.write(giden_istek);
  eski_veri = 203;
}

/*void sayac_goster(int sayac) {

  digitalWrite(pin_sayac_reset, LOW);
  digitalWrite(pin_sayac_reset, HIGH);
  digitalWrite(pin_sayac_reset, LOW);


  for (int i = 0; i < sayac; i++) {
    digitalWrite(pin_sayac_clock, LOW);
    digitalWrite(pin_sayac_clock, HIGH);
    digitalWrite(pin_sayac_clock, LOW);
  }
}*/

ISR (TIMER1_OVF_vect)    // Timer1 ISR //-
{
  snk_led++;
  if(snk_led>1)
  {      
    giden_istek = 202;
    xbee.write(giden_istek);
    eski_veri = 204;
    
    //TIMSK1 &= (0 << TOIE1) ;   // Timer1 taşma kesmesi devredışı (TOIE1)
    TIFR1 |= (1 << TOV1) ;//timer1 taşma bayragı sıfırlanır.
    snk_led=0;  
    digitalWrite(6, LOW); 
  }
  Serial.print("oto senkronizasyon");
  Serial.println(snk_led);

  //SÜREKLİ SENKRON
}

ISR (TIMER2_OVF_vect)    // Timer1 ISR //-
{
  blink_led++;
  if(blink_led>382)//196 = 100ms
  {
    TIMSK2 &= (0 << TOIE2) ;
      digitalWrite(7, LOW);
      TIMSK2 &= (0 << TOIE2) ;
      blink_led=0;
  }
}
