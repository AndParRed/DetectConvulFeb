/***********************************************************************
 * FICHERO/FILE:     DetectorConvulsionesFeb.ino
 * AUTOR/AUTHOR:     Andres Pardo Redondo
 * FECHA_CREACION/CREATION_DATE:             2020-05-015
 *
 *    Copyright(c)   Andres Pardo Redondo
 *-----------------------------------------------------------------------
 * DESCRIPCION_FICHERO/FILE_DESCRIPTION:*/
/**@file DetectorConvulsionesFeb.ino
 * @brief Funciones de manejo de los diferentes modulos del detector de convulsiones febriles.
 *
 * */
/***********************************************************************/
/***********************************************************************
 * LITERALES, MACROS TIPOS DE DATOS Y FUNCIONES IMPORTADAS               
 * IMPORTED LITERALS, MACROS, DATA TYPES AND FUNCTIONS                   
 ***********************************************************************/
#include <Wire.h>
#include "BluetoothSerial.h"
#define DATAX0 0x5B // Direccion hexadecimal de DATAX0 del registro interno.

/***********************************************************************
 * DECLARACION DE TIPOS DE DATOS, CONSTANTES Y VARIABLES INTERNOS       
 * DECLARATION OF INTERNAL DATA TYPES, CONSTANTS AND VARS               
 ***********************************************************************/
BluetoothSerial SerialBT;
int RAddress = 0x27; //Direccion del dispositivo que incluye el octavo bit de seleccion modo read.
// Tiempos--------------------------------
int T; //Tiempo del sistema
int Ttemp = 0; //Tiempo de la temperatura
int Tv = 0; //Tiempo de la tension
int Tav = 0; //Tiempo del altavoz
int Tled = 0; //Tiempo del LED
int TOFF1 = 950; //Tiempo LED apagado con Frq.1   
int TOFF2 = 400; //Tiempo LED apagado con Frq.2   
int TON1 = 50; //Tiempo LED encendido con Frq.1  
int TON2 = 100; //Tiempo LED encendido con Frq.2  
int Tmen = 0; // Tiempo de envio del mensaje
//-----------------------------------------

//Alertas----------------------------------
int ALTEN = 0; // Alerta por tension
int ALTEMP = 0; // Alerta por temperatura
int AVACT = 0; // Alerta de altavoz activo
int ALSENSOR = 0; // Alerta de fallo del sensor de temperatura
//-----------------------------------------

//Datos-------------------------------------
float TEMP; // Valor medio de la temperatura
float TEN; // Valor de la tension
int TENres; // Valor que representa el estado de la bateria
int LFRQ = 1; // Frecuencia del altavoz
//------------------------------------------

//Ventana de promedio-----------------------
float ventana[5]; // Array que contiene los 5 ultimos datos de temperatura
float maximo; // Variable que lleva el valor mas alto de temperatura que hay en ventana
float minimo; // Variable que lleva el valor mas bajo de temperatura que hay en ventana
float acumulado; // Variable que lleva la suma de todos los valores del array ventana
boolean llenoVentana = false; // Variable que indica si el array de ventana esta vacio
//------------------------------------------

//Comunicacion------------------------------
int COMUNICA = 0; //Comprueba si esta activa la comunicacion
String Mensaje; // Guarda el contenido del mensaje recibido
//------------------------------------------

//Constantes--------------------------------
float UMBRAL_T = 37.0; // Umbral de temperatura
float UMBRAL_T1 = 35.0;// Umbral fallo sensor
float UMBRAL_V = 3.40; // Umbral de tension
//------------------------------------------

//LED------------------------------------------
enum COLOR {VERDE,AZUL,AMAR,ROJO};
COLOR LCOLOR; //Color del LED
int LON = 0; // LED encendido
int LEDR = 0; // LED rojo encendido
int LEDV = 0; // LED verde encendido
int LEDA = 0; // LED amarillo encendido
uint8_t ledR = 15; //PIN LED ROJO
uint8_t ledG = 16; //PIN LED VERDE
uint8_t ledB = 17; //PIN LED AZUL
//---------------------------------------------

