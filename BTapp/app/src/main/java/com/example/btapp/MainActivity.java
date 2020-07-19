/***********************************************************************
* FICHERO/FILE:     MainActivity.java
* AUTOR/AUTHOR:     Andres Pardo Redondo
* FECHA_CREACION/CREATION_DATE:             2020-06-02
*
*    Copyright(c)   Andres Pardo Redondo
*-----------------------------------------------------------------------
* DESCRIPCION_FICHERO/FILE_DESCRIPTION:*/
/**@file MainActivity.java
* @brief Establece conexion bluetooth con el detector de convulsiones febriles,
* presenta en la interfaz grafica datos de temperatura y estado de bateria obtenidos de detector y
* activa una alarma de vibracion, si la temperatura supera los 37 grados
* y/o el estado de la bateria del detector esta con carga baja
*
* */
/***********************************************************************/

/***********************************************************************
* LITERALES, MACROS TIPOS DE DATOS Y FUNCIONES IMPORTADAS
* IMPORTED LITERALS, MACROS, DATA TYPES AND FUNCTIONS
***********************************************************************/
package com.example.btapp;
import androidx.appcompat.app.AppCompatActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Vibrator;
import java.io.IOException;
import java.io.InputStream;
import java.util.Set;
import java.util.UUID;
import static java.lang.Thread.sleep;

public class MainActivity extends AppCompatActivity {

/***********************************************************************
* DECLARACION DE TIPOS DE DATOS, CONSTANTES Y VARIABLES INTERNOS
* DECLARATION OF INTERNAL DATA TYPES, CONSTANTS AND VARS
***********************************************************************/

    //Variables del Bluetooth
    private final String DEVICE_NAME="DConvFeb";
    private final UUID PORT_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");//Serial Port Service ID
    private BluetoothDevice device;
    private BluetoothSocket socket;
    //Variables de para tratamiento de datos
    private InputStream inputStream;
    byte buffer[];
    String string;
    int datoTemp = 0;
    int datoV = 0;
    int At =0;
    int Av =0;
    //Variables de figuras graficas
    Button adv;
    TextView textView,data;
    ImageView imageView,imageView2;
    //Variables de tiempo
    long Tdato = 0;
    long T=0;

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description:  Asigna espacio de memoria al iniciar la APP.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param savedInstanceState:
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textView = (TextView) findViewById(R.id.textView); //instancia variable textView
        data = (TextView) findViewById(R.id.data); //instancia variable data
        imageView = (ImageView)findViewById(R.id.imageView); //instancia variable imageView
        imageView2 = (ImageView)findViewById(R.id.imageView2); //instancia variable imageView2

