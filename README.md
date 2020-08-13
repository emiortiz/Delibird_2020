
# tp-2020-1c-Recursantes-3.0
![entrega-final](https://imgur.com/HWoy6Iv.jpg)
## Indice
- [Introduccion al TP] (#Introduccion-al-TP)
- [Procesos] (#Procesos)
- [Comunicacion con el Broker](#comunicacion-con-el-broker)
  -  [Suscribirse a una cola](#suscribirse-a-una-cola)
  - [Mandar un mensaje](#mandar-un-mensaje)
  - [Recibir un mensaje](#recibir-un-mensaje)
 - [Buildear](buildear) 

## Introduccion al TP
El TP consiste en la simulacion de distintas funciones del sistema operativo. A travez de procesos que ejecutan tareas con pokemons y pueden funcionar gracias a una cola de mensajes que permite comunicarse a todos los procesos, con la persistencia. 

## Procesos
 -Gameboy
 -Team
 -Broker
 -Gamecard
 

## Comunicacion con el Broker

 - Primero hay que subscribirse a una cola, pasandole al Broker un id propio.
 - Cada vez que se manda un mensaje el Broker va a devolver un id unico para ese mensaje.
 - Los mensajes que el Broker manda a las colas estan compuestos por [0]id_correlacional(int) y [1]contenido del mensaje(struct_pokemon)
 - Cada vez que el Broker manda un mensaje a una cola hay que mandarle un `ACK` para marcarlo como recibido.

*Para mejores ejemplos ver [cliente-test.c](cliente-test/cliente-test.c)*

### Suscribirse a una cola 

```c
// Creo un paquete para la suscripcion a una cola
t_paquete* paquete = create_package(cola);

int* id = malloc(sizeof(int));
*id = 69; // Numero para identificarse ante el Broker
add_to_package(paquete, (void*) id, sizeof(int));

// Envio el paquete
send_package(paquete, broker_fd)

// Recibo el encabezado de la respuesta
MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
receive_header(broker_fd, buffer_header)

// Recibo la confirmacion (Siempre va a ser 1)
t_list* rta_list = receive_package(broker_fd, buffer_header);
int rta = *(int*) list_get(rta_list, 0);
```

### Mandar un mensaje
```c
// Creo el paquete
t_paquete* paquete = create_package(NEW_POK);

// Cargo la estructura del pokemon
t_new_pokemon* new_pok = malloc(sizeof(t_new_pokemon));
new_pok = create_new_pokemon("Pikachu", 1, 2, 3);
size_t tam = sizeof_new_pokemon(new_pok);
void* stream = new_pokemon_a_void(new_pok);

// Envio el paquete
add_to_package(paquete, stream, tam);
send_package(paquete, broker_fd);

// Recibo el encabezado de la respuesta
MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
receive_header(broker_fd, buffer_header)

// Recibo el id mensaje
t_list* rta_list = receive_package(broker_fd, buffer_header);
int id_mensaje_new_pokemon = *(int*) list_get(rta_list, 0);

```

### Recibir un mensaje
```c
MessageHeader* buffer_header = malloc(sizeof(MessageHeader));

if(receive_header(broker_fd, buffer_header) > 0) {
    t_list *rta_list = receive_package(broker_fd, buffer_header);

    if (buffer_header->type == NEW_POK) {

        // Deserealizo el stream
        int id_mensaje = *(int*) list_get(rta_list, 0);
        int id_correlativo = *(int*) list_get(rta_list, 1);
        t_new_pokemon* new_pokemon = void_a_new_pokemon(list_get(rta_list,2));


        // Mando el ACK(id_cliente, id_mensaje)
        t_paquete* paquete = create_package(ACK);
        add_to_package(paquete, (void*) &config.id_cliente, sizeof(int));
        add_to_package(paquete, (void*) &id_mensaje, sizeof(int));
        send_package(paquete, broker_fd);

        // Hacer algo con el pokemon
        meterse_a_pikachu_de_canto_en_el_orto(new_pokemon);
    }
}
```

## Build
Build la biblioteca:
```bash
cd commLib
sudo ./build_biblioteca.sh 
```
