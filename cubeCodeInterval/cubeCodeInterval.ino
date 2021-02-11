#define BAUD_RATE 115200
#define FFTPTS 512
#define EXINFOSIZE 11
IntervalTimer intervalTimer;

struct TransmitData
{
 float idata[FFTPTS];
 float qdata[FFTPTS];
 int ultraAmp;
 int freqOffset;
 int sampleInterval;
 int extraInfo[EXINFOSIZE];
};
struct ReceiveData
{
  int freqOffset = 0;
  int sampleInterval = 4000;
};

unsigned long  nowTime;
unsigned long  lastWriteTime = 0;
unsigned long  timeCounter = 0;
const int transPin = 23;
const int pwmIPin = 4;
const int pwmQPin = 3;
const int inIPin = A0;
const int inQPin = A1;
const int ampPin = A3;

float pwmInterval = 6.25;
volatile boolean pwmIValue = false;
volatile boolean pwmQValue = false;

void pwmHandler()
{
  if (!pwmIValue && !pwmQValue)
  {
    pwmIValue = true;
    digitalWriteFast(pwmIPin, pwmIValue);
    digitalWriteFast(transPin, pwmIValue);
    return;
  }
  if (pwmIValue && !pwmQValue)
  {
    pwmQValue = true;
    digitalWriteFast(pwmQPin, pwmQValue);
    return;
  }
  if (pwmIValue && pwmQValue)
  {
    pwmIValue = false;
    digitalWriteFast(pwmIPin, pwmIValue);
    digitalWriteFast(transPin, pwmIValue);
    return;
  }
  if (!pwmIValue && pwmQValue)
  {
    pwmQValue = false;
    digitalWriteFast(pwmQPin, pwmQValue);
    return;
  }
}

void setupPins(TransmitData* tData, ReceiveData* rData)
{
  tData->freqOffset = rData->freqOffset;
  tData->sampleInterval = rData->sampleInterval;
  pinMode(transPin, OUTPUT);
  pinMode(pwmIPin, OUTPUT);
  pinMode(pwmQPin, OUTPUT);
  pinMode(inIPin, INPUT);
  pinMode(inQPin, INPUT);
  pinMode(ampPin, INPUT);
  digitalWrite(transPin, pwmIValue);
  digitalWrite(pwmIPin, pwmIValue);
  digitalWrite(pwmQPin, pwmQValue);
  analogReadResolution(12);
  intervalTimer.begin(pwmHandler, pwmInterval);  

  for (int ii = 0; ii < EXINFOSIZE; ++ii) tData->extraInfo[ii] = 0;
  
  nowTime = micros();
  lastWriteTime = nowTime;
}
void processNewSetting(TransmitData* tData, ReceiveData* rData, ReceiveData* newData)
{
    tData->sampleInterval = newData->sampleInterval;
    rData->sampleInterval = newData->sampleInterval;
    if (tData->freqOffset != newData->freqOffset)
    {
      intervalTimer.update(pwmInterval - ((float) newData->freqOffset) * 0.000078125);
    }
    tData->freqOffset = newData->freqOffset;
    rData->freqOffset = newData->freqOffset;
}
boolean processData(TransmitData* tData, ReceiveData* rData)
{
  int ifftCounter = 0;
  int ampCounter = 0.0;
  int sampleCounter = 0.0;
  
  for (int ii = 0; ii < FFTPTS; ++ii)
  {
    tData->idata[ii] = 0;
    tData->qdata[ii] = 0;
  }
  tData->ultraAmp = 0;
  
  timeCounter = (unsigned long) (rData->sampleInterval);
  lastWriteTime = micros();
  
  while(ifftCounter < FFTPTS)
  {
    tData->idata[ifftCounter] = tData->idata[ifftCounter] + (float) analogRead(inIPin);
    tData->qdata[ifftCounter] = tData->qdata[ifftCounter] + (float) analogRead(inQPin);
    sampleCounter = sampleCounter + 1.0;

    tData->ultraAmp = tData->ultraAmp + (float) analogRead(ampPin);
    ampCounter = ampCounter + 1.0;
    nowTime = micros();
  
    if ((nowTime - lastWriteTime) > timeCounter )
    {
      tData->idata[ifftCounter] = tData->idata[ifftCounter] / sampleCounter;
      tData->qdata[ifftCounter] = tData->qdata[ifftCounter] / sampleCounter;
      ++ifftCounter;
      timeCounter = timeCounter + (unsigned long) (rData->sampleInterval);
      sampleCounter = 0.0;;
    }
  }
  tData->ultraAmp = tData->ultraAmp / ampCounter;
  return true;
}

const int microLEDPin = 13;
const int commLEDPin = 2;
boolean commLED = true;

struct TXinfo
{
  int cubeInit = 1;
  int newSettingDone = 0;
};
struct RXinfo
{
  int newSetting = 0;
};

struct TX
{
  TXinfo txInfo;
  TransmitData txData;
};
struct RX
{
  RXinfo rxInfo;
  ReceiveData rxData;
};
TX tx;
RX rx;
ReceiveData settingsStorage;

int sizeOfTx = 0;
int sizeOfRx = 0;

void setup()
{
  setupPins(&(tx.txData), &settingsStorage);
  pinMode(microLEDPin, OUTPUT);    
  pinMode(commLEDPin, OUTPUT);  
  digitalWrite(commLEDPin, commLED);
  digitalWrite(microLEDPin, commLED);

  sizeOfTx = sizeof(tx);
  sizeOfRx = sizeof(rx);
  Serial1.begin(BAUD_RATE);
  delay(1000);
}
void loop()
{
  boolean goodData = false;
  goodData = processData(&(tx.txData), &settingsStorage);
  if (goodData)
  {
    tx.txInfo.newSettingDone = 0;
    if(Serial1.available() > 0)
    { 
      commLED = !commLED;
      digitalWrite(commLEDPin, commLED);
      digitalWrite(microLEDPin, commLED);
      Serial1.readBytes((uint8_t*)&rx, sizeOfRx);
      
      if (rx.rxInfo.newSetting > 0)
      {
        processNewSetting(&(tx.txData), &settingsStorage, &(rx.rxData));
        tx.txInfo.newSettingDone = 1;
        tx.txInfo.cubeInit = 0;
      }
    }
    Serial1.write((uint8_t*)&tx, sizeOfTx);
  }
  
}
