
# DomDom - C01CH

## ¿Qué es es el proyecto C01CH?

-----

Este proyecto nació con la intención de crear un controlador para pantallas de acuario. Después de revisar otros proyectos anteriores a este nos dimos cuenta que todos ellos o se quedaban incompletos o abarcaban mucho más que la pantalla. Por ello se decidió hacer un equipo lo más completo posible pero centrándonos únicamente en las necesidades para controlar una pantalla de acuario.

## Características principales

-----

* Equipo económico (coste inferior a 10€)
* Corriente de tensión hasta 24V.
* 1 canal dimeable por control de corriente.

   Esta característica significa que en vez de apagar y encender el led muy muy rápido como hacen otros controladores (control por PWM) nosotros controlamos el brillo del led mediante el control de corriente. Esto permite un brillo inferior pero teniendo el led encendido el 100% del tiempo.

   Por otra parte, esta característica permite ser el equipo compatible con el 100% de pantallas que trabajan con un voltaje igual o inferior a  24V.

* Monitorización del consumo y de la potencia.
* Ajuste manual de la potencia que se mantendrá incluso depués de apagar el equipo.
* Programación de potencia en función de la hora con hasta 50 puntos de programación.
* Servicio NTP para configurar la fecha, hora y ajuste de horario verano/invierno de forma automática.
* Control para un ventilador externo.
* Calculo en tiempo real del espectro de nuestra pantalla. (Siempre que configuremos el tipo de leds que estamos usando)
* Conexión mediante WIFI.
* Equipo actualizable de forma remota mediante WIFI.
* Servidor web integrado para configurar el equipo desde cualquier dispositivo, utilizando únicamente un navegador web.

-----

## Lista de materiales

-----

Pongo los links donde yo he comprado las cosas porque me ha parecido un buen vendedor pero cabe recalcar que no tengo ningún acuerdo con las tiendas que aparecen.

| Componente                  | Cantidad | Precio Total| link |
| --------------------------- | -------: | -----------:| ---- |
| ESP32 Devkit-C de 36 pines  | 1        |         3.66€| [link](https://es.aliexpress.com/item/32807887667.html?spm=a2g0s.9042311.0.0.1f6c63c0jkFGcQ) |
| Mini DC-DC                  | 1        |         0.43€| [link](https://es.aliexpress.com/item/32649396601.html?spm=a2g0s.9042311.0.0.1f6c63c0jkFGcQ) |
| MOSFET PWM 400W 15A         | 1        |         0.67€| [link](https://es.aliexpress.com/item/32803005422.html?spm=a2g0s.9042311.0.0.274263c0MQxfmC) |
| INA219                      | 1        |         1.08€| [link](https://es.aliexpress.com/item/33025643671.html?spm=a2g0s.9042311.0.0.1f6c63c0jkFGcQ) |
| XL4015                      | 1        |         0.91€| [link](https://es.aliexpress.com/item/4001118651312.html?spm=a2g0s.9042311.0.0.1f6c63c0jkFGcQ) |
| Resistencia 507Ohms         | 1        |        0.005€| [link](https://es.aliexpress.com/item/32953278667.html?spm=a2g0s.9042311.0.0.274263c0MQxfmC) |
| Cables varios               | 1        |       1  €   | [link](https://es.aliexpress.com/item/32965231797.html?spm=a219c.12010615.8148356.1.6921396csAn5Dh)|
| DC Power pin hembra         | 3        |         0.28€| [link](https://es.aliexpress.com/item/32973509798.html?spm=a2g0s.9042311.0.0.1f6c63c0jkFGcQ) |
| Caja 3D                     | 1        |            3€| [link](./STL%20Caja/XL4015) |
| Tornillo 12mm M3 y tuerca   | 1        |        0.20€ | |
-----

## Esquema de conexiones

-----

Una imagen vale más que mil palabras...

![alt text](https://raw.githubusercontent.com/fg89o/C01CH/master/Esquema/Esquema.jpg "Esquema")

-----

## Firmware

-----

### Requisitos

* [NodeJS v12.18.3](https://nodejs.org/es/)
* [npm v6.14.6](https://www.npmjs.com/get-npm)
* [Visual studio code](https://code.visualstudio.com/download)
* [Platform IO](https://platformio.org/)
* [Vetur VS Code plugin](https://marketplace.visualstudio.com/items?itemName=octref.vetur)
* [C/C++ VS Code plugin](https://code.visualstudio.com/docs/languages/cpp)

### Compilacion

1. Tener instalados todos los requisitos mencionados anteriormente.
2. Clonar el repositorio
3. Abrir la carpeta "CH01CH UI" con el VS Code
4. Ejecutar en la terminal:

   ```javascript
   npm install
   ```

5. A continuación ejecutar:

   ```javascript
   npm run build
   ```

6. Abrir la carpeta "CH01CH fw" con el VS Code
7. Conectar el ESP32 por USB al ordenador
8. Ejecutar la opción "Task -> env:esp32dev -> Platform -> Upload Filesystem Image" o por terminal

   ```javascript
   platformio.exe run --target uploadfs --environment esp32dev
   ```

9. A) (opcional) Opcionalmente podemos modificar el fichero configuration.h con los datos de nuestra wifi para que se conecte directamente

   ```c
   // Define la red wifi a la que intetara conectase por defecto
   #define WIFI_STA_SSID_NAME "NOMBRE_DEL_WIFI"

   // Define el password de la red wifi por defecto
   #define WIFI_STA_PASSWORD "CONTRASEÑA"
   ```

10. B) Ejecutar la opción "Task -> General -> Upload" o por terminal

   ```javascript
   platformio.exe run --target upload
   ```

11. Comprobar que todo funciona correctamente:

* Si hemos configurado nuestro wifi en el paso 9.A solamente tendremos que acceder a http://C01CH.local.
* En caso de que no lo hayamos configurado debemos conectarnos al wifi creado por el equipo, que el SSID y será C01CH y después acceder a http://C01CH.local

# Soporte

Todo el que necesite soporte lo puede hacer bien a través de las issue de github de este proyecto o, los que dispongan de acceso, a través de este post en el que también se está siguiendo el proyecto.

# Agradecimientos

Muchas gracias a toda la gente que ha participado en este proyecto, bien con ideas, críticas que ayudan a crecer o de cualquier otro modo. En especial mencionar a todo el foro de [AcuariofiliaMadrid](https://acuariofiliamadrid.org/index.php) por el soporte dado y porque gracias al gran foro que mantienen he podido aprender los conocimientos necesarios para desarrollar satisfactoriamente este proyecto.