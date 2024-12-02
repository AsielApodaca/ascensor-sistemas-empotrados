#include <AccelStepper.h>
#include <ESP32Servo.h>
#include <NoDelay.h>

// Pines y constantes
#define PIN_MOTOR_IN1 25
#define PIN_MOTOR_IN2 26
#define PIN_MOTOR_IN3 27
#define PIN_MOTOR_IN4 14

#define PIN_SENSOR_PISO1 33
#define PIN_SENSOR_PISO2 32
#define PIN_SENSOR_PISO3 35
#define PIN_SENSOR_OBSTRUCCION 34

#define PIN_LED_PISO1 2
#define PIN_LED_PISO2 4
#define PIN_LED_PISO3 16

#define PIN_BOTON_PISO1 23
#define PIN_BOTON_PISO2 22
#define PIN_BOTON_PISO3 21

#define PIN_SERVO_PUERTA 13

AccelStepper motor(AccelStepper::FULL4WIRE, PIN_MOTOR_IN1, PIN_MOTOR_IN3, PIN_MOTOR_IN2, PIN_MOTOR_IN4);
Servo servoPuerta;

enum Piso { piso1, piso2, piso3, ningunPiso };
Piso estadoActualPiso = ningunPiso;

const int tiempoPuerta = 2000; // 2 segundos
const int tiempoParpadeo = 500; // 0.5 segundos

noDelay temporizadorPuerta(tiempoPuerta, false);
noDelay temporizadorParpadeo(tiempoParpadeo);

bool puertaAbierta = false;

void setup() {
  pinMode(PIN_SENSOR_PISO1, INPUT_PULLUP);
  pinMode(PIN_SENSOR_PISO2, INPUT_PULLUP);
  pinMode(PIN_SENSOR_PISO3, INPUT_PULLUP);
  pinMode(PIN_SENSOR_OBSTRUCCION, INPUT_PULLUP);

  pinMode(PIN_LED_PISO1, OUTPUT);
  pinMode(PIN_LED_PISO2, OUTPUT);
  pinMode(PIN_LED_PISO3, OUTPUT);

  pinMode(PIN_BOTON_PISO1, INPUT_PULLUP);
  pinMode(PIN_BOTON_PISO2, INPUT_PULLUP);
  pinMode(PIN_BOTON_PISO3, INPUT_PULLUP);

  servoPuerta.attach(PIN_SERVO_PUERTA);
  servoPuerta.write(0); // Puerta cerrada
}

void loop() {
  actualizarLedsParpadeo();
  manejarPuertas();

  // Lógica de botones
  if (digitalRead(PIN_BOTON_PISO1) == LOW) moverAscensor(piso1);
  if (digitalRead(PIN_BOTON_PISO2) == LOW) moverAscensor(piso2);
  if (digitalRead(PIN_BOTON_PISO3) == LOW) moverAscensor(piso3);
}

void moverAscensor(Piso pisoDestino) {
  if (estadoActualPiso == pisoDestino) return;

  int direccion = (estadoActualPiso < pisoDestino) ? 1 : -1;
  motor.setSpeed(300 * direccion);

  while (true) {
    motor.runSpeed();
    if ((pisoDestino == piso1 && digitalRead(PIN_SENSOR_PISO1) == LOW) ||
        (pisoDestino == piso2 && digitalRead(PIN_SENSOR_PISO2) == LOW) ||
        (pisoDestino == piso3 && digitalRead(PIN_SENSOR_PISO3) == LOW)) {
      motor.stop();
      estadoActualPiso = pisoDestino;
      break;
    }
  }
}

void manejarPuertas() {
  if (!puertaAbierta) {
    servoPuerta.write(90); // Abrir puerta
    puertaAbierta = true;
  } else if (digitalRead(PIN_SENSOR_OBSTRUCCION) == LOW) {
    servoPuerta.write(0); // Intentar cerrar puerta
    temporizadorPuerta.start();
  } else if (temporizadorPuerta.update()) {
    if (digitalRead(PIN_SENSOR_OBSTRUCCION) == LOW) {
      servoPuerta.write(90); // Reabrir puerta si hay obstrucción
    } else {
      puertaAbierta = false; // Puerta cerrada con éxito
    }
  }
}

void actualizarLedsParpadeo() {
  if (temporizadorParpadeo.update()) {
    bool estadoLed = digitalRead(PIN_LED_PISO1);
    digitalWrite(PIN_LED_PISO1, !estadoLed);
    digitalWrite(PIN_LED_PISO2, !estadoLed);
    digitalWrite(PIN_LED_PISO3, !estadoLed);
  }
}
