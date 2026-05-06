#include <Wire.h>
#include <TM1650.h>

//Pines a usar en ESP:

// Display 7-segmentos
const int PIN_DISP_DIO = 21;
const int PIN_DISP_CLK = 22;

// Led Hertz y Boton
const int PIN_LED_REF = 14;
const int PIN_BOTON = 33;

// Semáforo
const int PIN_SEM_R = 25;
const int PIN_SEM_Y = 26;
const int PIN_SEM_G = 27;

// LED RGB 
const int PIN_RGB_R = 19;
const int PIN_RGB_G = 18;
const int PIN_RGB_B = 5;

//Pines a usar en Arduino:
/*
// Led de referencia y Boton
const int PIN_LED_REF = 3;
const int PIN_BOTON = 2;

// Semáforo
const int PIN_SEM_R = 7;
const int PIN_SEM_Y = 8;
const int PIN_SEM_G = 9;

// LED RGB 
const int PIN_RGB_R = 4;
const int PIN_RGB_G = 5;
const int PIN_RGB_B = 6;
*/


//Estados de Juego y Semaforo
enum EstadoDeJuego {
  INICIO,
  JUGANDO,
  DERROTA
};

enum EstadoDeSemaforo {
  ROJO,
  AMARILLO,
  VERDE
};

EstadoDeJuego estadoJuego = INICIO;
EstadoDeSemaforo estadoSemaforo = ROJO;

//Variables Globales

// Tiempos (millis)
unsigned long tiempoAnteriorLed = 0;
unsigned long tiempoAnteriorBoton = 0;

// Lógica del LED de referencia
float frecuenciaLed = 1.0; // Inicia en 1 Hz
unsigned long intervaloLed = 500; // Mitad del periodo en ms (1000ms / 1Hz / 2)
bool estadoLedRef = false;

// Lógica del Semáforo
int parpadeosRojoObjetivo = 0; // Aleatorio entre 1 y 20
int parpadeosActuales = 0;

// Puntuación y Bonificadores
unsigned long puntuacion = 1;
int multiplicador = 1; // 1 a 5
bool botonPresionadoPreviamente = false;

//Display de 7 segmentos
TM1650 display;

void setup() {
  //Iniciamos Serial
  Serial.begin(9600);
  
  //Configuramos los pines en su modo correspondiente
  pinMode(PIN_LED_REF, OUTPUT);
  pinMode(PIN_BOTON, INPUT_PULLUP); 
  
  pinMode(PIN_SEM_R, OUTPUT);
  pinMode(PIN_SEM_Y, OUTPUT);
  pinMode(PIN_SEM_G, OUTPUT);
  
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);

  //Para usar random requerimos de una semilla, para que sea aleatorio, tomaremos una lectura de un pin analogico para que el ruido nos de esa aleatoriedad.
  randomSeed(analogRead(36)); 

  Wire.begin();
  display.init();
}

void loop() {
  //Tiempo actual de la ejecucion.
  unsigned long tiempoActual = millis();
  
  //Leemos si ha sido presionado el boton en cualquier momento de la ejecucion.
  bool botonPresionado = leerBoton(tiempoActual);
  
  //Esta linea esta para poder debuggear lo que hace el boton despues de presionado, aunque tambien ayuda para que el cambio entre pulsaciones sea mas tranquilo y con tiempo de reaccionar
  if (botonPresionado) {delay(3000);}

  //Logica principal del juego
  switch (estadoJuego) {
    case INICIO:
      iniciarJuego();
      break;
      
    case JUGANDO:
      actualizarReloj(tiempoActual); // Controla el parpadeo constante
      if (botonPresionado) {
        evaluarPulsacion();
      }
      break;
      
    case DERROTA:
      ejecutarAnimacionDerrota(tiempoActual);
      break;
  }
}

//Funciones del juego

