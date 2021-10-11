# Bibliotecas SOP
BibliotecasSOP es un sistema de gestión bibliotecario para Solicitar, Renovar o Devolver libros. Se compone de un único Servidor al cual múltiples Clientes pueden conectarse y procesas solicitudes

## Uso

### Servidor
> Uso: ./server -p pipeServidor -f baseDeDatos

el flag -f se utiliza para específicar el archivo de texto donde se almacena la base de datos de todos los libros ([veáse Base de datos](#base-de-datos))

### Cliente
> Uso: ./client [-i Archivo] -p pipeServidor
> [-i archivo] es opcional!

Las peticiones pueden realizarse mediante un archivo de texto con el flag -i, si no se utiliza este flag se mostrará un menú ([veáse Archivo de peticiones](#archivo-de-peticiones))

Sólo se puedentener un único servidor pero múltiples clientes conectados al mismo

## Archivos de texto
### Base de datos
Este archivo será leído por el Servidor, un ejemplo puede encontrarse en (./archivo_prueba/BD.txt)

Formato:
> Nombre del Libro,ISBN,ejemplares
> NumeroDeEjemplar,[estado](#estado),fecha(dd/mm/yy)

##### Estado
Caractér que indica el estado
D: Disponible
P: Prestado
### Archivo de peticiones
Este archivo será leído por el Cliente y es opcional, si no se utiliza archivo el cliente mostrará un menú ([véase Cliente](#cliente)), un ejemplo de este archivo puede ser encontrado en (./archivo_prueba/PS.txt) y (./archivo_prueba/PS1.txt)

Formato:
> [peticion](#peticion),Nombre del libro,ISBN
##### Peticion
Caractér que indica el tipo de petición
P: Prestar
D: Devolver
R: Renovar

## ¿Cómo se envían información entre Cliente y Servidor?
### Pipes
La comunicación entre Clientes y Servidor se da mediante pipes nominales de la librería POSIX

* El Servidor tiene un único pipe de lectura mediante el cual todos los clientes envían sus peticiones (Cliente->Servidor), el nombre de este pipe se da mediante parámetros en la creación del Servidor
* Cada cliente tiene su propio pipe de lectura mediante el cual el Servidor envía información al Cliente (Servidor->Cliente) y el nombre se asigna de acuerdo al PID del proceso cliente "pipeCliente_PID 
  
Según lo anterior, existen múltiples pipes (Servidor->Cliente) pero sólo un pipe (Cliente->Servidor) y será el creador del pipe el encargado de borrarlo al finalizar ([véase Protocolo de comunicación](#protocolo-de-comunicación))

### Paquetes
Para evitar problemas en la escritura y lectura de información en el pipe, tanto Clientes como Servidor escriben y reciben datos de tipo <<i>data_t</i>>, esta estructura es lo único que se puede leer y escribir de los pipes y usualmente nos referimos a ella como 'paquete', este paquete contiene el PID del cliente quien manda la petición, un indicador del tipo de paquete ([véase Tipo de Paquete](#tipo-de-paquete)), y una unión a la información del paquete

##### Tipo de paquete
Existen tres tipos de paquetes que pueden ser enviados: [SIGNAL](#signal), [BOOK](#book) y [ERR](#err)

###### SIGNAL
Este tipo de paquete indica que se está enviando una señal (Usualmente el Servidor manda una señal al Cliente de que la operación fue exitosa o que el libro no existe)
(data_t.data.signal)

**Listado de señales:**
_Señales de peticiones:_
>#define PET_ERROR -3 // Error de petición
>#define SOLICITUD 3  // Solicitud exitosa
>#define RENOVACION 4 // Renovación exitosa
>#define DEVOLUCION 5 // Devolución exitosa

_Señales de confirmación de comunicación:_
> #define START_COM 1   // Señal para empezar comunicación
> #define STOP_COM -1   // Señal para detener confirmación
> #define SUCCEED_COM 2 // Señal de confirmación de comunicación
> #define FAILED_COM -2 // Señal de fallo en la comunicación (TERMINACION)

###### BOOK
Este tipo de paquete contiene la información de un libro, usualmente el Cliente envía este tipo de paquete al Servidor para solicitar, renovar o devolver un libro, (data_t.data.libro)

Cada libro tiene un tipo de petición: SOLICITAR, RENOVAR, DEVOLVER y BUSCAR que el Servidor puede leer
###### ERR
Este tipo de dato no está asociado a ninguna estructura, se usa para indicar un error genérico como respuesta

## Protocolo de comunicación
* Sólo existe un pipe (Cliente -> Servidor) por el cual todos los Cliente se comunican con el servidor, este pipe lo crea y destruye el Servidor
* Existe un pipe por cada cliente (Servidor -> Cliente), este pipe lo crea y destruye el cliente dueño
* El servidor tiene una lista interna con los pid de todos los cliente actualmente conectados

### Apertura de la comunicación
**Cuando el Servidor inicia comunicación:**
* Servidor debe iniciar comunicación antes que cualquier cliente y sólo lo hace una vez al ejecutarse
  
1. Servidor crea el pipe (Cliente->Servidor)
2. Servidor abre el pipe (Cliente->Servidor) para LECTURA

**Cuando el Cliente inicia comunicación:**
* Cualquier cliente tiene que iniciar comunicación después que el servidor, de no existir el pipe (Cliente->Servidor) el proceso finalizará
* el Cliente intenta establecer conexión con el servidor un número determinado de intentos [INTENTOS_ESCRITURA] y esperará una respuesta del servidor durante [TIMEOUT_COMUNICACION] segundos
  
1. Cliente abre el pipe (Cliente->Servidor) para ESCRITURA
2. Cliente crea un pipe (Servidor->Cliente)
3. Cliente envía a Servidor el nombre del pipe (Servidor->Cliente) mediante una señal [START_COM]
4. Cliente abre el pipe (Servidor->Cliente) para LECTURA
5. Servidor abre el pipe (Servidor->Cliente) para ESCRITURA
6. Servidor guarda la información de Cliente con su respectivo pipe de comunicación
7. Servidor envía una señal de confirmación a Cliente
8. Cliente espera una señal de Servidor [SUCCEED_COM]

### Cierre de la comunicación

**Cuando el Cliente termina comunicación:**
* Cuando un Cliente pierda la comunicación debe terminar el proceso Cliente
* el Cliente que quiera finalizar la comunicación debe eliminar el pipe (Servidor->Cliente) asociado
* Cuando no hayan Clientes conectados al Servidor, éste también debe finalizar

1. Cliente manda una petición de terminación de comunicación al Servidor
2. Servidor cierra la escritura del pipe (Servidor->Cliente)
3. Servidor actualiza la lista de clientes
4. Cliente espera a que se cierre el pipe
5. Cliente cierra la lectura del pipe (Servidor->Cliente)
6. Cliente elimina el pipe (Servidor->Cliente)
7. Cliente cierra la escritura del pipe (Cliente->Servidor)
8. El proceso Cliente finaliza

## Errores
Si existe un error en la ejecución de alguno de los procesos o solicitudes, el programa puede retornar alguno de los siguientes códigos de error:

Errores genéricos
> #define SUCCESS_GENERIC 0  // Exitoso
> #define FAILURE_GENERIC -1 // Falló
> #define ERROR_MEMORY -6    // Error de alojamiento de memoria
> #define ERROR_FATAL 1      // Error irrecuperable
> #define ERROR_ARG_NOVAL 2  // Error en argumentos

Apertura de arhivos
> #define ERROR_APERTURA_ARCHIVO 3
> #define ERROR_CIERRE_ARCHIVO 4

Error de pipes
> #define ERROR_PIPE_SER_CTE 5
> #define ERROR_PIPE_CTE_SER 6
> #define ERROR_COMUNICACION 7

Lectura / Escritura
> #define ERROR_LECTURA 8
> #define ERROR_ESCRITURA 9

Otros errores
> #define ERROR_PID_NOT_EXIST -3
> #define ERROR_SOLICITUD 10

## Créditos
Proyecto para la materia de Sistemas Operativos

Pontificia Universidad Javeriana, Facultad de Ingeniería

* Ángel David Talero
* Juan Esteban Urquijo
* Humberto Rueda Cataño