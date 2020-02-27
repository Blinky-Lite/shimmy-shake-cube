#define FFTPTS 512
#define FFTPTS2 1024
#define WRITEBUFFSIZ 5012
#define BAUDRATE 115200

const int pwmPin = 6;
const int pwmRisePin = 5;
const int pwmFallPin = 7;
const int transPin = 23;
const int pwmIPin = 4;
const int pwmQPin = 3;
const int ledPin = 13;
const int blueLedPin = 2;
const int inIPin = A0;
const int inQPin = A1;
const int ampPin = A3;

int pwmValue = 128;
int pwmFrequency = 80000;
int pwmResolution = 8;
boolean ledPinValue = false;
volatile boolean pwmIValue = false;
volatile boolean pwmQValue = false;
float inIValue;
float inQValue;

int ifftCounterI = 0;
int ifftCounterQ = FFTPTS;
boolean readyToReceiveFlag = true;
unsigned long  nowTime;
unsigned long  lastWriteTime = 0;
unsigned long timeCounter = 0;
float sampleCounter = 0.0;
float ampCounter = 0.0;

struct Readings
{
 float number[FFTPTS2];
 int ultraAmp;
 int freqOffset;
 int sampleInterval;
 int check;
};
Readings readings;

struct Settings
{
  int freqOffset;
  int sampleInterval;
  int check;
};
Settings settings;

void setup()
{
  readyToReceiveFlag = false;
  readings.freqOffset = 0;
  readings.sampleInterval = 2000;
  settings.check = -1;
  readings.check = WRITEBUFFSIZ;

  
  analogWriteResolution(pwmResolution);
  analogWriteFrequency(pwmPin, pwmFrequency + readings.freqOffset);
  pinMode(ledPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(transPin, OUTPUT);
  pinMode(pwmIPin, OUTPUT);
  pinMode(transPin, OUTPUT);
  pinMode(pwmQPin, OUTPUT);
  pinMode(pwmRisePin, INPUT);
  pinMode(pwmFallPin, INPUT);
  pinMode(inIPin, INPUT);
  pinMode(inQPin, INPUT);
  pinMode(ampPin, INPUT);
  digitalWrite(ledPin, ledPinValue);
  digitalWrite(transPin, pwmIValue);
  digitalWrite(pwmIPin, pwmIValue);
  digitalWrite(pwmQPin, pwmQValue);
  digitalWrite(ledPin, ledPinValue);
  digitalWrite(blueLedPin, ledPinValue);
  attachInterrupt(pwmRisePin, pwmRisePinHandler, RISING);
  attachInterrupt(pwmFallPin, pwmFallPinHandler, FALLING);
  analogWrite(pwmPin, pwmValue);

  analogReadResolution(12);

  ifftCounterI = 0;
  ifftCounterQ = FFTPTS;
  readings.ultraAmp = 0;
  ampCounter = 0.0;
 
  for (int ii = 0; ii < FFTPTS2; ++ii)
  {
    readings.number[ii] = 0.0;
  }

  Serial1.begin(BAUDRATE);

}

void loop()
{
  if (readyToReceiveFlag)
  {
    readings.number[ifftCounterI] = readings.number[ifftCounterI] + (float) analogRead(inIPin);
    readings.number[ifftCounterQ] = readings.number[ifftCounterQ] + (float) analogRead(inQPin);
    sampleCounter = sampleCounter + 1.0;

    readings.ultraAmp = readings.ultraAmp + (float) analogRead(ampPin);
    ampCounter = ampCounter + 1.0;


    nowTime = micros();
  
    if ((nowTime - lastWriteTime) > timeCounter )
    {
      readings.number[ifftCounterI] = readings.number[ifftCounterI] / sampleCounter;
      readings.number[ifftCounterQ] = readings.number[ifftCounterQ] / sampleCounter;
      ++ifftCounterI;
      ++ifftCounterQ;
      timeCounter = timeCounter + (unsigned long) (readings.sampleInterval);
      sampleCounter = 0.0;;

    }
    if (ifftCounterI == FFTPTS)
    {

      readings.ultraAmp = readings.ultraAmp / ampCounter;

      Serial1.write((uint8_t*)&readings, WRITEBUFFSIZ);
 
      readyToReceiveFlag = false;

      for (int ii = 0; ii < FFTPTS; ++ii)
      {
        readings.number[ii] = 0;
        readings.number[ii + FFTPTS] = 0;
      }
      

      ifftCounterI = 0;
      ifftCounterQ = FFTPTS;
      readings.ultraAmp = 0;
      ampCounter = 0.0;
    }
  }
  if (!readyToReceiveFlag)
  {
    if (Serial1.available() > 0)
    {
      Serial1.readBytes((uint8_t*) &settings, 12);
      if (settings.check == 12)
      {
        readings.sampleInterval = settings.sampleInterval;
        if (readings.freqOffset != settings.freqOffset)
        {
          analogWriteFrequency(pwmPin, pwmFrequency + settings.freqOffset);
        }
        readings.freqOffset = settings.freqOffset;
        lastWriteTime = micros();
        timeCounter = (unsigned long) (readings.sampleInterval);
        sampleCounter = 0.0;
        ledPinValue = !ledPinValue;
        digitalWrite(ledPin, ledPinValue);
        digitalWrite(blueLedPin, ledPinValue);
        settings.check = -1;
      }
    }
    readyToReceiveFlag = true;
  } 
}
void pwmRisePinHandler()
{
  pwmIValue = !pwmIValue;
  digitalWrite(transPin, pwmIValue);
  digitalWrite(pwmIPin, pwmIValue);
}
void pwmFallPinHandler()
{
  pwmQValue = !pwmQValue;
  digitalWrite(pwmQPin, pwmQValue);
}
