#include <Arduino.h>
#include <BluetoothSerial.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <macros.h>
#include <Motores.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 400000UL, 100000UL);
Motores motoresDaniel;
BluetoothSerial SerialBT;

String comando = "";
String ultimoComando = "";
int pwmIzq = 0;
int pwmDer = 0;

void actualizarPantalla(String accion) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  
  // Título
  display.println("ROBOT BLUETOOTH");
  display.println("----------------");
  
  // Estado actual
  display.print("Accion: ");
  display.println(accion);
  
  display.println();
  
  // PWM de motores
  display.print("PWM Izq: ");
  display.println(pwmIzq);
  display.print("PWM Der: ");
  display.println(pwmDer);
  
  display.println();
  
  // Último comando
  display.print("CMD: ");
  display.println(ultimoComando);
  
  display.display();
}

void setup() {
  Serial.begin(115200);
  
  // Inicializar pantalla OLED
  Serial.print("Iniciando display...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("Falla al inicializar SSD1306");
    while(true);
  }
  Serial.println("OK");
  
  // Pantalla de bienvenida
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("ROBOT");
  display.println("Daniel");
  display.setTextSize(1);
  display.println();
  display.println("Esperando BT...");
  display.display();
  delay(2000);
  
  // Inicializar Bluetooth
  SerialBT.begin("RobotDaniel");
  Serial.println("Bluetooth iniciado");
  
  // Inicializar motores
  motoresDaniel.init(PIN_PWMA, PIN_AIN2, PIN_AIN1, PIN_STBY, PIN_BIN1, PIN_BIN2, PIN_PWMB);
  motoresDaniel.activa();
  
  pinMode(PIN_BOTON, INPUT_PULLUP);
  
  // Mostrar comandos disponibles
  SerialBT.println("=== ROBOT LISTO ===");
  SerialBT.println("Comandos:");
  SerialBT.println("  v[pwmI] [pwmD]");
  SerialBT.println("  a[pwm] - Avanzar");
  SerialBT.println("  r[pwm] - Retroceder");
  SerialBT.println("  g[pwm] - Girar");
  SerialBT.println("  p      - Parar");
  
  actualizarPantalla("Listo");
}

void procesarComando(String cmd) {
  cmd.trim();
  
  if (cmd.length() == 0) return;
  
  char tipoComando = cmd.charAt(0);
  String parametros = cmd.substring(1);
  parametros.trim();
  
  ultimoComando = cmd;
  
  Serial.print("Comando: ");
  Serial.println(cmd);
  SerialBT.print("> ");
  SerialBT.println(cmd);
  
  switch(tipoComando) {
    case 'v':
    case 'V': {
      int espacio = parametros.indexOf(' ');
      if (espacio > 0) {
        pwmIzq = parametros.substring(0, espacio).toInt();
        pwmDer = parametros.substring(espacio + 1).toInt();
        motoresDaniel.fijaPWMs(pwmIzq, pwmDer);
        actualizarPantalla("Velocidades");
        SerialBT.printf("Izq=%d Der=%d\n", pwmIzq, pwmDer);
      } else {
        SerialBT.println("Error: v[pwmI] [pwmD]");
      }
      break;
    }
    
    case 'a':
    case 'A': {
      int pwm = parametros.toInt();
      pwmIzq = pwm;
      pwmDer = pwm;
      motoresDaniel.fijaPWMs(pwm, pwm);
      actualizarPantalla("Avanzando");
      SerialBT.printf("Avanzar PWM=%d\n", pwm);
      break;
    }
    
    case 'r':
    case 'R': {
      int pwm = parametros.toInt();
      pwmIzq = -pwm;
      pwmDer = -pwm;
      motoresDaniel.fijaPWMs(-pwm, -pwm);
      actualizarPantalla("Retrocediendo");
      SerialBT.printf("Retroceder PWM=%d\n", pwm);
      break;
    }
    
    case 'g':
    case 'G': {
      int pwm = parametros.toInt();
      pwmIzq = pwm;
      pwmDer = -pwm;
      motoresDaniel.fijaPWMs(pwm, -pwm);
      actualizarPantalla("Girando");
      SerialBT.printf("Girar PWM=%d\n", pwm);
      break;
    }
    
    case 'p':
    case 'P': {
      pwmIzq = 0;
      pwmDer = 0;
      motoresDaniel.fijaPWMs(0, 0);
      actualizarPantalla("Detenido");
      SerialBT.println("Parado");
      break;
    }
    
    default:
      SerialBT.println("Comando no valido");
      actualizarPantalla("Error comando");
      break;
  }
}

void loop() {
  // Leer comandos desde Bluetooth
  while (SerialBT.available()) {
    char c = SerialBT.read();
    
    if (c == '\n' || c == '\r') {
      if (comando.length() > 0) {
        procesarComando(comando);
        comando = "";
      }
    } else {
      comando += c;
    }
  }
  
  // Botón de emergencia
  if (digitalRead(PIN_BOTON) == LOW) {
    pwmIzq = 0;
    pwmDer = 0;
    motoresDaniel.fijaPWMs(0, 0);
    actualizarPantalla("EMERGENCIA");
    SerialBT.println("PARADA EMERGENCIA");
    Serial.println("PARADA EMERGENCIA");
    delay(500);
  }
}