//
// Created by utnso on 07/04/20.
//
//TODO: arreglar archivode config
#include "gameboy.h"

t_log *logger;
config_struct config_params;

int main(int argc, char *argv[]) {

    leer_configuracion();
    logger = log_create("gameboy_log", "Gameboy", 1, LOG_LEVEL_INFO);//LOG_LEVEL_ERROR

    if (argc > 2) {
        switch (str2Proces(argv[1])) {
            case BROKER:
                broker_distribuidor(argc, argv);
                break;
            case TEAM:
                team_distribuidor(argc, argv);
                break;
            case GAMECARD:
                gamecard_distribuidor(argc, argv);
                break;
            case SUSCRIPTOR:
                if (argc < PARAMETROS_SUSCRIPCION) {
                    msj_error();
                    break;
                }
                suscribir(argv[2], argv[3]);
                break;
            default:
                msj_error();
        }
    } else
        msj_error();

    //config_destroy(archConfig);
    //log_destroy(logger);
}

void leer_configuracion(){
    t_config * archConfig = config_create("../gameboy_config");

    config_params.ip_broker = config_get_string_value(archConfig, "IP_BROKER");
    config_params.puerto_broker = config_get_int_value(archConfig, "PUERTO_BROKER");
    config_params.ip_team = config_get_string_value(archConfig, "IP_TEAM");
    config_params.puerto_team = config_get_int_value(archConfig, "PUERTO_TEAM");
    config_params.ip_gamecard = config_get_string_value(archConfig, "IP_GAMECARD");
    config_params.puerto_gamecard = config_get_int_value(archConfig, "PUERTO_GAMECARD");
    config_params.mac = config_get_int_value(archConfig, "MAC");

    //config_destroy(archConfig);
}

void msj_error() {
    printf("Erro en argumentos\n");
}

/**********************************
 * Funciones de convercion
 **********************************/

/*Busca en un key-value con los nombres del proceso para pasarlo a un valor numerico de enum
 * @param Un string con el nombre del proceso
 * @return el int que corresponde al proceso
 */
int str2Proces(const char *str) {
    int j;
    for (j = 0; j < sizeof(conversionProces) / sizeof(conversionProces[0]); ++j)
        if (!strcmp(str, conversionProces[j].str))
            return conversionProces[j].val;
    return -1;
}

/*Busca en un key-value con los nombres de los mensajes para pasarlo a un valor numerico de enum
 * @param Un string con el nombre del mensaje
 * @return el int que corresponde al mensaje
 */
int str2Msj(const char *str) {
    int j;
    for (j = 0; j < sizeof(conversionMsj) / sizeof(conversionMsj[0]); ++j)
        if (!strcmp(str, conversionMsj[j].str))
            return conversionMsj[j].val;
    return -1;
}

/*
 * Busca en un key-value con los nombres de la cola para pasarlo a un valor numerico de enum
 * @param Un string con el nombre de la cola
 * @return el int que corresponde al mensaje
 */
int str2Queue(const char *str) {
    int j;
    for (j = 0; j < sizeof(conversionQueue) / sizeof(conversionQueue[0]); ++j)
        if (!strcmp(str, conversionQueue[j].str))
            return conversionQueue[j].val;
    return -1;
}

/*
 * Funcion para convertir un string ok/fail a int
 */
int okFailToInt(char *resultado) {
    if (strcmp("OK", resultado) == 0) {
        return 1;
    } else {
        return (strcmp("FAIL", resultado) == 0) ? 0 : -1;
    }
}

/*************************************************************************
 * Funciones para la distribucion de mensajes por las distintas funciones
 *************************************************************************/

/*
 * Estas funciones se encargan de ver que tipo de mensaje es y tratarlo
 * @params el numero de argumentos con el que invocaron al main
 * @params un vector de char* que tiene todos los argumentos que le pasaron al main
 */
