// Create an IntervalTimer object 
IntervalTimer intervalTimer;

const int transPin = 23;
const int pwmIPin = 4;
const int pwmQPin = 3;
const int inIPin = A0;
const int inQPin = A1;
const int ampPin = A3;
volatile boolean pwmIValue = false;
volatile boolean pwmQValue = false;
float period = 6.25;
float freqSign = 1.0;

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

void setup() 
{
  pinMode(transPin, OUTPUT);
  pinMode(pwmIPin, OUTPUT);
  pinMode(transPin, OUTPUT);
  pinMode(pwmQPin, OUTPUT);
  pinMode(inIPin, INPUT);
  pinMode(inQPin, INPUT);
  pinMode(ampPin, INPUT);
  digitalWrite(pwmIPin,pwmIValue);
  digitalWrite(transPin,pwmIValue);
  digitalWrite(pwmQPin,pwmQValue);
  intervalTimer.begin(pwmHandler, period);  
}

void loop() 
{
  period = period + freqSign * 0.01;
  if (period > 8.33) freqSign = -1;
  if (period < 6.25) freqSign =  1;
  intervalTimer.update(period);
  delay(10);
}
