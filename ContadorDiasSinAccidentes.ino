#include <PCF85063TP.h>
#include <Wire.h>
#include <Preferences.h>

PCD85063TP RTclock;
Preferences prefs;

//Definición de pines
const int Boton1 = 18;
const int Boton2 = 5;
const int Boton3 = 17;
const int Boton4 = 16;
const int Boton5 = 36;

//Pines tarjeta LED
int dataPin = 26;  
int clockPin = 0;
int latPin = 32;  

bool ajusteFecha = false;

int minActual = 0;
int horaActual = 12;
int diaMes = 1;
int diaSemana = 1;
int mesActual = 1;
int anoActual = 26;

int diasSinAccidentes = 0;
int ultimoDiaVisto = 0;

bool estadoAnteriorBoton1 = HIGH;
bool estadoAnteriorBoton2 = HIGH;
bool estadoAnteriorBoton3 = HIGH;
bool estadoAnteriorBoton4 = HIGH;
bool estadoAnteriorBoton5 = HIGH;

unsigned long estadoInicioBoton2 = 0;
unsigned long estadoInicioBoton3 = 0;
unsigned long estadoInicioBoton4 = 0;

unsigned long ultimoParpadeo = 0;
bool parpadeoVisible = true;

unsigned long debounce = 0;
unsigned long primerPulso = 0;            

bool botonBloqueado = false;

int seleccionCampo = 0;

//Modo test
bool modoTest = false;
int numeroTest = 0;
unsigned long ultimoCambioTest = 0;

// LED equipo funcionando
const int ledEstado = 13;
unsigned long ultimoCambioLed = 0;
bool ledVisible = true;


void setup() {
  Wire.begin();
  RTclock.begin();

  prefs.begin("cartel", false);
  diasSinAccidentes = prefs.getInt("dias", 0);
 
  pinMode(Boton1, INPUT_PULLUP);
  pinMode(Boton2, INPUT_PULLUP);
  pinMode(Boton3, INPUT_PULLUP);
  pinMode(Boton4, INPUT_PULLUP);
  pinMode(Boton5, INPUT);

  //Configuración de pines como salidas
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latPin, OUTPUT);
  pinMode(ledEstado, OUTPUT);

  digitalWrite(dataPin, LOW);
  digitalWrite(clockPin, LOW);
  digitalWrite(latPin, LOW);
}

void mostrarContador(int valor) {
  int unidad = valor % 10;
  int decena = (valor / 10) % 10;
  int centena = (valor / 100) % 10;
  int millar = (valor / 1000) % 10;

  if (decena == 0 && valor < 10) {
    decena = 10;
  }
  if (centena == 0 && valor < 100) {
    centena = 10;
  }
  if (millar == 0 && valor < 1000) {
    millar = 10;
  }

  enviarDigito(unidad);
  enviarDigito(decena);
  enviarDigito(centena);
  enviarDigito(millar);

  digitalWrite(latPin, HIGH);
  digitalWrite(latPin, LOW);
}

void mostrarNumeroTest(int valor) {
  enviarDigito(valor);
  enviarDigito(valor);
  enviarDigito(valor);
  enviarDigito(valor);

  digitalWrite(latPin, HIGH);
  digitalWrite(latPin, LOW);
}