void broker_distribuidor(int argc, char *argv[]) {
    t_paquete *paquete;
    void *mensaje_serializado;
    int id;

    switch (str2Msj(argv[2])) {
        // ./gameboy BROKER NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD]
        case NEW_POK:

            if (argc < PARAMETROS_BROKER_NEW) {
                msj_error();
                break;
            }

            paquete = create_package(NEW_POK);
            t_new_pokemon *new_pokemon = create_new_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            add_to_package(paquete, mensaje_serializado, sizeof_new_pokemon(new_pokemon));

            //Envio el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_broker, config_params.puerto_broker);

            //Libero
            free(mensaje_serializado);
            free(new_pokemon);
            free_package(paquete);
            break;

        // ./gameboy BROKER APPEARED_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE_CORRELATIVO]
        case APPEARED_POK:

            if (argc < PARAMETROS_BROKER_APPEARED) {
                msj_error();
                break;
            }
            paquete = create_package(APPEARED_POK);
            t_appeared_pokemon *appeared_pokemon = create_appeared_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]));
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            id = atoi(argv[6]);
            add_to_package(paquete, (void *) &id, sizeof(uint32_t));
            add_to_package(paquete, mensaje_serializado, sizeof_appeared_pokemon(appeared_pokemon));

            //Envio el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_broker, config_params.puerto_broker);

            //Libero
            free(appeared_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        // ./gameboy BROKER CATCH_POKEMON [POKEMON] [POSX] [POSY]
        case CATCH_POK:
            if (argc < PARAMETROS_BROKER_CATCH) {
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POK);
            t_catch_pokemon *catch_pokemon = create_catch_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]));
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            add_to_package(paquete, mensaje_serializado, sizeof_catch_pokemon(catch_pokemon));

            //Envio el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_broker, config_params.puerto_broker);

            //Libero
            free(catch_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        //  ./gameboy BROKER CAUGHT_POKEMON [ID_MENSAJE_CORRELATIVO] [OK/FAIL]
        case CAUGHT_POK:
            if (argc < PARAMETROS_BROKER_CAUGHT) {
                msj_error();
                break;
            }
            paquete = create_package(CAUGHT_POK);
            t_caught_pokemon *caught_pokemon = create_caught_pokemon(okFailToInt(argv[4]));
            mensaje_serializado = caught_pokemon_a_void(caught_pokemon);
            id = atoi(argv[3]);
            add_to_package(paquete, (void *) &id, sizeof(uint32_t));
            add_to_package(paquete, mensaje_serializado, sizeof_caught_pokemon(caught_pokemon));

            //Envio el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_broker, config_params.puerto_broker);

            //Libero
            free(caught_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        // ./gameboy BROKER GET_POKEMON [POKEMON]
        case GET_POK:
            if (argc < PARAMETROS_BROKER_GET) {
                msj_error();
                break;
            }
            paquete = create_package(GET_POK);
            t_get_pokemon *get_pokemon = create_get_pokemon(argv[3]);
            mensaje_serializado = get_pokemon_a_void(get_pokemon);
            add_to_package(paquete, mensaje_serializado, sizeof_get_pokemon(get_pokemon));

            //Envio el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_broker, config_params.puerto_broker);

            free(get_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        default:
            printf("Error ese mensaje no se puede mandar al broker");
    }

}

void team_distribuidor(int argc, char *argv[]) {
    t_paquete *paquete;
    void *mensaje_serializado;

    switch (str2Msj(argv[2])) {

        // ./gameboy TEAM APPEARED_POKEMON [POKEMON] [POSX] [POSY]
        case APPEARED_POK:
            if (argc < PARAMETROS_TEAM_APPEARED) {
                msj_error();
                break;
            }

            paquete = create_package(APPEARED_POK);
            //Le tengo que mandar un id de mentira a eze para que pueda usar la misma funcion que usa con el broker
            uint32_t id_para_eze = -1;
            uint32_t id_correlativo_para_eze = 0;

            //Agrego los ids
            add_to_package(paquete, &id_para_eze,sizeof(uint32_t));
            add_to_package(paquete, &id_correlativo_para_eze,sizeof(uint32_t));

            //Agrego el mensaje apear
            t_appeared_pokemon *appeared_pokemon = create_appeared_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]));
            mensaje_serializado = appeared_pokemon_a_void(appeared_pokemon);
            add_to_package(paquete, mensaje_serializado, sizeof_appeared_pokemon(appeared_pokemon));

            //Mando el mensaje a su respectiva direccion dependiendo del proceso
            envio_mensaje(paquete, config_params.ip_team, config_params.puerto_team);

            //Libero
            free(appeared_pokemon);
            free(mensaje_serializado);
            free_package(paquete);

            break;

        default:
            printf("Error ese mensaje no se puede mandar al broker");
    }
}