        MiTarea tarea = new MiTarea(); //Crea objeto de tipo miTarea que ereda las funciones de una tarea asincrona
        tarea.execute(); //Ejecuta la tarea asincrona
    }

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description: Clase que heredafunciones de una tarea asincrona.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param <Integer,Integer, Integer>
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : no tiene
 ***********************************************************************/

    class MiTarea extends AsyncTask <Integer,Integer, Integer> //tarea asincrona
    {

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description:  Funcion de tarea asincrona que ejecuta la parte operacional del codigo.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param par: no utilizado
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : Integer
 ***********************************************************************/

        @Override
        protected Integer doInBackground(Integer... par)
        {
            boolean found;
            boolean connected;
            float convert;
            buffer = new byte[1024];
            byte[] rawBytes = new byte[1024];
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter(); //Recoge el estado del adaptador Bluetooth

            if (!bluetoothAdapter.isEnabled()) // Comprueba si esta activado el Bluetooth
            {
                Intent enableAdapter = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE); //manda una peticion para activar el bluetooth
                startActivityForResult(enableAdapter, 0);
                try {
                    sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            while (true) // Bucle para repetir proceso de conexion despues de perderla
            {
                while (true) //Bucle para verificar si el dispositivo esta emparejado
                {
                    found = false;

                    //if (bluetoothAdapter == null)
                       // publishProgress((int) 1);

                    Set<BluetoothDevice> bondedDevices = bluetoothAdapter.getBondedDevices(); //Obtiene la lista de dispositivos emparejados
                    if (!bondedDevices.isEmpty()) {
                        for (BluetoothDevice iterator : bondedDevices) {
                            if (iterator.getName().trim().equals(DEVICE_NAME.trim()))  //Comprueba si el modulo empotrado se encuentra en la lista de dispositivos emparejados
                            {
                                device = iterator;
                                found = true;
                            }
                        }
                    }

                    if (found)
                        break;
                }

                while (true) //Bucle para establecimiento del socket y canal de comunicacion
                {
                    publishProgress((int) 2); //Llama a una funcion para ejecutar elementos graficos en otro hilo
                    connected = true;
                    try {
                        socket = device.createRfcommSocketToServiceRecord(PORT_UUID); //Crea un socket Rfcomm
                        socket.connect(); // Conecta el socket
                    } catch (IOException e) {
                        e.printStackTrace();
                        connected = false;
                    }
                    if (connected) {
                        try {
                            inputStream = socket.getInputStream(); //abre un canal para recibir datos a traves del socket
                            break;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }

                while (true) // Buclede recepcion de datos
                {
                    try {
                        T = System.currentTimeMillis(); //Obtine el tiempo del sistema en milisegundos
                        if (inputStream.available() != -1) //Comprueba sin bloquear el numero de caracteres
                        {
                            Tdato = T + 10000; //lleva el tiempo del dato recibido
                            inputStream.read(rawBytes); //obtiene los datos del modulo empotrado
                            string = new String(rawBytes, "UTF-8");

                            datoTemp = Integer.parseInt(string.substring(0, 3).trim()); //Recoge la parte de temperatura del mensaje recibido
                            datoV = Integer.parseInt(string.substring(3, 4).trim()); //Recoge el estado de la bateria
                            At = Integer.parseInt(string.substring(4, 5).trim()); //Recoge el estado de alarma por temperatura
                            Av = Integer.parseInt(string.substring(5, 6).trim()); //Recoge el estado de alarma por tension

                            publishProgress((int) 3);
                        }
                        if ((datoTemp >= 370 && At == 1) || (datoV == 3 && Av == 1)) // Comprueba el estado de las alarmas recibidas
                        {
                            Vibrator vibrator = (Vibrator) getApplicationContext().getSystemService(Context.VIBRATOR_SERVICE); //Instancia vibrador del movil
                            vibrator.vibrate(5000); //Activa vibrador del movil
                            //sonar
                        }
                        if (T > Tdato) //Comprueba si el tiempo del sistema es mayor que el tiempo del dato recibido
                            break; //Sale del bucle

                    } catch (IOException ex) {
                        ex.printStackTrace();
                        break;
                    }
                }
                try {
                    inputStream.close(); //Cierra canal de recepcion de mensaje
                    socket.close(); //Cierra socket
                } catch (IOException ex) {
                    ex.printStackTrace();
                }

                if (!bluetoothAdapter.isEnabled()) //Comprueba si se ha desactivado el bluetooth
                    onDestroy(); //Cierra el programa
            }
        }

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description:  Funcion de la tarea asincrona que controla la parte grafica.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param prog: Indica que situar en la interfaz grafica
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/
        @Override
        protected void onProgressUpdate(Integer... prog)
        {
            switch (prog[0]) //switch case que se ejecuta dependiendo del valor recibido en la funcion doInBackground
            {
                case 1:
                    textView.setText("");
                    data.setText("");
                    Toast.makeText(getApplicationContext(), "El dispositivo movil no es compatible", Toast.LENGTH_LONG);
                    break;
                case 2:
                    textView.setText("");
                    data.setText("");
                    textView.setText("\nConectando...\n");
                    break;
                case 3:
                    textView.setText("");
                    mostrarTermometro(datoTemp); //Funcion que controla el valor que muestra el termometro
                    mostrarBateria(datoV); //Funcion que controla el valor que muestra el estado de la bateria
                    data.setText(string.substring(0, 2).trim() + "." + string.substring(2, 3).trim() + "C"); //Muestra en el texto debajo de la imagen del termometro la temperatura
                    break;
                default:
                    imageView.setImageResource(R.drawable.ter_error);
                    imageView2.setImageResource(R.drawable.b_init);
                    break;
            }
        }

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description:  Funcion que controla el valor mostrado en la imagen del termometro.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param i: Indica que imagen se ha de escoger
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/

        public void mostrarTermometro(int i)
        {
            switch (i) {
                case 356:
                case 357:
                    imageView.setImageResource(R.drawable.ter_356);
                    break;
                case 358:
                case 359:
                    imageView.setImageResource(R.drawable.ter_358);
                    break;
                case 360:
                case 361:
                    imageView.setImageResource(R.drawable.ter_360);
                    break;
                case 362:
                case 363:
                    imageView.setImageResource(R.drawable.ter_362);
                    break;
                case 364:
                case 365:
                    imageView.setImageResource(R.drawable.ter_364);
                    break;
                case 366:
                case 367:
                    imageView.setImageResource(R.drawable.ter_366);
                    break;
                case 368:
                case 369:
                    imageView.setImageResource(R.drawable.ter_368);
                    break;
                case 370:
                case 371:
                    imageView.setImageResource(R.drawable.ter_370);
                    break;
                case 372:
                case 373:
                    imageView.setImageResource(R.drawable.ter_372);
                    break;
                case 374:
                case 375:
                    imageView.setImageResource(R.drawable.ter_374);
                    break;
                case 376:
                case 377:
                    imageView.setImageResource(R.drawable.ter_376);
                    break;
                case 378:
                case 379:
                    imageView.setImageResource(R.drawable.ter_378);
                    break;
                case 380:
                case 381:
                    imageView.setImageResource(R.drawable.ter_380);
                    break;
                case 382:
                case 383:
                    imageView.setImageResource(R.drawable.ter_382);
                    break;
                case 384:
                case 385:
                    imageView.setImageResource(R.drawable.ter_384);
                    break;
                case 386:
                case 387:
                    imageView.setImageResource(R.drawable.ter_386);
                    break;
                case 388:
                case 389:
                    imageView.setImageResource(R.drawable.ter_388);
                    break;
                case 390:
                case 391:
                    imageView.setImageResource(R.drawable.ter_390);
                    break;
                case 392:
                case 393:
                    imageView.setImageResource(R.drawable.ter_392);
                    break;
                case 394:
                case 395:
                case 396:
                case 397:
                case 398:
                case 399:
                case 400:
                case 401:
                case 402:
                case 403:
                case 404:
                case 405:
                case 406:
                    imageView.setImageResource(R.drawable.ter_394);
                    break;
                default:
                    imageView.setImageResource(R.drawable.ter_amb);
                    break;
            }
        }

/***********************************************************************
 *  DESCRIPCION/DESCRIPTION:
 */
/** \b Description:  Funcion que controla el estado mostrado en la imagen de la bateria.
 *                                                              \if CERO
 *----------------------------------------------------------------------
 * PARAMETROS                                                   \endif
 *  \param i: Indica que imagen se ha de escoger
 *                                                              \if CERO
 * VALORES_DE_RETORNO/RETURN_VALUES:                            \endif
 *  \return : void
 ***********************************************************************/

        public void mostrarBateria(int i)
        {
            switch (i) {
                case 1:
                    imageView2.setImageResource(R.drawable.b_low);
                    break;
                case 2:
                    imageView2.setImageResource(R.drawable.b_mid);
                    break;
                case 3:
                    imageView2.setImageResource(R.drawable.b_ent);
                    break;
                default:
                    imageView2.setImageResource(R.drawable.b_init);
                    break;
            }
        }
    }
}