void mostrarFecha() {
  int diaSemanaSem = diaSemana;
  int horaUnidad = horaActual % 10;
  int horaDecena = horaActual / 10;
  int minUnidad = minActual % 10;
  int minDecena = minActual / 10;
  int diaUnidad = diaMes % 10;
  int diaDecena = diaMes / 10;
  int mesUnidad = mesActual % 10;
  int mesDecena = mesActual / 10;
  int anoUnidad = anoActual % 10;
  int anoDecena = anoActual / 10;

  switch (seleccionCampo) {
    case 0: // Hora parpadea, Minuto fijo
      if (horaDecena == 0) {
      horaDecena = 10;
      }
      if (parpadeoVisible == false) {
      horaUnidad = 10;
      horaDecena = 10;
      }
      enviarDigito(minUnidad);
      enviarDigito(minDecena);
      enviarDigito(horaUnidad);
      enviarDigito(horaDecena);
      break;

    case 6: // Hora fija (sin parpadeo), antes de salir
      if (horaDecena == 0) {
        horaDecena = 10;
      }
      enviarDigito(minUnidad);
      enviarDigito(minDecena);
      enviarDigito(horaUnidad);
      enviarDigito(horaDecena);
      break;
   
    case 1: // Minuto parpadea, Hora fijo
      if (parpadeoVisible == false) {
        minUnidad = 10;
        minDecena = 10;
      }
      enviarDigito(minUnidad);
      enviarDigito(minDecena);
      enviarDigito(horaUnidad);
      enviarDigito(horaDecena);
      break;

    case 2: // Día de semana parpadea, Fecha fija
      if (parpadeoVisible == false) {
        diaSemanaSem = 10;
      }
      enviarDigito(diaUnidad);
      enviarDigito(diaDecena);
      enviarDigito(diaSemanaSem);
      enviarDigito(10);
      break;

    case 3: // Fecha parpadea, Día de semana fijo
      if (diaDecena == 0) {
        diaDecena = 10;
      }
      if (parpadeoVisible == false) {
        diaUnidad = 10;
        diaDecena = 10;
      }
      enviarDigito(diaUnidad);
      enviarDigito(diaDecena);
      enviarDigito(diaSemana);
      enviarDigito(10);
      break;

    case 4: // Mes parpadea, Año fijo
      if (mesDecena == 0) {
        mesDecena = 10;
      }
      if (parpadeoVisible == false) {
        mesUnidad = 10;
        mesDecena = 10;
      }
      enviarDigito(anoUnidad);
      enviarDigito(anoDecena);
      enviarDigito(mesUnidad);
      enviarDigito(mesDecena);
      break;

    case 5: // Año parpadea, Mes fijo
      if (parpadeoVisible == false) {
        anoUnidad = 10;
        anoDecena = 10;
      }
      enviarDigito(anoUnidad);
      enviarDigito(anoDecena);
      enviarDigito(mesUnidad);
      enviarDigito(mesDecena);
      break;
  }

  digitalWrite(latPin, HIGH);
  digitalWrite(latPin, LOW);
}

void enviarDigito(int dato) {
  byte datoSalida;

  switch (dato) {
    case 10:
      datoSalida = 0x00;
      break;
    case 0:
      datoSalida = 0x3F;
      break;
    case 1:
      datoSalida = 0x06;
      break;
    case 2:
      datoSalida = 0x5B;
      break;
    case 3:
      datoSalida = 0x4F;
      break;
    case 4:
      datoSalida = 0x66;
      break;
    case 5:
      datoSalida = 0x6D;
      break;
    case 6:
      datoSalida = 0xFD;
      break;
    case 7:
      datoSalida = 0x07;
      break;
    case 8:
      datoSalida = 0x7F;
      break;
    case 9:
      datoSalida = 0x6F;
      break;
    default:
      datoSalida = 0x00;
      break;
  }

  shiftOut(dataPin, clockPin, MSBFIRST, datoSalida);
}


