#include <Wire.h>
#include <TM1650.h>

//Pines a usar:

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

//Estados de Juego y Semaforo
enum GameState {
  INICIO,
  JUGANDO,
  DERROTA
};

enum SemaforoState {
  ROJO,
  AMARILLO,
  VERDE
};

GameState estadoJuego = INICIO;
SemaforoState estadoSemaforo = ROJO;

// 3. VARIABLES GLOBALES (Temporizadores y Lógica)

// Tiempos (millis)
unsigned long tiempoAnteriorLed = 0;
unsigned long tiempoAnteriorBoton = 0;
const unsigned long DEBOUNCE_DELAY = 50; // Antirrebote para el botón

// Lógica del LED de Hertz
float frecuenciaLed = 1.0; // Inicia en 1 Hz
unsigned long intervaloLed = 500; // Mitad del periodo en ms (1000ms / 1Hz / 2)
bool estadoLedRef = false;

// Lógica del Semáforo
int parpadeosRojoObjetivo = 0; // Aleatorio entre 1 y 20
int parpadeosActuales = 0;

// Puntuación y Bonificadores
int puntuacion = 0;
int multiplicador = 1; // 1 a 5
bool botonPresionadoPreviamente = false;

//Display de 7 segmentos
TM1650 display;

// ==========================================
// 4. CONFIGURACIÓN INICIAL
// ==========================================
void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_LED_REF, OUTPUT);
  pinMode(PIN_BOTON, INPUT); 
  
  pinMode(PIN_SEM_R, OUTPUT);
  pinMode(PIN_SEM_Y, OUTPUT);
  pinMode(PIN_SEM_G, OUTPUT);
  
  pinMode(PIN_RGB_R, OUTPUT);
  pinMode(PIN_RGB_G, OUTPUT);
  pinMode(PIN_RGB_B, OUTPUT);
  
  // Inicializar semilla aleatoria para el semáforo leyendo un pin analógico al aire
  randomSeed(analogRead(34)); 

  Wire.begin()
  display.init();
  iniciarJuego();
}

// ==========================================
// 5. BUCLE PRINCIPAL (No bloqueante)
// ==========================================
void loop() {
  unsigned long tiempoActual = millis();
  
  // 1. Leer entradas
  bool botonPresionado = leerBoton(tiempoActual);
  
  // 2. Ejecutar lógica según el estado del juego
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

// 6. FUNCIONES DE LÓGICA (Para implementar)

void iniciarJuego() {
  puntuacion = 0;
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
  intervaloLed = (1000.0 / frecuenciaLed) / 2.0; 
}

bool leerBoton(unsigned long tiempoActual) {
  // Aquí debes implementar la lógica de lectura con "debounce" (antirrebote)
  // y detectar solo el "flanco de subida" (cuando pasa de NO presionado a presionado)
  bool estadoActualBoton = digitalRead(PIN_BOTON);
  
  // Implementación básica (se debe mejorar con el debounce):
  if (estadoActualBoton && !botonPresionadoPreviamente) {
    botonPresionadoPreviamente = true;
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
  // Aquí controlas el paso de ROJO -> AMARILLO -> VERDE
  // usando 'parpadeosActuales' y 'parpadeosRojoObjetivo'.
  if ((parpadeosActuales - 1) == parpadeosRojoObjetivo) {
      estadoSemaforo = AMARILLO
  }
  else if((parpadeosActuales - 2) == parpadepsRojoObjetivo) {
    estadoSemaforo = VERDE
  }
  parpadeosActuales++;
  actualizarLedsSemaforo();
}

void evaluarPulsacion() {
  // ¿El semáforo está en ROJO? -> Evaluar si hay salvavidas o ir a DERROTA
  // ¿El semáforo NO está en rojo? -> Acierto. Sumar puntos, subir Hz, reiniciar semáforo.
  // ¿El semáforo está en VERDE? -> Subir nivel del multiplicador.
}

void reiniciarSecuenciaSemaforo() {
  parpadeosRojoObjetivo = random(1, 21); // Entre 1 y 20
  parpadeosActuales = 0;
  estadoSemaforo = ROJO;
  actualizarLedsSemaforo();
}

void actualizarLedsSemaforo() {
  // Apagar todos y encender solo el correspondiente a 'estadoSemaforo'
  switch(estadoSemaforo) {
    case ROJO: 
    digitalWrite(PIN_SEM_R, LOW);
    digitalWrite(PIN_SEM_Y, LOW);
    digitalWrite(PIN_SEM_G, LOW);
    digitalWrite(PIN_SEM_R, HIGH);
    break;
    case AMARILLO: 
    digitalWrite(PIN_SEM_R, LOW);
    digitalWrite(PIN_SEM_Y, LOW);
    digitalWrite(PIN_SEM_G, LOW);
    digitalWrite(PIN_SEM_Y, HIGH);
    break;
    case VERDE: 
    digitalWrite(PIN_SEM_R, LOW);
    digitalWrite(PIN_SEM_Y, LOW);
    digitalWrite(PIN_SEM_G, LOW);
    digitalWrite(PIN_SEM_G, HIGH);
    break;
  }
  
}

void actualizarLedRGB() {
  // Cambiar el color (R, G, B) según el valor de 'multiplicador' (x1 a x5)
  switch (multiplicador) {
    case 1:
      cambiarColorLed(0,0,0);
      break;
    case 2:
      cambiarColorLed(1,1,1);
      break;
    case 3:
      cambiarColorLed(2,2,2);
      break;
    case 4:
      cambiarColorLed(3,3,3);
      break;
    case 5:
      cambiarColorLed(4,4,4);
      break;
  }
}

void actualizarDisplay() {
  // Mandar el valor de 'puntuacion' al módulo 7-segmentos
  display.displaOff();
  display.displayString("    ");
  display.displayOn();
  char str[30];
  char* line 0 itoa(puntuacion, str, 10);
  display.displayString(line);

}

void ejecutarAnimacionDerrota(unsigned long tiempoActual) {
  // Lógica no bloqueante para parpadear luces/display de derrota.
  // Después de un tiempo, cambiar estadoJuego = INICIO;
}

void cambiarColorLed(int r, int v, int a) {
  if ((r<255 %% r>=0) && (v<255 %% v>=0) && (a<255 %% a>=0)){
      analogWrite(PIN_RGB_R, r);
      analogWrite(PIN_RGB_G, v);
      analogWrite(PIN_RGB_B, a);
  }
}