void gamecard_distribuidor(int argc, char *argv[]) {
    t_paquete *paquete;
    void *mensaje_serializado;
    int id;
    // Es un id de mentira que lo paso para respetar el formato del broker
    int id_correlativo = 0;

    switch (str2Msj(argv[2])) {

        // ./gameboy GAMECARD NEW_POKEMON [POKEMON] [POSX] [POSY] [CANTIDAD] [ID_MENSAJE]
        case NEW_POK:
            if (argc < PARAMETROS_GAMECARD_NEW) {
                msj_error();
                break;
            }
            paquete = create_package(NEW_POK);
            t_new_pokemon *new_pokemon = create_new_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
            mensaje_serializado = new_pokemon_a_void(new_pokemon);
            id = atoi(argv[7]);

            //Agrego los datos al paquete
            add_to_package(paquete, (void *) &id, sizeof(uint32_t));
            add_to_package(paquete, (void *) &id_correlativo, sizeof(uint32_t));
            add_to_package(paquete, mensaje_serializado, sizeof_new_pokemon(new_pokemon));

            //Mando el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_gamecard, config_params.puerto_gamecard);

            //Libero
            free(new_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        // ./gameboy GAMECARD CATCH_POKEMON [POKEMON] [POSX] [POSY] [ID_MENSAJE]
        case CATCH_POK:
            if (argc < PARAMETROS_GAMECARD_CATCH) {
                msj_error();
                break;
            }

            paquete = create_package(CATCH_POK);
            t_catch_pokemon *catch_pokemon = create_catch_pokemon(argv[3], atoi(argv[4]), atoi(argv[5]));
            mensaje_serializado = catch_pokemon_a_void(catch_pokemon);
            id = atoi(argv[6]);

            //Agrego los datos al paquete
            add_to_package(paquete, (void *) &id, sizeof(uint32_t));
            add_to_package(paquete, (void *) &id_correlativo, sizeof(uint32_t));
            add_to_package(paquete, mensaje_serializado, sizeof_catch_pokemon(catch_pokemon));

            //Mando el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_gamecard, config_params.puerto_gamecard);

            //Libero
            free(catch_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        // ./gameboy GAMECARD GET_POKEMON [POKEMON] [ID_MENSAJE]
        case GET_POK:
            if (argc < PARAMETROS_GAMECARD_GET) {
                msj_error();
                break;
            }
            paquete = create_package(GET_POK);
            t_get_pokemon *get_pokemon = create_get_pokemon(argv[3]);
            mensaje_serializado = get_pokemon_a_void(get_pokemon);

            int id = atoi(argv[4]);

            //Agrego los datos al paquete
            add_to_package(paquete, &id, sizeof(uint32_t));
            add_to_package(paquete, (void *) &id_correlativo, sizeof(uint32_t));
            add_to_package(paquete, mensaje_serializado, sizeof_get_pokemon(get_pokemon));

            //Mando el mensaje a quien corresponda
            envio_mensaje(paquete, config_params.ip_gamecard, config_params.puerto_gamecard);

            //Libero
            free(get_pokemon);
            free(mensaje_serializado);
            free_package(paquete);
            break;

        default:
            printf("Error ese mensaje no se puede mandar al broker");
    }
}

/*
 * Funciones de envio de mensajes
 */

int envio_mensaje(t_paquete *paquete, char *ip, uint32_t puerto) {
    int server_socket = create_socket();

    if (server_socket == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    if (connect_socket(server_socket, ip, puerto) == -1) {
        log_error(logger, "Conexion fallida ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    log_info(logger, "Se logro conexion con ip: %s, puerto: %d\n", ip, puerto);

    if (send_package(paquete, server_socket) == -1) {
        log_error(logger, "Error al enviar paquete ip:%s, puerto:%d", ip, puerto);
        close_socket(server_socket);
        return -1;
    }

    // Trato de recibir el encabezado de la respuesta
    MessageHeader* buffer_header = malloc(sizeof(MessageHeader));
    if(receive_header(server_socket, buffer_header) <= 0) {
        log_info(logger, "No recibi un header");
        return false;
    }

    //Esto solo si es el proceso broker creo
    // Recibo la confirmacion
    t_list* rta_list = receive_package(server_socket, buffer_header);
    int rta = *(int*) list_get(rta_list, 0);
    printf("Llego mensaje de confirmacion exitoso: %d\n", rta);//TODO:Revisar si estoy hay que logearlo

    // Limpieza
    free(buffer_header);

    list_destroy_and_destroy_elements(rta_list, free);

    /////////////////////////////////////////////////////////
    //log_info(logger, "Se envio un mensaje a la ip: %s, puerto: %d\n", ip, puerto);

    close_socket(server_socket);
    return 1;

}


/*****************************************************
 * Estas funciones se usan en la suscripcion de colas
 *****************************************************/

/*
 * Funcion principal de suscripcion
 * @params: string del nombre de la cola
 * @params: el tiempo en formato string
 */

// ./gameboy SUSCRIPTOR [COLA_DE_MENSAJES] [TIEMPO]
void suscribir(char *cola_mensaje, char *tiempo) {
    //Genera el socket para enviar al broker
    int broker = crear_broker_socket();

    if (broker == -1) {
        printf("Error al crear el socket\n");
        return;
    }
    int resultado_suscripcion = suscribir_broker( broker , cola_mensaje );

    MessageHeader *buffer_header = malloc(sizeof(MessageHeader));
    t_list *rta_list;
    int datos_recividos;
    int id_correlativo;
    if ( resultado_suscripcion == 1 ) {

        timer(atoi(tiempo));

        bool a = true;// Para que salga la sombra amarrilla
        while (a) {

            //Recino el header y despues recibo los datos del mensaje
            datos_recividos = receive_header(broker, buffer_header);
            rta_list = receive_package(broker, buffer_header);

            //Logeo el mensaje
            logear_mensaje(buffer_header, rta_list);
            int id_mensaje = *(int*) list_get(rta_list, 0);
            int id_correlativo = *(int*) list_get(rta_list, 1);

            //Libero la lista
            list_destroy_and_destroy_elements(rta_list, &free);

            // Mando el ACK
            t_paquete* paquete = create_package(ACK);
            add_to_package(paquete, (void*) &config_params.mac, sizeof(int));
            add_to_package(paquete, (void*) &id_mensaje, sizeof(int));
            send_package(paquete, broker);

            free_package(paquete);
        }
    }
}

/*
 * Funcion para crear el socket que conecta con el
 * Broker
 */

int crear_broker_socket(){
    int broker = create_socket();

    char *ip_broker = config_params.ip_broker;
    int puerto_broker = config_params.puerto_broker;

    //Reviso si el socket se creo
    if (broker == -1) {
        printf("Error al crear el socket\n");
        return -1;
    }

    //Lo conecto con el broker
    if (connect_socket(broker, ip_broker, puerto_broker) == -1) {
        log_error(logger, "Conexion fallida ip:%s, puerto:%d", ip_broker, puerto_broker);
        close_socket(broker);
        return -1;
    }

    log_info(logger, "Se logro conexion con ip: %s, puerto: %d\n", ip_broker, puerto_broker);


    return broker;
}

/*
 * Funcion para el envio de mensaje de mensaje con el broker
 * y recepcion de la respuesta
 * @param: socket conectado con el broker
 * @param: string con el nombre de la cola
 */

int suscribir_broker(int broker_socket,char *cola_mensaje){

    //Creo un paquete para suscribirme a la cola pedida
    t_paquete *paquete = create_package(str2Queue(cola_mensaje));

    int mac = config_params.mac;
    add_to_package(paquete,(void*) &mac, sizeof(int));//Mi Identificador

    //Mando el mensaje de suscripcion al broker
    if (send_package(paquete, broker_socket) == -1) {
        printf("Envio fallido");
        return -1;
    }

    // Limpieza
    free_package(paquete);

    //Recibo la respuesta
    MessageHeader *buffer_header = malloc(sizeof(MessageHeader));
    if (receive_header(broker_socket, buffer_header) <= 0) {
        return -1;
    }

    // Recibo la confirmacion
    t_list *rta_list = receive_package(broker_socket, buffer_header);
    int rta = *(int *) list_get(rta_list, 0);

    list_destroy_and_destroy_elements(rta_list, &free);
    free(buffer_header);

    return (rta == 1)? 1 : -1;
}

/*
 * Funcion para el logeo de los distintos mensajaes que se
 * reciben del broker
 * @param: El header del mensaje para saber el tipo de mensaje
 * @param: La lista que obtenes al recivir los distintos mensajes
 */

void logear_mensaje(MessageHeader *buffer_header, t_list *rta_list) {
    int id_correlativo = *(int *) list_get(rta_list, 0);
    void *mensaje = list_get(rta_list, 2);

    switch (buffer_header->type) {
        case NEW_POK: {
            t_new_pokemon *newPokemon = void_a_new_pokemon(mensaje);

            //Arreglo error de fin de caracter
            char* nombre_pokemon = malloc(newPokemon->nombre_pokemon_length + 1);
            memcpy(nombre_pokemon, newPokemon->nombre_pokemon, newPokemon->nombre_pokemon_length);
            nombre_pokemon[newPokemon->nombre_pokemon_length] = '\0';

            free(newPokemon->nombre_pokemon);
            newPokemon->nombre_pokemon = nombre_pokemon;

            //Logeo
            log_info(logger, "Mensaje de cola NEW_POKEMON Id correlativo: %d Nombre Pokemon: %s Cantidad: %d Posicion: (%d,%d)",
                     id_correlativo, newPokemon->nombre_pokemon, newPokemon->cantidad, newPokemon->pos_x,
                     newPokemon->pos_y);

            //Libero
            free(newPokemon->nombre_pokemon);
            //free(newPokemon);
            printf("Llego un mensaje new_pok\n");
            break;
        }
        case APPEARED_POK: {
            t_appeared_pokemon *appearedPokemon = void_a_appeared_pokemon(mensaje);

            //Arreglo error de fin de caracter
            char* nombre_pokemon = malloc(appearedPokemon->nombre_pokemon_length + 1);
            memcpy(nombre_pokemon, appearedPokemon->nombre_pokemon, appearedPokemon->nombre_pokemon_length);
            nombre_pokemon[appearedPokemon->nombre_pokemon_length] = '\0';

            free(appearedPokemon->nombre_pokemon);
            appearedPokemon->nombre_pokemon = nombre_pokemon;

            log_info(logger, "Mensaje de cola APEPEARED_POKEMON Id correlativo: %d\n Nombre Pokemon: %s \nPosicion: (%d,%d)\n",
                     id_correlativo, appearedPokemon->nombre_pokemon, appearedPokemon->pos_x, appearedPokemon->pos_y);
            free(appearedPokemon->nombre_pokemon);
            //free(appearedPokemon);
            break;
        }
        case CATCH_POK: {
            t_catch_pokemon *catchPokemon = void_a_catch_pokemon(mensaje);

            //Arreglo error de fin de caracter
            char* nombre_pokemon = malloc(catchPokemon->nombre_pokemon_length + 1);
            memcpy(nombre_pokemon, catchPokemon->nombre_pokemon, catchPokemon->nombre_pokemon_length);
            nombre_pokemon[catchPokemon->nombre_pokemon_length] = '\0';

            free(catchPokemon->nombre_pokemon);
            catchPokemon->nombre_pokemon = nombre_pokemon;

            log_info(logger, "Mensaje de cola CATCH_POKEMON Id correlativo: %d\n Nombre Pokemon: %s \nPosicion: (%d,%d)\n",
                     id_correlativo,
                     catchPokemon->nombre_pokemon, catchPokemon->pos_x, catchPokemon->pos_y);
            free(catchPokemon->nombre_pokemon);
            //free(catchPokemon);
            break;
        }
        case CAUGHT_POK: {
            t_caught_pokemon *caughtPokemon = void_a_caught_pokemon(mensaje);

            log_info(logger, "Mensaje de cola CAUGHT_POKEMON Id correlativo: %d\n Fue atrapado: %d\n", id_correlativo,
                     caughtPokemon->atrapado);
            //free(caughtPokemon);
            break;
        }
        case GET_POK: {
            t_get_pokemon *getPokemon = void_a_get_pokemon(mensaje);

            //Arreglo error de fin de caracter
            char* nombre_pokemon = malloc(getPokemon->nombre_pokemon_length + 1);
            memcpy(nombre_pokemon, getPokemon->nombre_pokemon, getPokemon->nombre_pokemon_length);
            nombre_pokemon[getPokemon->nombre_pokemon_length] = '\0';


            free(getPokemon->nombre_pokemon);
            getPokemon->nombre_pokemon = nombre_pokemon;

            log_info(logger, "Mensaje de cola GET_POKEMON Id correlativo: %d\n Nombre: %s\n", id_correlativo,
                     getPokemon->nombre_pokemon);
            free(getPokemon->nombre_pokemon);
            //free(getPokemon);
            break;
        }
        case LOCALIZED_POK: {
            t_localized_pokemon *localizedPokemon = void_a_localized_pokemon(mensaje);

            //Arreglo error de fin de caracter
            char* nombre_pokemon = malloc(localizedPokemon->nombre_pokemon_length + 1);
            memcpy(nombre_pokemon, localizedPokemon->nombre_pokemon, localizedPokemon->nombre_pokemon_length);
            nombre_pokemon[localizedPokemon->nombre_pokemon_length] = '\0';

            free(localizedPokemon->nombre_pokemon);
            localizedPokemon->nombre_pokemon = nombre_pokemon;

            log_info(logger, "Mensaje de cola LOCALIZED_POKEMON Id correlativo: %d\n Nombre: %s Cantidad de coordenadas: %d\n",
                     id_correlativo, localizedPokemon->nombre_pokemon, localizedPokemon->cantidad_coordenas);
            for (int i = 0; i < (localizedPokemon->cantidad_coordenas*2); i+=2) {
                printf("Coordenadas (%d, %d)\n", localizedPokemon->coordenadas[i], localizedPokemon->coordenadas[i+1]);
            }
            free(localizedPokemon->nombre_pokemon);
            //free(localizedPokemon);
            break;
        }
        default:
            printf("Operacion desconocida. No se va a logear eso Ñery\n");

            break;
    }
}

/*
 * Inicia el Timer para parar la ejecucion y
 * setea todos los parametros necesarios
 * @param tiempo de ejecucion
 */

void timer(int tiempo) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGALRM, &sa, 0);

    struct itimerval timer;
    timer.it_value.tv_sec = tiempo;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;


    setitimer(ITIMER_REAL, &timer, 0);
}

/*
 * Sirve para atrapar la senial de alarma
 * @params la señal
 */

void timer_handler(int signum) {
    printf("\nTermino el tiempo!\n");
    log_destroy(logger);
    //config_destroy(archConfig);
    exit(-1);
}