void iniciarJuego() {
  // Inicializamos todos los datos en su valor de inicio predeterminado y empezamos el juego directamente.
  //Serial.println("iniciar Juego");
  puntuacion = 1;
  multiplicador = 1;
  frecuenciaLed = 1.0;
  actualizarIntervaloLed();
  
  estadoJuego = JUGANDO;
  reiniciarSecuenciaSemaforo();
  actualizarDisplay();
  actualizarLedRGB();
}

void actualizarIntervaloLed() {
  // Periodo (T) = 1 / Frecuencia. El intervalo es T/2 para encender/apagar
  //Serial.print("actualizo intervalo: ");
  intervaloLed = (1000.0 / frecuenciaLed) / 2.0; 
  //Serial.println(intervaloLed);
}

bool leerBoton(unsigned long tiempoActual) {
  bool estadoActualBoton = !digitalRead(PIN_BOTON);
  
  if (estadoActualBoton && !botonPresionadoPreviamente) {
    botonPresionadoPreviamente = true;
    Serial.println("BttnPresionado");
    return true; // Se acaba de presionar
  } else if (!estadoActualBoton) {
    botonPresionadoPreviamente = false;
  }
  return false;
}

void actualizarReloj(unsigned long tiempoActual) {
  // Esta función es el "corazón" del juego.
  if (tiempoActual - tiempoAnteriorLed >= intervaloLed) {
    tiempoAnteriorLed = tiempoActual;
    
    // Cambiar estado del LED de referencia
    estadoLedRef = !estadoLedRef;
    digitalWrite(PIN_LED_REF, estadoLedRef);
    
    // Solo avanzamos la lógica del semáforo cuando el LED completa un ciclo (se enciende)
    if (estadoLedRef == true) { 
      avanzarLogicaSemaforo();
    }
  }
}

void avanzarLogicaSemaforo() {
  //Serial.println("avanzo en logica");
  // Aquí controlamos el paso de ROJO -> AMARILLO -> VERDE
  if ((parpadeosActuales - 1) == parpadeosRojoObjetivo) {
      estadoSemaforo = AMARILLO;
  }
  else if((parpadeosActuales - 2) == parpadeosRojoObjetivo) {
    estadoSemaforo = VERDE;
  }
  else if ((parpadeosActuales - 3) == parpadeosRojoObjetivo) {
    estadoSemaforo = ROJO;
    reiniciarSecuenciaSemaforo();
    estadoJuego = DERROTA;
    
  }
  parpadeosActuales++;
  actualizarLedsSemaforo();
}

void evaluarPulsacion() {
  //Serial.println("LLego a EVALUAR PULSACION");
  
  switch (estadoSemaforo) {
    case ROJO:
      if (multiplicador != 1) {
        multiplicador = 1;
        //Mencionar en el display de 7 segmentos
        Serial.println("Te has salvado continua");
      }
      else {
        estadoJuego = DERROTA;
      }
      break;
    case VERDE:
      multiplicadorSumador();
      puntuacionSumador();
      actualizarDisplay();
      frecuenciaLed = frecuenciaLed + 0.1;
      actualizarIntervaloLed();
      reiniciarSecuenciaSemaforo();
      break;
    case AMARILLO:
      puntuacionSumador();
      frecuenciaLed = frecuenciaLed + 0.1;
      actualizarIntervaloLed();
      actualizarDisplay();
      reiniciarSecuenciaSemaforo();
      break;
  }
  Serial.print("Multiplicador: ");
  Serial.println(multiplicador);
}

void reiniciarSecuenciaSemaforo() {
  parpadeosRojoObjetivo = random(1, 21); // Entre 1 y 20
  //Serial.print("Objetivos de parpadeos: ");
  //Serial.println(parpadeosRojoObjetivo);
  parpadeosActuales = 0;
  estadoSemaforo = ROJO;
  actualizarLedsSemaforo();
}

