/*
  Title: Digital Clock Final Project
  Name: Yu-Heng Lin(Lin)
  Date: 6/5/2024
  Purpose: building a digital clock that contains the following functionality
           1. It can display minutes using 7 segment display, and displaying hours using LEDs
           2. It allows users to set alarms that will beep when the clock becomes the designated time
           3. It has a differnt track of alarm hours and minutes that's independent of the real time
           4. Every hour the clock will beep a randomized tone
           5. Users are allowed to adjust the time(hours and minutes) for both real time and alarm time manually
*/

// variable setup
byte minHighNibble, minLowNibble;
int second = 0, minute = 58, hour = 1, randomNumber, temp, alarmHours = 8, alarmMinutes = 0; // default alarm is 8 Oclock
volatile int timeKeeper = 0;
bool isSettingAlarm = false; // for checking whether is setting alarm
// constants
const int HOUR_BUTTON = 4, MINUTE_BUTTON = 3, ALARM_BUTTON = 5, AM_PM_LIGHT_LED = 6, CARRY = 60, TONE_PIN = 7, PAUSE = 250;
const int toneArrSize = 5;
int toneValues[toneArrSize] = {523, 587, 659, 698, 880};


void setup() {
  // IO setup
  DDRB = B11111111;  // sets Arduino PortB pins 0 to 7 as outputs
  DDRC = B11111111;  // sets Arduino PortC pins 0 to 7 as outputs
  PORTC = hour; // Display the hours by outputting the binary value to Port C
  Serial.begin(9600);
  pinMode(HOUR_BUTTON, INPUT_PULLUP);
  pinMode(MINUTE_BUTTON, INPUT_PULLUP);
  pinMode(AM_PM_LIGHT_LED, OUTPUT);
  pinMode(ALARM_BUTTON, INPUT_PULLUP);

  // random seed
  randomSeed(analogRead(3));

  // interrupts setup
  attachInterrupt(0, tackleInputs, FALLING);
  attachInterrupt(0, playTones, FALLING);
  attachInterrupt (0, Sixty_Hz, RISING);  // Use pin 2, rising edge, jump to ISR called "Sixty_Hz" when triggered
}

void hourLight()
{
  // if it's setting alarm
  if (isSettingAlarm)
  {
    if (alarmHours >= 0 && alarmHours < 12)
    {
      PORTC = alarmHours;
      return;
    }
    // hour is >= 12
    temp = alarmHours - 12;
    PORTC = temp;
    return;
  }

  // not setting alarm
  if (hour >= 0 && hour < 12)
  {
    PORTC = hour;
    return;
  }
  // hour is >= 12
  temp = hour - 12;
  PORTC = temp;
}

int ampmLight() // am 1 pm 0
{
  // setting alarm
  if (isSettingAlarm)
  {
    if (alarmHours >= 0 && alarmHours <= 11)
    {
      digitalWrite(AM_PM_LIGHT_LED, HIGH);
      return 1;
    }
    digitalWrite(AM_PM_LIGHT_LED, LOW);
    return 0;
  }
  // not setting alarm
  if (hour >= 0 && hour <= 11)
  {
    digitalWrite(AM_PM_LIGHT_LED, HIGH);
    return 1;
  }
  digitalWrite(AM_PM_LIGHT_LED, LOW);
  return 0;
}

void manualIncrementAlarmHour()
{
  alarmHours++;
  if (alarmHours > 23) alarmHours = 0;
}

void manaulIncrementAlarmMinute()
{
  alarmMinutes++;
  if (alarmMinutes > 59)
  {
    alarmMinutes = 0;
    manualIncrementAlarmHour();
  }
}


void manualIncrementHour()
{
  hour++;
  if (hour > 23) hour = 0;
  playTones();
}

void manualIncrementMin()
{
  // increment minute
  minute++;
  // if minute exceeds
  if (minute > 59)
  {
    minute = 0;
    manualIncrementHour();
  }
}


void incrementTime()
{
  second++;
  // if not exceeds do nothing
  if (second <= 59) return;

  // if second exceeds 59, then go 0
  second = 0;

  // handles min and hour changes
  manualIncrementMin();
}



