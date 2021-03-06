#ifndef COMMLIB_CONNECTIONS_H_
#define COMMLIB_CONNECTIONS_H_

#define MAX_CONN 40
#define null NULL
#define BILLION 1000000000L

#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <limits.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/syscall.h>
//#include <readline/readline.h>
//#include <readline/history.h>
#include "structures.h"

int client_socket_array[MAX_CONN];

/**
 * Creo un socket
 * @return -1 en caso de error, sino el socket en cuestion
 */
int create_socket();

/**
 * Uno un socket a un puerto del SERVIDOR para recibir/enviar mensajes por este
 * @param socket, socket creado en la funcion create_socket
 * @param port, puerto que quiero abrir en el servidor
 * @return -1 en caso de error, 0 en caso contrario
 */
int bind_socket(int socket, int port);

/**
 * Analoga del bind_socket pero para el CLIENTE, le asigno la ip de un servidor y el puerto en el que este escucha a
 * un socket
 * @param socket, socket creado en la funcion create_socket
 * @param IP, IP sobre la que voy a enviar/recibir mensajes
 * @param port, puerto sobre el que voy a enviar/ recibir mensajes
 * @return -1 en caso de error, 0 en caso contrario
 */
int connect_socket(int socket, char * IP, int port);

/**
 * Esta funcion sirve para cerrar los sockets, siempre hacerlo ya que sino la direccion a la que esta conectado puede
 * quedar bloqueada por unos minutos
 * @param socket, socket a cerrar
 * @return 0, creo que no puede fallar?
 */
int close_socket(int socket);

/**
 * Recibo el header del paquete para saber de que tipo es y mandarlo a la funcion correspondiente
 * @param socket, socket sobre el que voy a recibir el mensaje
 * @param buffer, puntero a MessageHeader sobre el que voy a cargar el header recibido
 * @return -1 hubo un error, 0 el servidor se desconecto, +0 cant de bytes recibidos
 */
int receive_header(int socket, MessageHeader * buffer);

/**
 * Servidor creado utilizando select, recibe 3 funciones, las que se encargan de todos los tipos posibles de acciones
 * @param socket, socket que abri
 * @param new_connection, funcion que se ejecuta al recibir una nueva conexion
 * @param lost_connection, funcion que se ejecuta al perder una conexion
 * @param incoming_message, funcion que se ejecuta al recibir un mensaje nuevo, el grueso de la funcionalidad estara
 * aca, ya que recibe el header del paquete y decide que hacer segun el tipo del mismo
 * @return
 */
int start_server(int socket,
    void (*new_connection)(int fd, char * ip, int port),
    void (*lost_connection)(int fd, char * ip, int port),
    void (*incoming_message)(int fd, char * ip, int port, MessageHeader * header));

/**
 * Creo un paquete un paquete vacion de un tipo dado al que luego le tengo que agregar datos
 * @param tipo, tipo de mensaje al que pertenece el paquete
 * @return el paquete creado
 */
t_paquete* create_package(MessageType tipo);

/**
 * Agrego un elemnto a un paquete dado
 * @param paquete, paquete al que le voy a agregar datos
 * @param valor, elemento que le voy a agregar al paquete
 * @param tamanio, tamanio del elemento a enviar
 */
void add_to_package(t_paquete* paquete, void* valor, int tamanio);

/**
 * Envio un paquete a un socket dado
 * @param paquete, paquete a enviar
 * @param socket_cliente, socket al que le envio el paquete
 * @return -1 en caso de error, cant de bytes en caso contrario
 */
int send_package(t_paquete* paquete, int socket_cliente);

/**
 * Serializo un paquete a enviar, queda acomodado de tal manera que simplifica su recepcion(supuestamente), esta
 * funcion solo se utiliza internamente, no se en que otro contexto se podria utilizar
 * @param paquete, paquete a serializar
 * @param bytes, cantidad de bytes del paquete a serializar
 * @return retorna el paquete serializado
 */
void* serialize_package(t_paquete* paquete, int bytes);

/**
 * Funcion que libera la mempria reservada por un paquete, SIEMPRE se debe eliminar luego de utilizado ya que es memoria
 * dinamica y genera leaks
 * @param paquete
 */
void free_package(t_paquete* paquete);

/**
 * Esta funcion recibe el contenido de un paquete y lo deserializa, luego agarra cada elemento y los agrega a una
 * lista(t_list), cada elemento de la lista es un void*, por lo que hay que hacer los casteos correspondientes.
 * Esta funcion esta divida en 2, la otra es receive_header, esto es asi ya que el servidor necesitaba solo el header
 * de los paquetes para hacer ciertas verificaciones.
 * @param socket_cliente, socket sobre el que recibe el paquete
 * @param header, header recibido con la funcion receive_header
 * @return retorna los elementos contenidos en el paquete recibido en una lista
 */
t_list* receive_package(int socket_cliente, MessageHeader *header);

/**
 * Esta funcion es la que se ejecuta en cada hilo del servidor multihilos
 * @param params, en esta bolsa de paramentros se incluye la ip y el puerto que iniciaron esta conexion, y las
 * funciones para la perdida de conexion y cuando se recibe un mensaje nuevo
 * @return nada, esta firma es asi ya que la biblioteca pthread la necesita
 */
void* server_client(void* params);

