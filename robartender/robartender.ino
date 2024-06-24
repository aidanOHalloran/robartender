#include <AFMotor.h>

// Constants for the setup configuration
const int BUTTON_PIN = 2;
const int GREEN_LED_PIN = 10;
const int BLUE_LED_PIN = A1;
const int RED_LED_PIN = 9;

// Stepper motor setup
AF_Stepper motor1(48, 1);
AF_Stepper motor2(48, 2);

// Machine state variables
bool raftFound = false;
bool drinkRequested = false;

// Drink configuration matrix setup
const int OPTIC_COUNT = 4;
const int PARAMETER_COUNT = 3; // Represents the number of parameters per optic (position, delay, repeat)
int drinkMatrix[OPTIC_COUNT][PARAMETER_COUNT] = {0};
//drinkMatrix[optic][0] is the distance needed to reach given optic station
//drinkMatrix[optic][1] is the delay time specified for given optic station
//drinkMatrix[optic][2] is the number of repeats specified for given optic station



// Function declarations
void checkArray();
void setColor(int red, int green, int blue);

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  motor1.setSpeed(600);
  motor2.setSpeed(600);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
}

void loop() {
  // Raft location detection
  if (!raftFound) {
    Serial.println("Raft location not known yet");
    setColor(255, 0, 0); // Set LED to red indicating search for raft
    delay(1000); // Short delay before searching
    Serial.print("Looking for the raft...");
    while (digitalRead(BUTTON_PIN) == LOW) { //starting point (contact switch) not reached
      motor1.step(10, FORWARD, SINGLE); //move raft towards starting point (contact switch)
    }
    raftFound = true; //raft has reached contact switch
    Serial.println("Raft found! :)");
    motor1.release(); //stop moving towards contact switch
    setColor(0, 255, 0); // Set LED to green indicating raft found
    delay(700);
  }

  // Process drink dispensing
  if (drinkRequested) {
    for (int optic = 0; optic < OPTIC_COUNT; optic++) {
      // Move to next pump position
      motor1.step(drinkMatrix[optic][0] * 10, BACKWARD, SINGLE);
      motor1.release();

      // Dispense the drink
      while (drinkMatrix[optic][2] > 0) { //while 3rd parameter, or number of repeats at given optic station, is > 0
        delay(500); //half second wait
        motor2.step(2100, BACKWARD, DOUBLE); //twist dispenser rods (motor 2) to push trigger up
        delay(drinkMatrix[optic][1] * 100); //wait specified amount
        motor2.step(2100, FORWARD, DOUBLE); //twist dispenser rods opposite direction to bring 
        motor2.release();
        drinkMatrix[optic][2]--; //reduce the repeat parameter each go around
        delay(500);
      }
    }

    Serial.println("Drinks ready, enjoy.");
    setColor(0, 0, 255); // Set LED to blue indicating drink is ready
    drinkRequested = false; //reset variable
    raftFound = false; //reset variable
  }

  // Read drink configuration from serial
  if (!drinkRequested && Serial.available()) { //if no drink already requested and data waiting in serial buffer
    int numberCollector = 0; //variable used to assemble or "collect" numbers from a sequence of digits received one at a time.
    for (int optic = 0; optic < OPTIC_COUNT; optic++) { //for every optic specified in setup
      for (int parameter = 0; parameter < PARAMETER_COUNT; parameter++) { //for every parameter per optic
        for (int digit = 0; digit < 2; digit++) { // Assume each parameter is two digits
          while (!Serial.available()) delay(50); // Wait for data
          char ch = Serial.read(); //read in the data
          if (ch >= '0' && ch <= '9') { // Ensure it's a digit
            numberCollector = numberCollector * 10 + (ch - '0'); // x*10 pushes the number left a decimal position
            //for example: 1, 2, 3 becomes 123
          }
        }
        drinkMatrix[optic][parameter] = numberCollector;
        numberCollector = 0;
        Serial.read(); // Skip delimiter (e.g., comma)
      } 
    }
    checkArray();
    drinkRequested = true;
  }
}

void checkArray() {
  for (int i = 0; i < OPTIC_COUNT; i++) {
    for (int j = 0; j < PARAMETER_COUNT; j++) {
      Serial.print(drinkMatrix[i][j]);
      Serial.print(",");
    }
    Serial.println();
  }
}

void setColor(int red, int green, int blue) {
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(RED_LED_PIN, red);
  analogWrite(GREEN_LED_PIN, green);
  analogWrite(BLUE_LED_PIN, blue);  
}
