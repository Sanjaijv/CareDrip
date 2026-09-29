// ==========================================
// CAREDRIP
// IR SENSOR + 3 LEDs + BUZZER
// ==========================================

// -------------------------------
// PIN CONFIGURATION
// -------------------------------

#define IR_PIN       18
#define BUZZER_PIN   23

#define GREEN_LED    19
#define YELLOW_LED   17
#define RED_LED      16


// -------------------------------
// DRIP SETTINGS
// -------------------------------

// 10-second measurement window
const unsigned long WINDOW_TIME = 10000;

// Normal IV range
const float MIN_NORMAL_RATE = 10.0;
const float MAX_NORMAL_RATE = 60.0;

// Minimum time between two valid detections
// This removes IR sensor noise.
const unsigned long DEBOUNCE_TIME = 250;


// -------------------------------
// VARIABLES
// -------------------------------

volatile unsigned long dropCount = 0;

volatile unsigned long lastDetectionTime = 0;

unsigned long windowStart = 0;

float dripRate = 0;


// ==========================================
// IR SENSOR INTERRUPT
// ==========================================

void IRAM_ATTR detectDrop()
{
  unsigned long currentTime = millis();

  // Ignore extremely fast false transitions
  if (currentTime - lastDetectionTime >= DEBOUNCE_TIME)
  {
    dropCount++;

    lastDetectionTime = currentTime;
  }
}


// ==========================================
// LED FUNCTIONS
// ==========================================

void allOff()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  digitalWrite(BUZZER_PIN, LOW);
}


// ------------------------------------------
// NORMAL
// ------------------------------------------

void normalState()
{
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  digitalWrite(BUZZER_PIN, LOW);
}


// ------------------------------------------
// SLOW
// ------------------------------------------

void slowState()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  digitalWrite(BUZZER_PIN, HIGH);
}


// ------------------------------------------
// CRITICAL
// ------------------------------------------

void criticalState()
{
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  digitalWrite(BUZZER_PIN, HIGH);
}


// ==========================================
// SETUP
// ==========================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("        CAREDRIP SYSTEM");
  Serial.println("=================================");

  Serial.println("IR SENSOR : GPIO 18");
  Serial.println("BUZZER    : GPIO 23");
  Serial.println("GREEN LED : GPIO 19");
  Serial.println("YELLOW LED: GPIO 17");
  Serial.println("RED LED   : GPIO 16");

  Serial.println();
  Serial.println("Starting system...");


  // -------------------------------
  // PIN MODES
  // -------------------------------

  pinMode(IR_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);


  // Everything OFF
  allOff();


  // -------------------------------
  // IR INTERRUPT
  // -------------------------------

  attachInterrupt(
    digitalPinToInterrupt(IR_PIN),
    detectDrop,
    FALLING
  );


  windowStart = millis();


  Serial.println("System ready.");
  Serial.println();
}


// ==========================================
// MAIN LOOP
// ==========================================

void loop()
{
  unsigned long currentTime = millis();


  // ========================================
  // CHECK 10-SECOND WINDOW
  // ========================================

  if (currentTime - windowStart >= WINDOW_TIME)
  {

    // Safely copy counter
    noInterrupts();

    unsigned long drops = dropCount;

    dropCount = 0;

    interrupts();


    // ======================================
    // CALCULATE DROPS/MIN
    // ======================================

    dripRate =
      drops * (60000.0 / WINDOW_TIME);


    windowStart = currentTime;


    // ======================================
    // DISPLAY
    // ======================================

    Serial.println();
    Serial.println("---------------------------------");

    Serial.print("Drops detected : ");
    Serial.println(drops);

    Serial.print("Drip rate      : ");
    Serial.print(dripRate, 1);
    Serial.println(" drops/min");


    // ======================================
    // NO DRIP
    // ======================================

    if (drops == 0)
    {
      criticalState();

      Serial.println("STATUS         : CRITICAL");
      Serial.println("ALERT          : NO DRIP");
      Serial.println("RED LED        : ON");
      Serial.println("BUZZER         : ON");
    }


    // ======================================
    // TOO SLOW
    // ======================================

    else if (dripRate < MIN_NORMAL_RATE)
    {
      slowState();

      Serial.println("STATUS         : WARNING");
      Serial.println("ALERT          : DRIP TOO SLOW");
      Serial.println("YELLOW LED     : ON");
      Serial.println("BUZZER         : ON");
    }


    // ======================================
    // TOO FAST
    // ======================================

    else if (dripRate > MAX_NORMAL_RATE)
    {
      criticalState();

      Serial.println("STATUS         : CRITICAL");
      Serial.println("ALERT          : DRIP TOO FAST");
      Serial.println("RED LED        : ON");
      Serial.println("BUZZER         : ON");
    }


    // ======================================
    // NORMAL
    // ======================================

    else
    {
      normalState();

      Serial.println("STATUS         : NORMAL");
      Serial.println("GREEN LED      : ON");
      Serial.println("BUZZER         : OFF");
    }


    Serial.println("---------------------------------");
  }


  delay(5);
}