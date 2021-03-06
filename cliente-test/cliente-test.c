//
// Created by utnso on 29/05/20.
//

/*
gcc cliente-test.c -o cliente-test -Wall -lcommons -lcommLib -lpthread; ./cliente-test
 */

#include "cliente-test.h"
MessageType sub_to_men(MessageType cola);
char* cola_to_string(MessageType cola);

pthread_mutex_t MUTEX;

int main(int argc, char **argv) {
    logger = log_create("cliente_test.log", "CLIENTE_TEST", 1, LOG_LEVEL_TRACE);
    log_info(logger, "Log started.");
    long ID;
    if (argc != 2) {
        ID = 69;
    } else {
        ID = strtol(argv[1], NULL, 10);;
    }
    log_warning(logger, "El id del proceso es: %d", ID);
    bool init_config = set_config((int)ID);
    init_config ? log_info(logger, "Config setted up successfully. :)") : log_info(logger,
                                                                                   "Error while setting config :|");
    broker_fd = connect_to_broker();
    printf("%d\n", broker_fd);

    PENDIENTES = list_create();
    pthread_mutex_init(&MUTEX, NULL);

    if (subscribir_cola(SUB_NEW)) {
        log_info(logger, "Se subscribio bien a NEW");
    }
    if (subscribir_cola(SUB_APPEARED)) {
        log_info(logger, "Se subscribio bien a APPEARED");
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server, NULL);
    pthread_detach(server_thread);

    CANTIDAD_MENSAJES_A_ENVIAR = 100;

    sleep(3);
    for (int i = 0; i < CANTIDAD_MENSAJES_A_ENVIAR; ++i) {
        int broker_socket_mensaje = connect_to_broker();
        mandar_mensaje(broker_socket_mensaje);
        close_socket(broker_socket_mensaje);
        usleep(1000);
    }
    sleep(5);
//    getchar();
    terminar();

    char x;
    scanf("%c", &x);
    x == '1' ? (close(broker_fd) == 0 ? log_info(logger, "chau") : log_info(logger, "error closing connection")) : log_info(logger, "keep connected");
}

void server(){
    bool xd = true;
    while(xd){
        usleep(1000);
        MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
        log_warning(logger, "Wait en SERVER");
        pthread_mutex_lock(&MUTEX);
        if(receive_header(broker_fd, buffer_header) > 0) {
            t_list *rta_list = receive_package(broker_fd, buffer_header);
            log_info(logger, "Recibi algo");

            switch (buffer_header->type) {
                case (NEW_POK):;
                    int id_mensaje = *(int*) list_get(rta_list, 0);
                    log_debug(logger, "Me llega un New con id \033[1;31m%d\033[0m", id_mensaje);
                    int id_correlativo = *(int*) list_get(rta_list, 1);

                    delete_msj(id_mensaje);

                    t_new_pokemon* new_pokemon = void_a_new_pokemon(list_get(rta_list,2));

                    // Mando el ACK
                    t_paquete* paquete = create_package(ACK);
                    add_to_package(paquete, (void*) &config.id_cliente, sizeof(int));
                    add_to_package(paquete, (void*) &id_mensaje, sizeof(int));

                    int resultado_send = send_package(paquete, broker_fd);
                    log_error(logger, "Resultado del send: %d", resultado_send);

                    // Limpieza
                    free_package(paquete);
                    free(new_pokemon);
                    break;

                case (APPEARED_POK):
                    break;

                default:
                    log_warning(logger, "Operacion desconocida. No quieras meter la pata\n");
            }
        }
        else {
            log_info(logger, "No hay conexion con el Broker");
            exit(EXIT_SUCCESS);
        }
        free(buffer_header);
        log_warning(logger, "Signal en SERVER");
        pthread_mutex_unlock(&MUTEX);
    }
}


bool set_config(int ID){

    config.id_cliente = ID;
    config.broker_ip = "127.0.0.1";
    config.broker_port = 5002;
    config.cliente_test_ip = "127.0.0.1";
    config.cliente_test_port = 5003;
    return (config.broker_ip && config.broker_port && config.cliente_test_ip && config.cliente_test_port) ? true : false;

}

int connect_to_broker(){

    int client_socket;
    if((client_socket = create_socket()) == -1) {
        log_error(logger, "Error creating client socket :|");
        exit(-1);
    }
    if(connect_socket(client_socket, config.broker_ip, config.broker_port) == -1){
        log_error(logger, "Error connecting to Broker :?");
        exit(-1);
    }
    log_info(logger, "Successfully connected to broker");
    return client_socket;
}

