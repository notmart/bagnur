/*

------------------------------
Soil Moisture Chart
 0 : in air @ 24c
 120 : skin
 0 - 250 : BAGNATO
 300 - 500 : OK
 500 - 1021 : ASCIUTTO
 
------------------------------
*/
 

int ledblu = 3;
//usa il 5 perchè analogico
int ledverde = 5;
int ledrosso = 6;
//il 2 non è analogico
int rubinetto = 2;

int numCampioniAsciutto = 0;

int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

double intensity = 1;

void setup()
{
  Serial.begin(9800);
  pinMode(ledblu,OUTPUT);
  pinMode(ledverde,OUTPUT);
  pinMode(ledrosso,OUTPUT);
  pinMode(rubinetto,OUTPUT);
}

void fade(int red, int green, int blue, double newIntensity)
{
   while (currentRed != red || currentGreen != green || currentBlue != blue ||
          fabs(intensity - newIntensity) > (double)(0.02)) {
       if (currentRed != red) {
           currentRed = currentRed > red ? --currentRed : ++currentRed;
       }
       if (currentGreen != green) {
           currentGreen = currentGreen > green ? --currentGreen : ++currentGreen;
       }
       if (currentBlue != blue) {
           currentBlue = currentBlue > blue ? --currentBlue : ++currentBlue;
       }

       if (fabs(intensity - newIntensity) > (double)(0.02)) {
           intensity += intensity > newIntensity ? -0.01 : 0.01;
       }
       analogWrite(ledrosso, (int)((double)currentRed * intensity));
       analogWrite(ledverde, (int)((double)currentGreen * intensity));
       analogWrite(ledblu, (int)((double)currentBlue * intensity));
       delay(20);
   }
}

//attorno a 250: verde
//attorno a 1000: rosso
//valori intermedi: via di mezzo
void fadeFromSensorValue(int sensorValue, double newIntensity)
{
    int normalized = ((double)(sensorValue)/(double)1023) * 254;
    Serial.print("Valore sensore normalizzato: ");
    Serial.println(normalized);
    fade(normalized, 255 - normalized, 0, newIntensity);
}

void loop()
{
   
  Serial.print("Moisture Sensor Value:");
  int sensorValue = analogRead(5);
  Serial.println(sensorValue);

  int dialValue = analogRead(4);
  Serial.print("Manual dial value: ");
  Serial.println(dialValue);

  Serial.print("Letture asciutto:");
  Serial.println(numCampioniAsciutto);

  int moistureAdjustment = ((double)dialValue/(double)1024 - (double)0.5) * 100;
  Serial.print("Correzione moisture: ");
  Serial.println(moistureAdjustment);

  if (sensorValue < 250 - moistureAdjustment) { 
      Serial.println("bagnato");
      numCampioniAsciutto = 0;
  } else if (sensorValue >= 250 - moistureAdjustment && sensorValue < 500 - moistureAdjustment) { 
      Serial.println("OK");
  } else { 
      Serial.println("asciutto");
      numCampioniAsciutto++;
  }
  
  fadeFromSensorValue(sensorValue, 1);

  if (numCampioniAsciutto >= 10) {
      digitalWrite(rubinetto, HIGH);
      //chiudi un contatto per 2 secondi poi riaprilo: usabile sia per
      //un rubinetto a relais che per un cazzillo fatto con il servo
      fade(0, 0, 255, 1);
      //da 1 a 30 secondi
      Serial.print("Apro il rubinetto per millisecondi:");
      Serial.println(1000 + (dialValue * 20));
      delay(1000 + (dialValue * 30));
      numCampioniAsciutto = 0;
      fadeFromSensorValue(analogRead(5), 1);
      digitalWrite(rubinetto, LOW);
  }

  delay(1000);
  Serial.println("Fade sensore");
  fadeFromSensorValue(sensorValue, 0.01);

  delay(10000);
}

