#include <Arduino.h>

// pins
int progClockPin = 3;
int stepsPin3 = 4;
int stepsPin6 = 7;
int modePin = 5;
int progPin = 6;
int outPin = 9;
int rootPin = A0;

// clock related
float BPM = 0.0;
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;
unsigned long ArpPulse = 0;
unsigned long ProgPulse = 0;
volatile bool clock = false;

// functionnality controls
int steps, progressionSelected;
bool mode;

// définition des intervalles et du mode en binaire
unsigned long demiTon1 = ((unsigned long)833 * (unsigned long)1024) / 5 / 4;
int demiTon = demiTon1 / 10000;
unsigned long minor1 = ((unsigned long)7479 * (unsigned long)1024) / 5 / 4;
int minor = minor1 / 10000;

// harmony and arpeggios
int countAccord, countNote, rootNote;
int progression[6][4][6] =
    {
        {{0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}}, //6545
        {{0, 4, 7, 12, 16, 19}, {0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}}, //2615
        {{0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}, {0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}}, //1465
        {{0, 4, 7, 12, 16, 19}, {0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}}, //1654
        {{0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}, {0, 4, 7, 12, 16, 19}}, //6415
        {{0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}, {0, 3, 7, 12, 15, 19}, {0, 4, 7, 12, 16, 19}}  //2465
};

void clockGo()
{
  clock = digitalRead(progClockPin) == HIGH;
}

void setup()
{
  pinMode(progClockPin, INPUT);
  pinMode(stepsPin3, INPUT);
  pinMode(stepsPin6, INPUT);
  pinMode(modePin, INPUT);
  pinMode(progPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(progClockPin), clockGo, RISING);
  pinMode(outPin, OUTPUT);
  pinMode(rootPin, INPUT);
  Serial.begin(9600);
}

void loop()
{
  rootNote = map(analogRead(rootPin), 0, 1023, 0, 255);
  progressionSelected = map(analogRead(progPin), 0, 1023, 0, 5);
  mode = analogRead(modePin);

  if (stepsPin3 != LOW && stepsPin6 != LOW) // check le combre de steps sélectionés
  {
    if (stepsPin3 == HIGH)
    {
      steps = 3;
    }
    else
    {
      steps = 6;
    }
  }
  else
  {
    steps = 4;
  }

  if (clock) // définition du delay entre les notes
  {

    if (IgnoreReading == false)
    {
      if (FirstPulseDetected == false)
      {
        FirstPulseTime = millis();
        FirstPulseDetected = true;
        Serial.println("first");
      }
      else
      {
        SecondPulseTime = millis();
        PulseInterval = SecondPulseTime - FirstPulseTime;
        FirstPulseTime = SecondPulseTime;
        IgnoreReading = true;
        Serial.println("second");
      }
    }
  }

  BPM = (1.0 / PulseInterval) * 60.0 * 1000 * 2; //juste pour contrôle comparatif visuel avec ma clock externe
  ArpPulse = PulseInterval / steps;
  ProgPulse = PulseInterval * 2;

  // Serial.print("PulseInterval ");
  // Serial.println(PulseInterval);

  while (clock)
  {

    if (countAccord == 4)
    {
      countAccord = 0;
    }

    if (countNote == steps)
    {
      countNote = 0;
      countAccord++;
    }

    // Serial.print("ARPEGE ");
    // Serial.println(countAccord);
    // Serial.print("note ");
    // Serial.println(countNote);
    // Serial.println("mapping demiton");
    // Serial.println(demiTon);
    // Serial.println("");
    // Serial.println("");

    int toGoOut = rootNote + (progression[progressionSelected][countAccord][countNote] * demiTon);
    // analogWrite(outPin, toGoOut);

    if (mode == false)
    {
      analogWrite(outPin, toGoOut);
    }
    else
    {
      analogWrite(outPin, (toGoOut + minor)); // transposition mineur
    }
    countNote++;
    delay(ArpPulse);
    // delay(1000);
  }
}
