## Protocolo de comunicación entre Cliente - Servidor
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
3. Cliente abre el pipe (Servidor->Cliente) para LECTURA
4. Cliente envía a Servidor el nombre del pipe (Servidor->Cliente)
5. Servidor abre el pipe (Servidor->Cliente) para ESCRITURA
6. Servidor guarda la información de Cliente con su respectivo pipe de comunicación
7. Servidor envía una señal de confirmación a Cliente
8. Cliente espera una señal de Servidor

### Cierre de la comunicación
* No se puede finalizar una comunicación mientras una transacción está en curso

**Cuando el Servidor termina comunicación:**
* Si el Servidor termina la comunicación el proceso debe finalizar
* Si Servidor termina la comunicación y aún hay clientes conectados, todos los clientes están obligados a cerrar comunicación y terminar la ejecución al igual que Servidor
* Cada cliente se encarga de eliminar su propio pipe (Servidor->Cliente) y Servidor se encargará de eliminar el pipe de (Cliente->Servidor)

**Cuando el Cliente termina comunicación:**
* el Cliente que quiera finalizar la comunicación debe eliminar el pipe (Servidor->Cliente) asociado

1. Cliente manda una petición de terminación de comunicación al Servidor
2. Servidor cierra la escritura del pipe (Servidor->Cliente)
3. Servidor actualiza la lista de clientes
4. Cliente espera a que se cierre el pipe
5. Cliente cierra la lectura del pipe (Servidor->Cliente)
6. Cliente elimina el pipe (Servidor->Cliente)
7. Cliente cierra la escritura del pipe (Cliente->Servidor)
8. El proceso Cliente finaliza