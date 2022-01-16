// Librerias necesarias
#include <Adafruit_NeoPixel.h>
#include "OneButton.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// GPIO donde se conecta el aro
#define PIXELPIN       4

// Cantidad de leds que tiene el aro
#define NUMPIXELS      24

// GPIO donde se conecta el boton
#define BUTTON_TAP     3

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET); 

// Inicializo el boton
OneButton buttonTAP(BUTTON_TAP, false);

// Inicializo el aro
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);

// Variables
int playerIndex = -1;
int playerIndexTrail = -1;
int playerDirection = 1;
float nextMove = 0;
float currentPlayerSpeed = 100;
unsigned long countdown  = 0;
int enemyIndex = -1;
int coinIndex = -1;
int score = 0;
int bestScore = 0;
int lastScore = 0;
bool gameOver = false;

void setup()
{
  // Seteo el boton como entrada
  pinMode( BUTTON_TAP, INPUT );
  
  buttonTAP.attachClick(singleClick);
  buttonTAP.attachLongPressStart(longClick);

  // Inicializo el aro
  pixels.begin();
  pixels.setBrightness(20);

  countdown = millis() + 2000;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);//CONFIGURA LA DIRECCION DEL DISPLAY
  display.clearDisplay();//BORRAR DISPLAY
  display.setTextColor(SSD1306_WHITE);//TEXTO EN COLOR BLANCO
  display.setTextSize(1);//TAMAÑO DE LA LETRA (SE PUDE AJUSTRAR CON DECIMALES EJEMPLO (1.1, 1.2, 1.3...)
  display.setCursor(5,10);
  display.println("Iniciando...");//TEXTO EN PANTALLA
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
}

void loop()
{
  buttonTAP.tick();

  if ( gameOver )
    return;

  SetLevel();
  if ( countdown > millis() )
  {
    pixels.show();
    return;
  }
  DisplayPlayer();
  pixels.show();
}

// Limpio el nivel
void ClearLevel()
{
  for(int i=0;i<NUMPIXELS;i++)
      pixels.setPixelColor(i, pixels.Color(0,0,0));

  pixels.show();
}

// Muestro el mejor puntaje en amarillo, si el ultimo puntaje es menor al mejor puntaje lo muestro en rojo
void BestScore()
{
  // Mejor puntaje en amarillo
  for(int i=0;i<NUMPIXELS;i++)
  {
    if ( i < bestScore )
      pixels.setPixelColor(i, pixels.Color(255,155,0)); 
    else
      pixels.setPixelColor(i, pixels.Color(0,0,0)); 
  }

  if ( lastScore < bestScore )
  {
    for(int i=0;i<lastScore;i++)
      pixels.setPixelColor(i, pixels.Color(255,0,0)); 
  }
  pixels.show();
  display.setCursor(4,4);
  display.print("Ultimo Puntaje: ");
  display.print(lastScore);
  display.setCursor(4,18);
  display.print("Mejor Puntaje: ");
  display.print(bestScore);
  display.display();
  delay(3000);
  display.clearDisplay(); 
}

// Animacion de Game over
void GameOver()
{
  int a = enemyIndex;
  int b = enemyIndex;
  
  for(int i=0;i<NUMPIXELS/2;i++)
  {
      pixels.setPixelColor(a, pixels.Color(255,0,0)); 
      pixels.setPixelColor(b, pixels.Color(255,0,0)); 
      a = ( a + 1) % NUMPIXELS;
      b--;
      if ( b == -1 )
        b = NUMPIXELS;

      pixels.show();
      delay(20);
  }

  delay(100);

  a = enemyIndex;
  b = enemyIndex;
  
  for(int i=0;i<NUMPIXELS/2;i++)
  {
      pixels.setPixelColor(a, pixels.Color(0,0,0)); 
      pixels.setPixelColor(b, pixels.Color(0,0,0)); 

      a = ( a + 1) % NUMPIXELS;
      b--;
      if ( b == -1 )
        b = NUMPIXELS;

        pixels.show();
        delay(20);
  }

  // Mejor puntaje
  delay(100);
  BestScore();
}

// Seteo el nivel
void SetLevel()
{
  if ( enemyIndex < 0 )
  {
    if ( playerIndex < 0 )
    {
      enemyIndex = NUMPIXELS / 2;
    }
    else
    {
      enemyIndex = random(0, NUMPIXELS);

      while ( abs(enemyIndex - playerIndex ) < ( NUMPIXELS / 4 ) )
        enemyIndex = random(0, NUMPIXELS);
    }
  }
  if ( coinIndex < 0 )
  {
      coinIndex = random(0, NUMPIXELS);

      while ( abs(coinIndex - playerIndex ) < 7 || ( abs(coinIndex - enemyIndex ) < 7 ) )
        coinIndex = random(0, NUMPIXELS);
  }

  pixels.setPixelColor(enemyIndex, pixels.Color(255,0,0));
  pixels.setPixelColor(coinIndex, pixels.Color(255,255,0));
}

void DisplayPlayer()
{
  
  if ( nextMove < millis() )
  {
    nextMove = millis() + currentPlayerSpeed;

    if ( playerIndexTrail >= 0 )
        pixels.setPixelColor(playerIndexTrail, pixels.Color(0, 0, 0));

    if ( playerIndex >= 0)
    {
      pixels.setPixelColor(playerIndex, pixels.Color(0, 100, 0));
      playerIndexTrail = playerIndex;
    }

    playerIndex += playerDirection;

    if ( playerIndex < 0 )
      playerIndex = NUMPIXELS - 1;
    else if ( playerIndex == NUMPIXELS )
      playerIndex = 0;
    
    pixels.setPixelColor(playerIndex, pixels.Color(0, 255, 0));

    if ( playerIndex == coinIndex )
    {
      enemyIndex = -1;
      coinIndex = -1;
      score++;
      currentPlayerSpeed = constrain( currentPlayerSpeed - 10, 50, 150 );
      ClearLevel();
      pixels.setPixelColor(playerIndex, pixels.Color(0, 255, 0));
    }
    else if ( playerIndex == enemyIndex )
    {
      lastScore = score;
      if ( score >= bestScore )
        bestScore = score;
        
      GameOver();
      
      gameOver = true;
      enemyIndex = -1;
      coinIndex = -1;
      playerIndex = -1;
    }
  }
}

// Click simple de boton 
void singleClick()
{
  if ( countdown > millis() )
    return;

  playerDirection = -playerDirection;
}
  
// Click largo de boton
void longClick()
{
    gameOver = !gameOver;
    if ( gameOver )
    {
      enemyIndex = -1;
      coinIndex = -1;
      playerIndex = -1;
      currentPlayerSpeed = 150;
      ClearLevel();
    }
    else
    {
      ClearLevel();
      score = 0;
      currentPlayerSpeed = 150;
      countdown = millis() + 2000;
    }
}
