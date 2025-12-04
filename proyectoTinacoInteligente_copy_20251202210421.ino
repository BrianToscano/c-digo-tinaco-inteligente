#include <LiquidCrystal.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Ultrasonic sensor pins
const int PinTrig = 7;
const int PinEcho = 6;

// ——— CONFIGURACIÓN DEL SISTEMA ———
const float DIST_VACIO = 36.0;   // cm → recipiente vacío (0 ml)
const float DIST_LLENO = 0.0;     // cm → recipiente lleno (20000 ml)
const int CAPACIDAD_MAX = 18500;   // ml

// Filtro: promedio de 3 lecturas
const int NUM_MUESTRAS = 3;
float lecturas[NUM_MUESTRAS];
int idx = 0;
bool primeraMedia = false;

void setup() {
  Serial.begin(9600);
  pinMode(PinTrig, OUTPUT);
  pinMode(PinEcho, INPUT);
  lcd.begin(16, 2);

  // Inicializar buffer
  for (int i = 0; i < NUM_MUESTRAS; i++) {
    lecturas[i] = DIST_VACIO;
  }

  lcd.print("Iniciando...");
  delay(1000);
}

void loop() {
  // Medir distancia
  float d = medirDistancia();

  // Guardar en buffer circular
  lecturas[idx] = d;
  idx = (idx + 1) % NUM_MUESTRAS;

  // Calcular promedio
  float suma = 0;
  for (int i = 0; i < NUM_MUESTRAS; i++) suma += lecturas[i];
  float media = suma / NUM_MUESTRAS;

  if (idx == 0) primeraMedia = true;

  if (primeraMedia) {
    // Limitar al rango válido
    if (media > DIST_VACIO) media = DIST_VACIO;
    if (media < DIST_LLENO) media = DIST_LLENO;

    // Calcular volumen y porcentaje
    float rango = DIST_VACIO - DIST_LLENO; // 300 cm
    float ml = CAPACIDAD_MAX * (DIST_VACIO - media) / rango;
    int porcentaje = (int)(ml / CAPACIDAD_MAX * 100 + 0.5); // redondeo

    // Mostrar en LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print((int)ml); lcd.print(" ml");

    lcd.setCursor(0, 1);
    lcd.print(porcentaje); lcd.print(" %");

    // Depuración
    Serial.print("Dist: "); Serial.print(media, 1);
    Serial.print(" cm | ");
    Serial.print((int)ml); Serial.println(" ml");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Calibrando ");
    lcd.print(idx + 1);
  }

  delay(500);
}

// ——— FUNCION DE MEDICION ———
float medirDistancia() {
  digitalWrite(PinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(PinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(PinTrig, LOW);

  unsigned long duracion = pulseIn(PinEcho, HIGH, 50000); // timeout 50 ms (~8.5 m max)

  if (duracion == 0) {
    return DIST_VACIO + 10.0; // forzar "vacío" si no hay eco
  }

  float distancia = duracion * 0.034 / 2.0; // cm

  // Validar rango físico
  if (distancia < 0 || distancia > 500) {
    return DIST_VACIO + 10.0;
  }

  return distancia;
}