int subscribir_cola(MessageType cola){
    log_warning(logger, "Wait en SUB");
    pthread_mutex_lock(&MUTEX);
    // Creo un paquete para la suscripcion a una cola
    t_paquete* paquete = create_package(cola);

    add_to_package(paquete, (void*) &config.id_cliente, sizeof(int));

    // Envio el paquete, si no se puede enviar retorno false
    if(send_package(paquete, broker_fd)  == -1){
        return false;
    }

    // Limpieza
    free_package(paquete);

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(broker_fd, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return false;
    }

    // Recibo la confirmacion
    t_list* rta_list = receive_package(broker_fd, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);
//    log_warning(logger, "Respuesta de subscripcion a %s", cola_to_string(sub_to_men(buffer_header->type)));

    // Limpieza
    free(buffer_header);
    void element_destroyer(void* element){
        free(element);
    }
    list_destroy_and_destroy_elements(rta_list, element_destroyer);

    log_warning(logger, "Signal en SUB");
    pthread_mutex_unlock(&MUTEX);
    return rta == 1;
}

void mandar_mensaje(int fd){
    // Mando un NEW_POK
    t_new_pokemon* new_pok = malloc(sizeof(t_new_pokemon));
    new_pok = create_new_pokemon("Mamame las bolas", 1, 2, 3);
    t_paquete* paquete = create_package(NEW_POK);
    size_t tam = sizeof(uint32_t)*4 + new_pok->nombre_pokemon_length;
    add_to_package(paquete, new_pokemon_a_void(new_pok), tam);
    send_package(paquete, fd);
    log_info(logger, "Mando el mensaje de New Pokemon");

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(fd, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return;
    }
    if(buffer_header->type == NEW_POK){
        log_info(logger, "Estoy por recibir el id del mensaje, todo piola");
    } else {
        exit(-1);
    }

    // Recibo el id mensaje
    t_list* rta_list = receive_package(fd, buffer_header);
    int id_mensaje_new_pokemon = *(int*) list_get(rta_list, 0);
    log_info(logger, "Mande el mensaje mensaje: \033[1;35m%d\033[0m", id_mensaje_new_pokemon);
    list_add(PENDIENTES, create_msj(id_mensaje_new_pokemon));
}

void terminar(){
    log_debug(logger, "Falto recibir los mensajes:");
    int perdidos;
    for (int i = 0; i < list_size(PENDIENTES); ++i) {
        msj* m = list_get(PENDIENTES, i);
        log_debug(logger, "\t%d", m->id_mensaje);
        perdidos++;
    }
    log_error(logger, "Se perdieron %d/%d", perdidos, CANTIDAD_MENSAJES_A_ENVIAR);
    log_debug(logger, "FIN");
    exit(EXIT_SUCCESS);
}

msj* create_msj(int id){
    msj* nuevo_msj = malloc(sizeof(msj));
    nuevo_msj->id_mensaje = id;
    return nuevo_msj;
}

void delete_msj(int id){
    bool id_search(void* un_mensaje){
        msj* m = (msj*) un_mensaje;
        return m->id_mensaje == id;
    }
    void element_destroyer(void* element){
        free(element);
    }
    list_remove_and_destroy_by_condition(PENDIENTES, id_search, element_destroyer);
}

MessageType sub_to_men(MessageType cola) {
    switch (cola) {
        case SUB_NEW:
            return NEW_POK;
        case SUB_GET:
            return GET_POK;
        case SUB_CATCH:
            return CATCH_POK;
        case SUB_APPEARED:
            return APPEARED_POK;
        case SUB_LOCALIZED:
            return LOCALIZED_POK;
        case SUB_CAUGHT:
            return CAUGHT_POK;
        default:
            log_error(logger, "Error en sub_to_men");
            exit(EXIT_FAILURE);
    }
}

char* cola_to_string(MessageType cola) {
    switch (cola) {
        case NEW_POK:
            return "NEW_POK";
        case GET_POK:
            return "GET_POK";
        case CATCH_POK:
            return "CATCH_POK";
        case APPEARED_POK:
            return "APPEARED_POK";
        case LOCALIZED_POK:
            return "LOCALIZED_POK";
        case CAUGHT_POK:
            return "CAUGHT_POK";
        default:
            return "No es una cola";
    }
}