void loop() {
  bool estadoBoton1 = digitalRead(Boton1);
  bool estadoBoton2 = digitalRead(Boton2);
  bool estadoBoton3 = digitalRead(Boton3);
  bool estadoBoton4 = digitalRead(Boton4);
  bool estadoBoton5 = digitalRead(Boton5);

  RTclock.getTime();
  int diaDelRTC = RTclock.dayOfMonth;
  if (ultimoDiaVisto == 0) {
    ultimoDiaVisto = diaDelRTC;
  } else if (diaDelRTC != ultimoDiaVisto) {
    diasSinAccidentes = diasSinAccidentes + 1;
    ultimoDiaVisto = diaDelRTC;
    prefs.putInt("dias", diasSinAccidentes);
  }

   diaMes = RTclock.dayOfMonth;
   diaSemana = RTclock.dayOfWeek + 1;
   mesActual = RTclock.month;
   anoActual = RTclock.year  % 100;
   horaActual = RTclock.hour;
   minActual = RTclock.minute;
 
  if (ajusteFecha == true && (millis() - ultimoParpadeo) >= 300) {
    ultimoParpadeo = millis();
    parpadeoVisible = !parpadeoVisible;
  }

  if (estadoBoton1 == LOW && estadoAnteriorBoton1 == HIGH && (millis() - debounce) > 150) {
    debounce = millis();
    if (ajusteFecha == false) {
      horaActual = 12;
      minActual = 0;
      seleccionCampo = 0;
    }
    ajusteFecha = true;
  }

  if (estadoBoton2 == LOW && estadoAnteriorBoton2 == HIGH && ajusteFecha == true && (millis() - debounce) > 150) {
    debounce = millis();
    if (seleccionCampo == 6) {
      ajusteFecha = false;
      diasSinAccidentes = 0;
      prefs.putInt("dias", diasSinAccidentes);
    } else {
      estadoInicioBoton2 = millis();
      seleccionCampo = seleccionCampo + 1;
    }
  }

  if (estadoBoton3 == LOW && estadoAnteriorBoton3 == HIGH && ajusteFecha == true && (millis() - debounce) > 150) {
    debounce = millis();
    estadoInicioBoton3 = millis();

    switch (seleccionCampo) {
      case 0:
      case 6:
        horaActual = horaActual + 1;
        if (horaActual > 23) {
          horaActual = 0;
        }
        break;

      case 1:
        minActual = minActual + 1;
        if (minActual > 59) {
          minActual = 0;
        }
        break;

      case 2:
        diaSemana = diaSemana + 1;
        if (diaSemana > 7) {
          diaSemana = 1;
        }
        break;

      case 3:
        diaMes = diaMes + 1;
        if (diaMes > 31) {
          diaMes = 1;
        }
        break;

      case 4:
        mesActual = mesActual + 1;
        if (mesActual > 12) {
          mesActual = 1;
        }
        break;

      case 5:
        anoActual = (anoActual + 1) % 100;
      break;      
    }

    RTclock.stopClock();
    RTclock.fillByYMD(anoActual + 2000, mesActual, diaMes);
    RTclock.fillByHMS(horaActual, minActual, 0);
    RTclock.fillDayOfWeek(diaSemana - 1);
    RTclock.setTime();
    RTclock.startClock();
   
  }

  if (estadoBoton3 == LOW && estadoAnteriorBoton3 == HIGH && ajusteFecha == false && (millis() - debounce) > 150) {
    debounce = millis();
    estadoInicioBoton3 = millis();
  }

  if (estadoBoton3 == LOW && (millis() - estadoInicioBoton3) >= 5000 && ajusteFecha == false) {
    diasSinAccidentes = 0;
    prefs.putInt("dias", diasSinAccidentes);

    RTclock.stopClock();
    RTclock.fillByYMD(2020,5,1);//May 1,2020
    RTclock.fillByHMS(12,00,00);//12:28 30"
    RTclock.fillDayOfWeek(FRI);//Viernes
    RTclock.setTime();//write time to the RTC chip
    RTclock.startClock();
 
  }

  if (estadoBoton4 == LOW && estadoAnteriorBoton4 == HIGH && ajusteFecha == false && (millis() - debounce) > 200 && botonBloqueado == false) {
    debounce = millis();
    estadoInicioBoton4 = millis();
    diasSinAccidentes = diasSinAccidentes + 1;
    botonBloqueado = true;
    prefs.putInt("dias", diasSinAccidentes);
  }

  if (estadoBoton4 == HIGH) {
    botonBloqueado = false;
  }

  //Test
  if (estadoBoton5 == LOW && estadoAnteriorBoton5 == HIGH && modoTest == false && (millis() - debounce) > 150) {
    debounce = millis();
    modoTest = true;
    numeroTest = 0;
    ultimoCambioTest = millis();
  }

  // Logica del Modo Test
  if (modoTest == true) {
    if ((millis() - ultimoCambioTest) >= 3000) {
      ultimoCambioTest = millis();
      numeroTest = numeroTest + 1;
      if (numeroTest > 9) {
        modoTest = false;
      }
    }
  }

  estadoAnteriorBoton5 = estadoBoton5;

  if (modoTest == true) {
    mostrarNumeroTest(numeroTest);
    estadoAnteriorBoton1 = estadoBoton1;
    estadoAnteriorBoton2 = estadoBoton2;
    estadoAnteriorBoton3 = estadoBoton3;
    estadoAnteriorBoton4 = estadoBoton4;
    return;
  }

  // Logica cuando el equipo esta funcionando el led parpadea
  if ((millis() - ultimoCambioLed) >= 350) {
    ultimoCambioLed = millis();
    ledVisible = !ledVisible;
  }

  digitalWrite(ledEstado, ledVisible);
 
  if (ajusteFecha == true) {
    mostrarFecha();
  } else {
    mostrarContador(diasSinAccidentes);
  }
  estadoAnteriorBoton1 = estadoBoton1;
  estadoAnteriorBoton2 = estadoBoton2;
  estadoAnteriorBoton3 = estadoBoton3;
  estadoAnteriorBoton4 = estadoBoton4;
}