void actualizarLedsSemaforo() {
  //Serial.println("Han cambiado los leds del semaforo.");
  //Apagar todos y encender solo el correspondiente a 'estadoSemaforo'
  switch(estadoSemaforo) {
    case ROJO: 
    digitalWrite(PIN_SEM_Y, LOW);
    digitalWrite(PIN_SEM_G, LOW);
    digitalWrite(PIN_SEM_R, HIGH);
    break;
    case AMARILLO: 
    digitalWrite(PIN_SEM_R, LOW);
    digitalWrite(PIN_SEM_G, LOW);
    digitalWrite(PIN_SEM_Y, HIGH);
    break;
    case VERDE: 
    digitalWrite(PIN_SEM_R, LOW);
    digitalWrite(PIN_SEM_Y, LOW);
    digitalWrite(PIN_SEM_G, HIGH);
    break;
  }
  
}

void actualizarLedRGB() {
  // Cambiar el color (R, G, B) según el valor de 'multiplicador' (x1 a x5)
  //Serial.println("Se ha actualizado el RGB");
  switch (multiplicador) {
    case 1:
      cambiarColorLed(0,0,1);
      break;
    case 2:
      cambiarColorLed(0,1,0);
      break;
    case 3:
      cambiarColorLed(0,1,1);
      break;
    case 4:
      cambiarColorLed(1,0,0);
      break;
    case 5:
      cambiarColorLed(1,0,1);
      break;
  }
}

void actualizarDisplay() {
  // Mandar el valor de 'puntuacion' al módulo 7-segmentos
  ///
  //Ejecucion para ESP
  display.displayOff();
  display.displayString("    ");
  display.displayOn();
  char str[30];
  char* line = itoa(puntuacion, str, 10);
  display.displayString(line);
  //*/
  //Ejecucion para arduino
  //Serial.print("Puntuacion:");
  //Serial.println(puntuacion);
}

void ejecutarAnimacionDerrota(unsigned long tiempoActual) {
  ///*
  //Ejecucion para ESP
  display.displayOff();
  display.displayString("    ");
  display.displayOn();
  char* str[] = "YOU LOSE"
  while (tiempoActual - milis() > 4000) {
  if (display.displayRunning(str)) {
    while (display.displayRunningShift()) delay(500);
  }
  delay(1000);
  actualizarDisplay();
  }
  */

  //Ejecucion para arduino
  //Serial.println("Perdiste.");
  //Serial.println(puntuacion);
  //delay(5000);
  estadoJuego = INICIO;
}

void ejecutarAnimacionInicio(){
  ///*
  //Ejecucion para ESP
  display.displayOff();
  display.displayString("    ");
  display.displayOn();
  char* str[] = "toca para empezar";
  while (tiempoActual - milis() > 4000) {
  if (display.displayRunning(str)) {
    while (display.displayRunningShift()) delay(500);
  }
  delay(1000);
  actualizarDisplay();
  }
  //*/
  //Ejecucion para arduino
  //Serial.println("Toca para empezar");
  delay(5000);
}

void cambiarColorLed(int r, int v, int a) {
  //Cambia el color del rgb, ya que este puede usar valores intermedios se usarian en formato analogo principalmente
  //de lo contrario su uso seria digital con 6 distintos colores.
  if ((r<255 && r>=0) && (v<255 && v>=0) && (a<255 && a>=0)){
      digitalWrite(PIN_RGB_R, r);
      digitalWrite(PIN_RGB_G, v);
      digitalWrite(PIN_RGB_B, a);
  }
}

void multiplicadorSumador() {
  //Aumenta el multiplicador y a su vez actualiza el estado del rgb
  if (multiplicador < 5) {
    multiplicador++;
    actualizarLedRGB();
  }
}

void puntuacionSumador() {
  //Suma para la puntuacion segun el multiplicador
  if (puntuacion == 0) {
    puntuacion++;
  }
  else {
    puntuacion = puntuacion + (1 * multiplicador);
  }
}