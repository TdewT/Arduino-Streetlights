#include <Arduino.h>
#include <map>
//FIXME: Doesn't work due to timer issues, won't fix
//#include <Arduino_LED_Matrix.h>
//#include "MatrixForPhotoresistor.h"

#define lightSensorPin A0
#define buttonPin 13
//FIXME: Doesn't work due to timer issues, won't fix
//ArduinoLEDMatrix matrix;

using namespace std;

class StreetLight {
private:
    // Last state of flashYellow
    bool flashYellowOn = true;
    // Delay when changing from yellow to other colors
    const int yellowTimeout = 1500;
    // Delay to wait between red and green lights change
    const int mainTimeout = 8000;
    // Time when last changeLight() was executed
    unsigned long lastMillis = 0;
    // Last light color
    string lastState;
    // Stores current states of all lights
    std::map<string, bool> lightStates{};
    // Interval of blinking in flashYellow mode
    const int flashYellowInterval = 500;

public:
    // Stores which led color has which pin
    std::map<string, int> lightPins;

    explicit StreetLight(const std::map<string, int> &lightPins, const string &startingLight) {
        // Load colors and pins from dictionary
        this->lightPins = lightPins;
        // Add all lights to lightStates
        for (const pair<const basic_string<char>, int> &light: lightPins) {
            lightStates[light.first] = false;
        }
        // Set the starting color of the streetlight
        this->lightStates[startingLight] = true;
    }

    void changeLight(unsigned long millis, int &lightStrength) {
        // Disable yellow light if it was turned on by flashing yellow mode
        if (flashYellowOn && !this->lightStates["yellow"]) {
            lightOff("yellow");
        }

        // Update strength of the light when color is not changing
        for (const pair<const basic_string<char>, bool> &state: lightStates) {
            if (state.second) {
                lightOn(state.first, lightStrength);
            }
        }


        // Change light from yellow
        if (this->lightStates["yellow"]) {
            // If the last state before turning yellow was green then change to red
            if (this->lastState == "green" && millis - lastMillis > yellowTimeout) {
                lightOff("yellow");
                lightOn("red", lightStrength);
            }
                // If the last state before turning yellow was red then change to green
            else if (this->lastState == "red" && millis - lastMillis > yellowTimeout) {
                lightOff("yellow");
                lightOn("green", lightStrength);
            }
        }
        else {
            // If the light is green
            if (this->lightStates["green"]) {
                // If enough time has passed change the color to yellow
                if (millis - lastMillis > mainTimeout) {
                    this->lastState = "green";
                    lightOff("green");
                    lightOn("yellow", lightStrength);
                    this->lastMillis = millis;
                }
            }
                // If the light is red
            else {
                // If enough time has passed change the color to yellow
                if (millis - lastMillis > mainTimeout) {
                    this->lastState = "red";
                    lightOff("red");
                    lightOn("yellow", lightStrength);
                    this->lastMillis = millis;
                }
            }
        }
    }

    // Turn on the light
    void lightOn(string color, int &lightStrength) {
        analogWrite(this->lightPins[color], lightStrength);
        lightStates[color] = true;
    }

    // Turn off the light
    void lightOff(string color) {
        analogWrite(this->lightPins[color], 0);
        lightStates[color] = false;
    }


    // flashing yellow lights mode
    void flashYellow(unsigned long millis, int strength = 255) {
        // Time since the last change of the state of the light
        static unsigned long lastBlinkMillis = 0;
        // Helps main function deal with yellow light that would
        // otherwise stay on after changing back to normal mode
        flashYellowOn = true;
        // Determines if the light is on or off
        static bool blinkState = false;

        // Determines the value of blinkState
        if (millis - lastBlinkMillis >= this->flashYellowInterval) {
            blinkState = !blinkState;
            lastBlinkMillis = millis;
        }

        // Override main functions light states
        analogWrite(this->lightPins["red"], 0);
        analogWrite(this->lightPins["green"], 0);

        // Turn the light on or off based on the blinkState
        if (blinkState) {
            analogWrite(this->lightPins["yellow"], strength);
        }
        else {
            analogWrite(this->lightPins["yellow"], 0);
        }
    }
};

// Create an array of streetLight objects
StreetLight streetLights[] = {
        StreetLight({
                            {"red",    0},
                            {"yellow", 1},
                            {"green",  2}},
                    "green"),
        StreetLight({
                            {"red",    3},
                            {"yellow", 4},
                            {"green",  5}
                    }, "red"),
        StreetLight({
                               {"red",    6},
                               {"yellow", 7},
                               {"green",  8}
                       }, "red"),
        StreetLight({
                                {"red",    9},
                                {"yellow", 10},
                                {"green",  11}
                        }, "green")
};

[[maybe_unused]] void setup() {
    //FIXME: Doesn't work due to timer issues, won't fix
//    matrix.begin();

    Serial.begin(9600);

    // Loop through all streetLight objects and initialise appropriate pinModes
    for (const StreetLight &streetLight: streetLights) {
        for (const auto &[key, value]: streetLight.lightPins) {
            pinMode(value, OUTPUT);
        }
    }
    // Setup additional pins
    pinMode(buttonPin, INPUT_PULLDOWN);
    pinMode(lightSensorPin, INPUT_PULLDOWN);
}

// Button variables
// Boolean determining if flashing yellow mode on lights should be enabled
bool flashingYellow = false;
// Time to wait before recording new button press
const int debounceDelay = 500;
// Time since last button press
unsigned long debounceMillis = 0;

// Photo-resistor variables
// Time since last highestLightValue was reset
unsigned long lastReset = 0;
// Time after which highestLightValue will reset
const int lightResetDelay = 60000;
// Time of the last sensor read
unsigned long lastRead = 0;
// Time to wait between readings
const int readDelay = 500;
// Current sensor reading
int lightSensorVal = 0;
// Highest recorded reading
int highestLightValue = 0;


void loop() {
    // Get current running time
    unsigned long timeNow = millis();

    // Get photo-resistor's reading
    if (timeNow - lastRead > readDelay) {
        // Read data from sensor
        lightSensorVal = analogRead(lightSensorPin);
        // Update time of last read
        lastRead = timeNow;


        // If value is higher than the highest recorded, than set that as new high
        if (lightSensorVal > highestLightValue || timeNow - lastReset > lightResetDelay) {
            highestLightValue = lightSensorVal;

            // Update time of last reset
            lastReset = timeNow;
        }
    }

    //FIXME: Doesn't work due to timer issues, won't fix
    // Matrix stuff
//    int lightPercent = ::map(lightSensorVal, 0, highestLightValue - 10, 0, 96);
//    auto frame = generate_frame(lightPercent);
//    matrix.loadFrame(frame.data());


    // Read button state
    if (digitalRead(buttonPin) && timeNow - debounceMillis > debounceDelay) {
        // Reset timeout on button press
        debounceMillis = timeNow;
        // Change mode to flashingYellow
        flashingYellow = !flashingYellow;
    }

    Serial.println(lightSensorVal);

    // Calculate new streetlight strength based on ambient lighting
    int lightStrength = ::map(lightSensorVal, 0, highestLightValue, 0, 200);

    // Check if flashing yellow mode is active
    if (!flashingYellow) {
        for (StreetLight &streetLight: streetLights) {
            // Change the lights (individual timers are inside the class)
            streetLight.changeLight(timeNow, lightStrength);
        }
    }
    else {
        // Engage flashing yellow mode for every streetlight
        for (StreetLight &streetLight: streetLights) {
            streetLight.flashYellow(timeNow, lightStrength);
        }
    }
}