void debounce(int button)
{
  // debouncing noise
  while (!digitalRead(button)) delay(10);
}

void readDigitalInputButton()
{
  // checking hour and minute button inputs and do action based on the state(whether the alarm button is held or not)
  if (!digitalRead(HOUR_BUTTON))
  {
    debounce(HOUR_BUTTON);

    if (isSettingAlarm)
    {
      manualIncrementAlarmHour();
      terminalPrint(); // print if it's alarm mode
      displayMinutes(); // get the alarm hours and min displayed as well
      hourLight();
    }
    else manualIncrementHour();
    return;
  }
  if (!digitalRead(MINUTE_BUTTON))
  {
    debounce(MINUTE_BUTTON);
    if (isSettingAlarm)
    {
      manaulIncrementAlarmMinute();
      terminalPrint();
      displayMinutes(); // get the alarm hours and min displayed as well
      hourLight();
    }
    else manualIncrementMin();
    return;
  }
}

void tackleAlarmSetting(int button)
{
  
  isSettingAlarm = false;
  if (!digitalRead(button))
  {
    isSettingAlarm = true;
    terminalPrint(); // print if it's alarm mode
    displayMinutes(); // get the alarm hours and min displayed as well
    hourLight();
  }
  // enter a dead loop as long as the alarm button is held
  while (!digitalRead(button))
  {
    isSettingAlarm = true;
    readDigitalInputButton();
    delay(10);
  }
}

void tackleInputs()
{
  // check if it's setting alarm
  tackleAlarmSetting(ALARM_BUTTON);
  readDigitalInputButton();
}


void terminalPrint()
{
  if (isSettingAlarm)
  {
    Serial.print("Alarm Time: ");
    Serial.print(alarmHours);
    Serial.print(":\t");
    Serial.print(alarmMinutes);
    if (!ampmLight()) Serial.println(" PM");
    else Serial.println(" AM");
    return;
  }

  Serial.print("Real Time: ");
  Serial.print(hour);
  Serial.print(":\t");
  Serial.print(minute);
  Serial.print(":\t");
  Serial.print(second);
  Serial.print("\t");
  if (!ampmLight()) Serial.println(" PM");
  else Serial.println(" AM");
}

void playAlarm()
{
  if (!(alarmHours == hour && alarmMinutes == minute && second < 5)) return;
  terminalPrint();
  tone(TONE_PIN, toneValues[4]);
  delay(PAUSE);
  noTone(TONE_PIN);
  incrementTime();
}

void playTones()
{
  // play for 250 ms
  randomNumber = random(0, 4);
  terminalPrint();
  displayMinutes();
  hourLight();
  tone(TONE_PIN, toneValues[randomNumber]);
  delay(PAUSE);
  noTone(TONE_PIN);
  incrementTime();
}

void Sixty_Hz() // This is the interrupt (ISR) that is triggered by a low to high transition on pin 2
{
  timeKeeper++;
}


void displayMinutes()
{
  if (isSettingAlarm)
  {
    minHighNibble = alarmMinutes / 10;
    minLowNibble = alarmMinutes % 10; // Perform modulo operation to convert
  }
  else
  {
    minHighNibble = minute / 10;
    minLowNibble = minute % 10; // Perform modulo operation to convert
  }

  PORTB = minLowNibble;        // data from binary to BCD
  PORTB = PORTB | B00010000;   // Pulse the 1st digit data load input
  PORTB = PORTB & B11101111;
  PORTB = minHighNibble;
  PORTB = PORTB | B00100000;   // Pulse the 2nd digit data load input
  PORTB = PORTB & B11011111;
}


void loop() {
  // Main code goes here...
  displayMinutes();     // Call the function to display the minutes on the 7-segment displays
  hourLight();
  if (timeKeeper >= CARRY)  // 1 second has passed
  {
    terminalPrint();
    timeKeeper -= CARRY;
    incrementTime();
  }
  // take inputs, and check AM PM and alarms
  tackleInputs();
  ampmLight();
  playAlarm();
}
