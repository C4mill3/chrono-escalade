/////////////////////////
// Boards: ESP32-WROOM-DA Module
// Boardsz Manager: esp32 2.0.14 //attention en 3.0 NeoMatrix est incompatible a l'heure actuelle

// Library:
// Adafruit BusIO 1.16.1
// Adafruit GFX Library 1.11.9
// Adafruit NeoMatreix 1.3.2
// Adafruit neoPixel 1.12.2

/////////////////////////


///////////////////////// Library -> Adafruit NeoMartrix by Adafruit
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Définition des broches
const int startButtonPin = 22; // broched de la plaque depart
const int stopButtonPin = 23; //broche du buzzer
const int obstacleSensorPin = 26;  // Broche du capteur infrarouge

// Variables
unsigned long startTime = 0;
unsigned long elapsedTime = 0;
unsigned long StartresetCooldown = 0;
unsigned long resetCooldown = 0;
bool isRunning = false;


// DATA PIN
#define DATA_PIN 2

// Configuration de la matrice
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, DATA_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

//Variable affichage
int luminosite=150;
unsigned long resetTime=2000; //temps nécessaire pour reset en millieme
int resetCooldownLed=0;
uint16_t chronoColor = matrix.Color(255, 0, 0);
uint16_t readychronoColor = matrix.Color(0, 255, 0);
uint16_t actualChronoColor = chronoColor;
uint16_t textColor = matrix.Color(0, 255, 255); //correspond au texte afficher via BLE
uint16_t cooldownColor = matrix.Color(255, 255, 255);

//////////Declaration des fonctions
void DetectStartStop();
void ShowChronoOnMatrix(String secondes, String precision);


void setup() {
  // Initialisation du port série
  Serial.begin(115200);

  // Configuration des boutons-poussoirs
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(obstacleSensorPin, INPUT_PULLUP);

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(luminosite); //regle la luminosite
}

void loop() {
  DetectStartStop();
  // Calcul du temps écoulé
  if (isRunning){
    elapsedTime = millis() - startTime;
  }
  // Pour générer une chaine de caractere au millieme prêt (DEC indique qu'on utilise la Base decimal, jsp pourquoi j'ai eu le droit a de l'hexa avant de le mettre)
  String secondes = String((elapsedTime / 10000) % 10, DEC) + String((elapsedTime / 1000) % 10, DEC);
  String precision = String((elapsedTime % 1000) / 100, DEC) + String((elapsedTime % 100) / 10, DEC) + String(elapsedTime % 10, DEC);
  ShowChronoOnMatrix(secondes,precision);
}

//////////////////////////////////////////////:

void DetectStartStop(){
  if (digitalRead(startButtonPin) == LOW){ // Tant que la plaque est enfoncé le chrono est reset 
    if (StartresetCooldown == 0) {
      StartresetCooldown = millis();
      resetCooldown = 0;
    } else if (resetCooldown < resetTime){
      resetCooldown = millis() - StartresetCooldown;
      resetCooldownLed = int(resetCooldown / (resetTime / 32));
    } else{ //reset
      resetCooldownLed = 32;
      actualChronoColor=readychronoColor;  // Couleur du texte
      startTime = millis();
      elapsedTime = millis() - startTime;
      isRunning = true;
    }

  }else{
    actualChronoColor=chronoColor;  // Couleur du texte
    StartresetCooldown = 0;
    resetCooldownLed = 0;
  }
  
  if ( ((digitalRead(obstacleSensorPin) == LOW) || (digitalRead(stopButtonPin) == LOW)) && isRunning ) {
    // Arrête le chronomètre si infra détecte ou si bouton presser
    elapsedTime = millis() - startTime;
    isRunning = false;

    String secondes = String((elapsedTime / 10000) % 10, DEC) + String((elapsedTime / 1000) % 10, DEC);
    String precision = String((elapsedTime % 1000) / 100, DEC) + String((elapsedTime % 100) / 10, DEC) + String(elapsedTime % 10, DEC);

  } else if (elapsedTime>=99999 && isRunning) {
    // Arrête le chronomètre si temps > 99,999 sec
    elapsedTime = 99999;
    isRunning = false;
  }

}

void ShowChronoOnMatrix(String secondes, String precision){
  /* affiche le txt donner en param sur la matrice et dans le serial monitor  */
  
  //Serial.println(secondes + "." + precision); //Affiche le texte dans la console pr debug

  matrix.fillScreen(0);  // Efface l'écran (remplit de noir tt les pixels)
  
  matrix.setTextColor(actualChronoColor);

  matrix.setCursor(0, 1);  // Place le curseur au coin supérieur gauche
  matrix.print(secondes);  // print le texte sur la matrice (en mémoire pour l'instant)
  
  //affiche le carré de pixel qui remplace le point/virgule
  matrix.drawRect(12, 6, 2, 2, actualChronoColor);

  matrix.setCursor(15, 1);  // Place le curseur au millieu supérieur gauche
  matrix.print(precision);  // print le texte sur la matrice (en mémoire pour l'instant)

  matrix.drawRect(-1, 0, resetCooldownLed+1, 1, cooldownColor); //bar de chargement 

  matrix.show();  // affiche ce qui est en mémoire
}