/**
 * Este es un servidor implementado de otra manera, con hilos en vez de utilizar un select, cada conexion abre un hilo
 * nuevo, mantiene la misma interfaz que el anterior
 * @param socket, socket del servidor
 * @param new_connection, funcion que se ejecuta al recibir una nueva conexion
 * @param lost_connection, funcion que se ejecuta al perder una conexion
 * @param incoming_message, funcion que se ejecuta al recibir un mensaje nuevo
 * @return
 */
int start_multithread_server(int socket, void (*new_connection)(int fd, char *ip, int port),
    void (*lost_connection)(int fd, char *ip, int port),
    void (*incoming_message)(int fd, char *ip, int port, MessageHeader *header));


/**
 * Constructor de t_new_pokemon
 * @param nombre_pokemon, nombre del pokemon
 * @param pos_x, Posicion en X
 * @param pos_y, Posicion en Y
 * @param cantidad, Cantidad de pokemon de esa especie en esa posicion
 * @return Puntero a la estructura creada
 */
t_new_pokemon* create_new_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y, uint32_t cantidad);

/**
 * Serealizar t_new_pokemon
 * @param new_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* new_pokemon_a_void(t_new_pokemon* new_pokemon);

/**
 * Deserealizar t_new_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_new_pokemon* void_a_new_pokemon(void* stream);


/**
 * Constructor de t_get_pokemon
 * @param nombre_pokemon, nombre del pokemon
 * @return Puntero a la estructura creada
 */
t_get_pokemon* create_get_pokemon(char* nombre_pokemon);

/**
 * Serealizar t_get_pokemon
 * @param get_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* get_pokemon_a_void(t_get_pokemon* get_pokemon);

/**
 * Deserealizar t_get_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_get_pokemon* void_a_get_pokemon(void* stream);


/**
 * Constructor de t_localized_pokemon
 * @param nombre_pokemon, Nombre del pokemon
 * @param cantidad_coordenadas, Cantidad de pares de coordenadas que le voy a pasar por parametro
 * @param coordenas, posicion en x coma posicion en y
 * @return Puntero a la estructura creada
 */
t_localized_pokemon* create_localized_pokemon(char* nombre_pokemon, uint32_t cantidad_coordenadas, uint32_t* coordenadas);

/**
 * Serealizar t_localized_pokemon
 * @param localized_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* localized_pokemon_a_void(t_localized_pokemon* localized_pokemon);

/**
 * Deserealizar t_localized_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_localized_pokemon* void_a_localized_pokemon(void* stream);


/**
 * Constructor de t_caught_pokemon
 * @param atrapado, 1 es verdadero 0 es falso
 * @return Puntero a la estructura creada
 */
t_caught_pokemon* create_caught_pokemon(uint32_t atrapado);

/**
 * Serealizar t_caught_pokemon
 * @param caught_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* caught_pokemon_a_void(t_caught_pokemon* caught_pokemon);

/**
 * Deserealizar t_caught_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_caught_pokemon* void_a_caught_pokemon(void* stream);


/**
 * Constructor de t_catch_pokemon
 * @param nombre_pokemon, nombre del pokemon
 * @param pos_x, Posicion en X
 * @param pos_y, Posicion en Y
 * @return Puntero a la estructura creada
 */
t_catch_pokemon* create_catch_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y);

/**
 * Serealizar t_catch_pokemon
 * @param catch_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* catch_pokemon_a_void(t_catch_pokemon* catch_pokemon);

/**
 * Deserealizar t_catch_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_catch_pokemon* void_a_catch_pokemon(void* stream);


/**
 * Constructor de t_catch_pokemon
 * @param nombre_pokemon, nombre del pokemon
 * @param pos_x, Posicion en X
 * @param pos_y, Posicion en Y
 * @return Puntero a la estructura creada
 */
t_appeared_pokemon* create_appeared_pokemon(char* nombre_pokemon, uint32_t pos_x, uint32_t pos_y);

/**
 * Serealizar t_appeared_pokemon
 * @param appeared_pokemon, puntero a la estructura
 * @return Puntero a void de la estructura
 */
void* appeared_pokemon_a_void(t_appeared_pokemon* appeared_pokemon);

/**
 * Deserealizar t_appeared_pokemon
 * @param stream, puntero void de la estructura
 * @return Puntero a la estructura
 */
t_appeared_pokemon* void_a_appeared_pokemon(void* stream);

/**
 * @return timestamp
 * */
uint64_t unix_epoch();


/**
 * @return tamanio de new_pokemon
 * */
size_t sizeof_new_pokemon(t_new_pokemon* estructura);

/**
 * @return tamanio de appeared_pokemon
 * */
size_t sizeof_appeared_pokemon(t_appeared_pokemon* estructura);

/**
 * @return tamanio de get_pokemon
 * */
size_t sizeof_get_pokemon(t_get_pokemon* estructura);

/**
 * @return tamanio de localized_pokemon
 * */
size_t sizeof_localized_pokemon(t_localized_pokemon* estructura);

/**
 * @return tamanio de catch_pokemon
 * */
size_t sizeof_catch_pokemon(t_catch_pokemon* estructura);

/**
 * @return tamanio de caught_pokemon
 * */
size_t sizeof_caught_pokemon(t_caught_pokemon* estructura);


#endif
