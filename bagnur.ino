/*

------------------------------
Soil Moisture Chart
 0 : in air @ 24c
 120 : skin
 0 - 250 : WET
 300 - 500 : OK
 500 - 1021 : DRY
 
------------------------------
*/
 

int blueLed = 3;
//use ping 5 because analog
int greenLed = 5;
int redLed = 6;
//2 not analog
int tapPin = 2;

int numCampioniAsciutto = 0;

int currentRed = 0;
int currentGreen = 0;
int currentBlue = 0;

double intensity = 1;

void setup()
{
  Serial.begin(9800);
  pinMode(blueLed,OUTPUT);
  pinMode(greenLed,OUTPUT);
  pinMode(redLed,OUTPUT);
  pinMode(tapPin,OUTPUT);
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
       analogWrite(redLed, (int)((double)currentRed * intensity));
       analogWrite(greenLed, (int)((double)currentGreen * intensity));
       analogWrite(blueLed, (int)((double)currentBlue * intensity));
       delay(20);
   }
}

//around a 250: green
//around a 1000: red
//valori intermedi: via di mezzo
void fadeFromSensorValue(int sensorValue, double newIntensity)
{
    int normalized = ((double)(sensorValue)/(double)1023) * 254;
    //Serial.print("Sensor value normalized 0-255: ");
    //Serial.println(normalized);
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

  Serial.print("Number of dry samplings:");
  Serial.println(numCampioniAsciutto);

  //slightly unbalanced towards dry (0.3 instead 0.5) since sensor seems to be a tad too sensible
  int moistureAdjustment = ((double)dialValue/(double)1024 - (double)0.3) * 200;
  Serial.print("Moisture correction: ");
  Serial.println(moistureAdjustment);

  int adjustedMoisture = max(0, min(1024, sensorValue + moistureAdjustment));

  Serial.print(adjustedMoisture);
 
  if (adjustedMoisture < 150 ) { 
      Serial.println(": wet");
      numCampioniAsciutto = 0;
  } else if (adjustedMoisture >= 150 && adjustedMoisture < 400) { 
      Serial.println(": OK");
  } else { 
      Serial.println(": dry");
      numCampioniAsciutto++;
  }
  
  fadeFromSensorValue(adjustedMoisture, 1);

  if (numCampioniAsciutto >= 10) {
      //Open the solenoid-controlled watering tap
      fade(0, 0, 255, 1);
      digitalWrite(tapPin, HIGH);
      //da 1 a 30 secondi
      Serial.print("Water tap open for milliseconds:");
      Serial.println(1000 + (dialValue * 20));
      delay(1000 + (dialValue * 30));
      digitalWrite(tapPin, LOW);
      numCampioniAsciutto = 0;
      sensorValue = analogRead(5);
      adjustedMoisture = adjustedMoisture = max(0, min(1024, sensorValue + moistureAdjustment));
      fadeFromSensorValue(adjustedMoisture, 1);
  }

  delay(1000);
  Serial.println("Led Fade");
  fadeFromSensorValue(adjustedMoisture, 0.01);
  Serial.println("=========== Wait for next sampling ==============");
 
  delay(10000);
}