//Altavoz-------------------------------------
boolean TONO = false; // Indica si el altavoz esta sonando
//-------------------------------------------- 

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de arduino para inicializar variables pinenes etc.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void setup() 
{
  SerialBT.begin("DConvFeb"); //Pone como nombre del bluetooth DConvFeb
//------------------------------------------------
  Wire.begin(22,21); // SDA, SCL
//------------------------------------------------
  Serial.begin(115200);
  
  ledcAttachPin(12, 0); // Pin del altavoz
  ledcSetup(0, 0, 8); //Configuracion altavoz 
  
  ledcAttachPin(ledR, 1); // Pin del LED rojo
  ledcAttachPin(ledG, 2); // Pin del LED verde
  ledcAttachPin(ledB, 3); // Pin del LED azul

  ledcSetup(1, 12000, 8); // Configuracion del LED rojo
  ledcSetup(2, 12000, 8); // Configuracion del LED verde
  ledcSetup(3, 12000, 8); // Configuracion del LED azul

  delay(100);
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de arduino de ejecucion del codigo.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void loop() 
{
  T = millis(); // Tiempo del sistema
  
  Temperatura();
  Tension();
  Conexion();
  Altavoz();
  Alarmas();
  LED();

}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que controla el tiempo de toma de dato.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Temperatura() // FUNCION DE TIEMPOS DE TOMA DE TEMPERATURA
{
   if (T > Ttemp) // Controla el tiempo de acceso al dato de tension y alerta de tension
   {
      sensorTemperatura();
      
      if( ALTEN == 1 ) //si la bateria esta baja triplica el tiempo de captura de temperatura
        Ttemp = T +15000;
      else
        Ttemp = T + 5000;
   }
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que alerta de temperatura etc.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void sensorTemperatura() // FUNCION DE ALERTA DE TEMPERATURA
{
  preguntarTemperatura();
  
  ALSENSOR = 0; //Alerta por fallo de sensor o lectura de temperatura por debajo de la normal humana
  ALTEMP = 0; //Alerta de temperatura por fiebre
  if(TEMP > UMBRAL_T)
    ALTEMP = 1;
  if(TEMP < UMBRAL_T1)
    ALSENSOR = 1;
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de toma de datos de temperatura
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void preguntarTemperatura() // FUNCION DE TOMA DE DATOS DE TEMPERATURA
{
  unsigned long temp;
  byte c;
  float dato;
  char tmp[20];
  
  Wire.beginTransmission(DATAX0); //Envia el comando de lectura
  Wire.write(RAddress);  // Escribe la direccion del dispositivo con el comando read
  Wire.endTransmission(false);
  
  Wire.requestFrom(DATAX0,3); // solicita los 2 bytes de datos y el byte de control de errores(este ultimo se descartara)

  c = Wire.read();// Lee datos del registro. Recibe un byte como caracter.
  temp = Wire.read();
  temp = (temp<<8)|c; //Desplaza 8 bits el dato y lo concatena con el caracter leido
  temp = (temp*2) -27315; // realiza la conversion del dato para representarlo en temperatura
  TEMP = temp;
  TEMP = TEMP/100; //se obtiene la temperatura con decimales
  
  if(!llenoVentana) //comprueba si la variable que comprueba el estado del array de ventana esta con valor falso
  {
    for(int i = 0; i < 5; i++) //llena el array ventana con el dato de temperatura
    {
      ventana[i]=TEMP;
      llenoVentana=true; //pone con valor true la variable que representa el estado de ventana
    }
  }
  else
    TEMP = vAcumulado(TEMP); //llama a la funcion que va a hacer el promediado de los datos de temperatura
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de ventana de promedio de los datos de temperatura
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : val => Valor de temperatura a promediar.
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return :  Retorna el dato de temperatura promediado.
 ***********************************************************************/
float vAcumulado(float val) // FUNCION DE VENTANA DE PROMEDIO DE LOS DATOS DE TEMPERATURA
{
  
  memmove(&ventana[1],&ventana[0],4*sizeof(float)); //desplaza el valor del array una posicion a la derecha
  ventana[0] = val;
  maximo = 0; //Lleva la cuenta del valor mas alto de temperatura que hay en ventana
  minimo =100.0; //Lleva la cuenta del valor mas bajo de temperatura que hay en ventana
  acumulado = 0; //Lleva la media de las temperaturas de ventana
  
  for(int i = 0; i < 5; i++) //Encuentra el valor mas alto y mas bajo de temperatura en la ventana
  {
      if(ventana[i]>maximo)
        maximo = ventana[i];
        
      if(ventana[i]<minimo)
        minimo = ventana[i];

      acumulado += ventana[i];   
  }

  return (acumulado - maximo - minimo)/3; //retorna el valor de promedio de la ventana
  
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description: Funcion que controla el tiempo de toma de la tension de bateria.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Tension() // FUNCION DE TIEMPOS DE TOMA DE TENSION DE BATERIA
{
  if (T > Tv) // Controla el tiempo de acceso al dato de tension y alerta de tension
   {
      sensorTension();
      Tv = T + 60000;
   }
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de alerta por tension de bateria.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void sensorTension() // FUNCION DE ALERTA DE TENSION DE BATERIA
{
  preguntarTension();

  if(TEN < UMBRAL_V) //Activa o desactiva la alerta por tension sengun el valor obtenido de tension
    ALTEN = 1;
  else
    ALTEN = 0;
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de toma de datos de tension de bateria.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void preguntarTension() // FUNCION DE TOMA DE DATOS DE TENSION DE BATERIA
{
  TEN = analogRead(33); 
  TEN = (TEN*1.100*5.545)/4096; //ajusta para los 5V que entran 

  if(TEN < 3.5) //bateria < 15%
    TENres = 1;
  else if(TEN < 3.8) //bateria < 50%
    TENres = 2;
  else            //bateria > 50%
    TENres = 3;
    
  Serial.print("Ten=>");
  Serial.println(TEN);
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que comprueba si hay conexion con dispositivo 
 *  android y controla el tiempo de envio del mensaje.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Conexion() // FUNCION QUE COMPRUEBA SI HAY CONEXION Y EL TIEMPO DE ENVIO DEL MENSAJE
{
  if(T > Tmen) // Controla el tiempo de envio del mensaje
  {
    
    if(SerialBT.hasClient() == true) //comprueba si hay un dispositivo conectado
    {
      COMUNICA = 1;
      
      Conexion_BLE();
    }
    else
      COMUNICA = 0;
    Tmen = T + 15000;
  }
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que genera y envia el mensaje por bluetooth 
 *  con el contenido de dato de temperatura, estado de bateria y alertas.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Conexion_BLE() //FUNCION QUE EMPAQUETA LOS DATOS DE TEMP, ESTADO BAT y ALERTAS Y ENVIA MENSAJE POR BT
{
  char tmp[20];

    sprintf(tmp,"%03d%01d%01d%01d",(int)(TEMP*10),TENres,ALTEMP,ALTEN); // Encapsulado del mensaje a enviar
    SerialBT.print(tmp);
    
    sprintf(tmp,"%03d %01d %01d %01d",(int)(TEMP*10),TENres,ALTEMP,ALTEN); // comprobacion por pantalla del dato a enviar
    Serial.println(tmp);
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion de control del funcionamiento del altavoz.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Altavoz()// FUNCION DE CONTROL DEL ALTAVOZ
{
  if(AVACT == 1) // Pregunta si esta activado el altavoz
  {
    if( T > Tav) // Controla el tiempo de ejecucion del altavoz
    {
      if(TONO == true) // pregunta si el tono del altavoz esta activado
      {
        TONO = false; // Apaga el tono para la proxima ejecucion de la funcion
        noTono();
        Tav = T + 400;
      }
      else
      {
        TONO = true; // Activa el tono para la proxima ejecucion de la funcion
        Tono();
    //    delay(250);
    //    noTono();
        Tav = T + 250;
      }
    }    
  }
  else // Si la variable de altavoz activo esta desactivado valor 0
  {
    if(TONO == true) // Si el altavoz esta sonando
    {
      TONO = false; // Desactiva el tono del altavoz
      noTono();
      Tav = 0;
    }
  }
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que enciende el altavoz.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Tono() // FUNCION QUE ENCIENDE EL ALTAVOZ
{
  ledcWriteTone (0,2000);
  ledcWrite(0,25);
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que apaga el altavoz.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void noTono() // FUNCION QUE APAGA EL ALTAVOZ
{
  ledcWriteTone(0,0);
  ledcWrite(0,0);
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que lleva el control de las alarmas.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void Alarmas() //FUNCION QUE LLEVA EL CONTROL DE LAS ALARMAS
{
  LFRQ = 1; // Frecuencia de parpadeo por defecto del led
  LCOLOR = VERDE; // Pone por defecto el color ver a la variable del led
  AVACT = 0;

  if(COMUNICA == 1) // Comprueba si el modulo esta conectado por bluetooth
    LCOLOR = AZUL; // Pone el color azul a la variable de color del led
  if(ALTEN == 1) // Comprueba si hay alerta por tension activa
    LCOLOR = AMAR; // Pone el color amarillo a la variable de color del led
  if(ALSENSOR == 1) // Comprueba si hay alerta por sensor no funcionando/ temperatura por debajo de la normal humana
  {
      LCOLOR = ROJO; // Pone el color rojo a la variable de color del led
      AVACT = 1;
  }
  if(ALTEMP == 1) // Comprueba si hay alerta por temperatura alta
  {
    LCOLOR = ROJO; // Pone el color rojo a la variable de color del led
    AVACT = 1;
    LFRQ = 2; // Frecuencia de parpadeo del led
  }
}

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:                                             
 */
/** \b Description:  Funcion que lleva el control del encendido del led.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param : void
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
void LED() //FUNCION QUE LLEVA EL CONTROL DEL ENCENDIDO DEL LED
{
  if(T > Tled) // Controla el tiempo de activacion del led
  { 
    if(LON == 1) // Pregunta si el led esta encendido
    {
      LEDR = 0; 
      LEDV = 0; 
      LEDA = 0; 

      ledcWrite(1, LEDR); 
      ledcWrite(2, LEDV); 
      ledcWrite(3, LEDA); 
      
      LON = 0; // Desactiva el led

      if(LFRQ == 1) // cambia el tiempo de apagado segun la frecuencia
        Tled = T + TOFF1;
      else
        Tled = T + TOFF2;
    }
    else
    {
      LEDR = 0;
      LEDV = 0;
      LEDA = 0;
      
      if(LCOLOR == ROJO) // Si se decta el color rojo y se activa el led para que ilumene con dicho color
      {
        LEDR = 255;
        ledcWrite(1, LEDR);
      }
      else if(LCOLOR == VERDE) // Si se decta el color verde y se activa el led para que ilumene con dicho color
      {
        LEDV = 255;
        ledcWrite(2, LEDV);
      }
      else if(LCOLOR == AZUL) // Si se decta el color azul y se activa el led para que ilumene con dicho color
      {
        LEDA = 255;
        ledcWrite(3, LEDA);
      }
      else if(LCOLOR == AMAR) // Si se decta color amarillo activa el led para que ilumene con dicho color
      {
        LEDR = 255;
        LEDV = 40;
        ledcWrite(1, LEDR);
        ledcWrite(2, LEDV);
      }
      
      LON = 1; // Se activa el led
      if(LFRQ == 1) // cambia el tiempo de encendido segun la frecuencia
        Tled = T + TON1;
      else
        Tled = T+ TON2;
 //     delay(50);
      ledcWrite(1, 0);
      ledcWrite(2, 0);
      ledcWrite(3, 0);
    } 
  }
}
