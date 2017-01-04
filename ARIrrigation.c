const int solPins[4] = {8, 9, 10, 11}; // Solenoid pins {sol1, sol2, sol3, sol4}
const int statusPins[3] = {3, 6, 5}; // Status LED pins {Red, Green, Blue}

const int testLED = 13;

// Set Manual Start pin
const int runPin = 2;

// For Debounce on Manual Start Pin
int runPinState = LOW;
int lastRunPinState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;

// For Triggering Full Watering Routine
int runWater = 0; //Trigger for routine run
int waterRunning = 0; //Tracks if routine currently running

int x = 0; //Arbitrary Counter for For loops

// For Running a Watering Routine
int runRoutine[10][5] = { //2D Array for Storing Routines. Enter Default Routine below. Each step runs for longest amount in minutes specified in each step.
    {10,0,0,0,0}, //Step 1. Format: {sol1,sol2,sol3,sol4,Delay} All run simultaneously. Delay specifies Delay between STARTS of steps.
    {0,7,0,0,0}, //Step 2
    {0,0,7,0,0},
    {0,0,0,10,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0},
    {0,0,0,0,0} //Step 10
    };
int runStep = 0; //Keep track of current routine step
int lastRunStep = 0; //for determining if the current step is different this loop
unsigned long stepRunTime = 0; //Maximum run time of current step
unsigned long stepTimeCurrent = 0; //current step and solenoid time for determining maximum for current step
unsigned long stepStart = 0; //Time that current step started

void setup() {
  for (x = 0; x < 4; x++) {
    pinMode(solPins[x], OUTPUT);
    digitalWrite(solPins[x], LOW);
  }
  pinMode(runPin, INPUT);
  for (x = 0; x < 3; x++) {
    pinMode(statusPins[x], OUTPUT);
    digitalWrite(statusPins[x], HIGH); // Test Status LEDs
    delay(333);
    digitalWrite(statusPins[x], LOW);
  }
  pinMode(testLED, OUTPUT);
  Serial.begin(9600);
  digitalWrite(testLED, HIGH);
  delay(500);
  digitalWrite(testLED, LOW);
  Serial.println("Initialised");
}

void loop() {
  // Debounce Manual Start Pin to see if full watering routine should be started
  int reading = digitalRead(runPin);
  if (reading != lastRunPinState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != runPinState) {
      runPinState = reading;
      runWater = runPinState;
    }
  }
  digitalWrite(testLED, runWater);
  // End Debounce
  lastRunPinState = reading;
  digitalWrite(testLED, runPinState);
  if (runWater == 1) { //Check for trigger state
    if (waterRunning == 0) { //Check that no routine running
      waterRunning = 1; //Note that routine has started
      runStep = 0; //Reset current step count
      findRunTime(runStep);
      Serial.println("Starting");
      Serial.print("First step will run for ");
      Serial.print(stepRunTime);
      Serial.println(" minutes.");
      stepRunTime = stepRunTime * 60 * 1000; //convert run time of step from minutes to milliseconds
      stepStart = millis();
    }
  }
  digitalWrite(statusPins[1], waterRunning);
  if (waterRunning == 1) { //Run until routine finishes
    for (x = 0; x < 4; x++) { //Check if each solenoid
      if (runRoutine[runStep][x] * 60 * 1000 > millis() - stepStart) { //Check if the current solenoid should still be running
        digitalWrite(solPins[x], HIGH); //Open the solenoid if it should be running
      } else {
        digitalWrite(solPins[x], LOW); //Close the solenoid if the timer has passed the run time
      }
    }
    if (stepStart + stepRunTime < millis()) { //check if it is time to move on to next step
      runStep++;
      if (runStep == 10) { //Check if the final step has been passed
        waterRunning = 0; //Reset routine running
        Serial.println("Finished");
        for (x = 0; x < 4; x++) { //For each solenoid
          digitalWrite(solPins[x], LOW); //Make sure solenoid isn't running
        }
      } else {
        findRunTime(runStep);
        Serial.print("Step ");
      	Serial.print(runStep + 1);
      	Serial.print(" will run for ");
      	Serial.print(stepRunTime);
      	Serial.println(" minutes.");
      	stepRunTime = stepRunTime * 60 * 1000; //convert run time of step from minutes to milliseconds
      }
      stepStart = millis();
    }
  }
}

int findRunTime(int x){
  stepRunTime = 0; //Reset run time of step
  for (y = 0; y < 5; y++) { //find Max run time of current step
    stepTimeCurrent = runRoutine[x][y];
    if (stepTimeCurrent > stepRunTime) {
      stepRunTime = stepTimeCurrent;
    }
  }
}