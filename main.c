#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define FICHERO_PRINCIPAL "./tareas.txt"
#define TAM 256
#define SALIR '0'
#define CREAR_TAREA '1'
#define VER_TAREA '2'
#define VER_LISTA '3'
#define CANCELAR '0'
#define PENDIENTES '1'
#define TERMINADAS '2'
#define TODAS '3'
#define POR_PANTALLA '1'
#define POR_FICHERO '2'
#define GUARDAR_CAMBIOS '1'
#define DESCARTAR_CAMBIOS '2'

// definiciones de tipo
typedef char cadena[TAM];
typedef struct tm Tiempo;
typedef struct Tarea Tarea;
typedef struct Dependencia Dependencia;

// estructura Dependencia
struct Dependencia
{
    cadena valor;
    Dependencia *siguiente;
};

// estructura Tarea
struct Tarea
{
    cadena tipo;
    cadena id;
    cadena descripcion;
    cadena propietario;
    cadena prioridad;
    cadena estado;
    cadena plazo;
    Dependencia *dependencias;
    Tarea *siguiente;
};

// Utilidades
void imprimeMenu(void);
void asteriscos(int lineas, int asteriscos);
void imprimeCabecera(void);
void lee(cadena cad, FILE *F, int nChars);
void remueveEnter(cadena cad, int nChars);
void debugTareaSinDependencias(Tarea *tarea);
void debugTarea(Tarea *tarea);
void debugTareas(Tarea *tareas);
void debugTareaPorFichero(Tarea *tarea, FILE *F);
void debugDependencias(Tarea *tarea);
int asignaDependencias(Tarea *tarea, cadena dependencias);
void sTiempoActualPrint(cadena cad);
int esFechaAnterior(cadena fecha1, cadena fecha2);
int estaDentroPorDescripcion(Tarea **tareas, Tarea *tarea);
int estaDentroPorId(Tarea **tareas, Tarea *tarea);
int getMayorId(Tarea **tareas);
Tarea *getTareaPorId(Tarea **tareas, Tarea *tarea);
int diasPara(cadena fecha);
int diferenciaDeAnyos(int a1, int a2);
int diasAnyo1(int d1, int me1);
int diasAnyoDos(int d2, int me2);
void listaPendientes(Tarea **tareas, cadena usuario, FILE *F);
void listaTerminadas(Tarea **tareas, cadena usuario, FILE *F);
void listaTodas(Tarea **tareas, cadena usuario, FILE *F);
int cantidadDeTareasPendientes(Tarea **tareas, cadena usuario);
int cantidadDeTareasTerminadas(Tarea **tareas, cadena usuario);
int cantidadDeTareas(Tarea **tareas, cadena usuario);
void guardarCamios(Tarea **tareas);
int contiene(cadena cad, char c);

// Funcionalidades principales
int cargaTareas(Tarea **tareas);
void creaTarea(Tarea **tareas, cadena usuario);
void verTarea(Tarea **tareas, cadena usuario);
void verLista(Tarea **tareas, cadena usuario);

int main(void)
{
    Tarea *tareas;

    tareas = NULL;
    if (!cargaTareas(&tareas)) {
        fprintf(stderr, "Error: No se han podido cargar tareas\n");
        return 1;
    }

//    debugTareas(tareas);
//    return 0;

    cadena usuario;
    cadena opcion;
    int crearTarea;
    char c;

    crearTarea = 0;
    imprimeCabecera();
    printf("Identifiquese: ");
    lee(usuario, stdin, 1);

    while (1) {
        do {
            imprimeMenu();
            lee(opcion, stdin, 1);
            c = opcion[0];
        } while (c != '0' && c != '1' && c != '2' && c != '3');
        switch (c) {
        case CREAR_TAREA:
            crearTarea = 1;
            creaTarea(&tareas, usuario);
            break;
        case VER_TAREA:
            verTarea(&tareas, usuario);
            break;
        case VER_LISTA:
            verLista(&tareas, usuario);
            break;
        case SALIR:
            if (crearTarea) {
                do {
                    printf("1)  Guardar cambios\n");
                    printf("2)  Descartar cambios\n");
                    printf("0)  Cancelar\n");
                    printf("Opcion?: ");
                    lee(opcion, stdin, 1);
                    c = opcion[0];
                } while (c != '0' && c != '1' && c != '2');
                switch (c) {
                case GUARDAR_CAMBIOS:
                    guardarCamios(&tareas);
                    return 0;
                case DESCARTAR_CAMBIOS:
                    crearTarea = 0;
                    break;
                case CANCELAR:
                    break;
                }
            } else {
                return 0;
            }
        }
    }
    return 0;
}

void sTiempoActualPrint(cadena cad)
{
    time_t tiempo;
    Tiempo *actual;
    tiempo = time(NULL);
    actual = localtime(&tiempo);
    sprintf(cad, "%d/%d/%d:%d:%d", actual->tm_mday,
            actual->tm_mon + 1, actual->tm_year + 1900, actual->tm_hour,
            actual->tm_min);
}

int esFechaAnterior(cadena fecha1, cadena fecha2)
{
    char c;
    int dia1, mes1, anyo1, hora1, min1;
    int dia2, mes2, anyo2, hora2, min2;
    sscanf(fecha1, "%d%c%d%c%d%c%d%c%d", &dia1, &c, &mes1, &c, &anyo1, &c,
           &hora1, &c, &min1);
    sscanf(fecha2, "%d%c%d%c%d%c%d%c%d", &dia2, &c, &mes2, &c, &anyo2, &c,
           &hora2, &c, &min2);
    if (anyo1 < anyo2) {
        return 1;
    } else if (anyo1 == anyo2) {
        if (mes1 < mes2) {
            return 1;
        } else if (mes1 == mes2) {
            if (dia1 < dia2) {
                return 1;
            } else if (dia1 == dia2) {
                if (hora1 < hora2) {
                    return 1;
                } else if (hora1 == hora2) {
                    if (min1 < min2) {
                        return 1;
                    } else if (min1 == min2) {
                        return 0;
                    }
                }
            }
        }
    }
    return 0;
}

int estaDentroPorDescripcion(Tarea **tareas, Tarea *tarea)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->descripcion, tarea->descripcion) == 0) {
            return 1;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return 0;
}

int estaDentroPorId(Tarea **tareas, Tarea *tarea)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->id, tarea->id) == 0) {
            return 1;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return 0;
}

int getMayorId(Tarea **tareas)
{
    int id;
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    id = atoi(ptr_aux->id);
    while (ptr_aux != NULL) {
        if (atoi(ptr_aux->id) > id) {
            id = atoi(ptr_aux->id);
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return id;
}

Tarea *getTareaPorId(Tarea **tareas, Tarea *tarea)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->id, tarea->id) == 0) {
            return ptr_aux;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return NULL;
}

void creaTarea(Tarea **tareas, cadena usuario)
{
    Tarea T = { "", "", "", "", "", "pendiente", "-", NULL, NULL };
    Tarea K;
    Tarea *ptr_aux_tarea;
    cadena dependencias;
    cadena fechaActual;
    Dependencia *ptr_aux_dep;

    strcpy(T.propietario, usuario);
    printf("Introducir los datos de la nueva tarea:\n");
    printf("Introducir tipo de tarea: ");
    lee(T.tipo, stdin, 1);

    sprintf(T.id, "%d", getMayorId(tareas) + 1);

    if (strcmp(T.tipo, "con_plazo") == 0) {
        printf("Introducir plazo: ");
        lee(T.plazo, stdin, 1);
        sTiempoActualPrint(fechaActual);
        if (esFechaAnterior(T.plazo, fechaActual)) {
            printf("AVISO: La fecha debe ser posterior a la actual\n");
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
    }

    printf("Introducir descripcion de la tarea: ");
    lee(T.descripcion, stdin, 1);
    printf("Introducir prioridad: ");
    lee(T.prioridad, stdin, 1);

    printf("Introducir dependencias (0 si no se establecen dependencias): ");
    lee(dependencias, stdin, 1);
    asignaDependencias(&T, dependencias);

    if (estaDentroPorDescripcion(tareas, &T)) {
        printf("AVISO: Tarea ya existente\n");
        printf("Pulsar ENTER para volver al menú principal del programa:");
        getchar();
        return;
    }

    ptr_aux_dep = T.dependencias;
    while (ptr_aux_dep != NULL) {
        strcpy(K.id, ptr_aux_dep->valor);
        if (!estaDentroPorId(tareas, &K)) {
            printf("AVISO: Alguna de las depencencias indicadas es"
                   " incorrecta\n");
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
        ptr_aux_dep = ptr_aux_dep->siguiente;
    }

    // agregala a la lista
    if (*tareas == NULL) {
        // directamente
        *tareas = (Tarea *) malloc(sizeof(Tarea));
        ptr_aux_tarea = *tareas;
        *ptr_aux_tarea = T;
    } else {
        // hasta el final
        ptr_aux_tarea = *tareas;
        while (ptr_aux_tarea->siguiente != NULL) {
            ptr_aux_tarea = ptr_aux_tarea->siguiente;
        }
        ptr_aux_tarea->siguiente = (Tarea *) malloc(sizeof(Tarea));
        ptr_aux_tarea = ptr_aux_tarea->siguiente;
        *ptr_aux_tarea = T;
    }
}

void verTarea(Tarea **tareas, cadena usuario)
{
    Tarea T;
    Tarea *ptr_tarea;
    Tarea *ptr_aux_tarea;
    Dependencia *ptr_aux_dep;
    int contador;

    printf("Introducir el identificador de la tarea: ");
    lee(T.id, stdin, 1);
    ptr_tarea = getTareaPorId(tareas, &T);
    if (ptr_tarea == NULL) {
        printf("AVISO: Tarea no existente\n");
        printf("Pulsar ENTER para volver al menú principal del programa:");
        getchar();
        return;
    }
    if (strcmp(usuario, ptr_tarea->propietario) == 0) {
        debugTareaSinDependencias(ptr_tarea);
        if (strcmp(ptr_tarea->tipo, "con_plazo") == 0) {
            printf("Dias para terminar la tarea: %d\n",
                   diasPara(ptr_tarea->plazo));
        }
        printf("Tareas de las que depende la tarea:\n");
        printf("Directamente: ");
        debugDependencias(ptr_tarea);
        printf("\nIndirectamente: ");
        ptr_aux_dep = ptr_tarea->dependencias;
        if (ptr_aux_dep == NULL) {
            printf("-\n");
        } else {
            while (ptr_aux_dep != NULL) {
                strcpy(T.id, ptr_aux_dep->valor);
                debugDependencias(getTareaPorId(tareas, &T));
                ptr_aux_dep = ptr_aux_dep->siguiente;
            }
        }
        printf("Tareas dependientes directamente de la tarea: ");
        ptr_aux_tarea = *tareas;
        while (ptr_aux_tarea != NULL) {
            ptr_aux_dep = ptr_aux_tarea->dependencias;
            contador = 0;
            while (ptr_aux_dep != NULL) {
                if (atoi(ptr_aux_dep->valor) == atoi(ptr_tarea->id)) {
                    printf("%d ", atoi(ptr_aux_tarea->id));
                    ++contador;
                }
                ptr_aux_dep = ptr_aux_dep->siguiente;
            }
            ptr_aux_tarea = ptr_aux_tarea->siguiente;
        }
        if (contador == 0) {
            printf("-");
        }
        printf("\n");
        printf("Pulsar ENTER para volver al menú principal del programa:");
        getchar();
    } else {
        printf("AVISO: Operacion no valida sobre esta tarea\n");
        printf("Pulsar ENTER para volver al menú principal del programa:");
        getchar();
    }
}

void verLista(Tarea **tareas, cadena usuario)
{
    char opcionPresentar;
    char opcionDestino;
    cadena nombreFichero;
    cadena opcion;
    FILE *F;
    do {
        printf("1) Pendientes\n");
        printf("2) Terminadas\n");
        printf("3) Todas\n");
        printf("0) Cancelar\n");
        printf("Tareas a presentar?: ");
        lee(opcion, stdin, 1);
        opcionPresentar = opcion[0];
    } while (opcionPresentar != '0' && opcionPresentar != '1'
             && opcionPresentar != '2' && opcionPresentar != '3');
    if (opcionPresentar == CANCELAR) {
        return;
    }
    do {
        printf("1) Por pantalla\n");
        printf("2) A fichero\n");
        printf("0) Cancelar\n");
        printf("Presentacion de resultado?: ");
        lee(opcion, stdin, 1);
        opcionDestino = opcion[0];
    } while (opcionDestino != '0' && opcionDestino != '1'
             && opcionDestino != '2');
    if (opcionDestino == CANCELAR) {
        return;
    }
    if (opcionDestino == POR_PANTALLA) {
        F = stdout;
    } else {
        printf("Indicar el identificador del fichero (nombre con extensión "
               ".txt): ");
        lee(nombreFichero, stdin, 1);
        F = fopen(nombreFichero, "w");
        if (F == NULL) {
            printf("ERROR: No es posible abrir el fichero: <%s>\n",
                   nombreFichero);
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
    }
    switch (opcionPresentar) {
    case PENDIENTES:
        if (cantidadDeTareasPendientes(tareas, usuario) == 0) {
            printf("AVISO: El usuario no tiene tareas de ese tipo o no tiene "
                   "tareas.\n");
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
        listaPendientes(tareas, usuario, F);
        break;
    case TERMINADAS:
        if (cantidadDeTareasTerminadas(tareas, usuario) == 0) {
            printf("AVISO: El usuario no tiene tareas de ese tipo o no tiene "
                   "tareas.\n");
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
        listaTerminadas(tareas, usuario, F);
        break;
    case TODAS:
        if (cantidadDeTareas(tareas, usuario) == 0) {
            printf("AVISO: El usuario no tiene tareas de ese tipo o no tiene "
                   "tareas.\n");
            printf("Pulsar ENTER para volver al menú principal del programa:");
            getchar();
            return;
        }
        listaTodas(tareas, usuario, F);
        break;
    case CANCELAR:
        return;
    }
   if (F != stdout) {
       fflush(F);
       fclose(F);
   }
}

void listaPendientes(Tarea **tareas, cadena usuario, FILE *F)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(usuario, ptr_aux->propietario) == 0
                && strcmp(ptr_aux->estado, "pendiente") == 0) {
            debugTareaPorFichero(ptr_aux, F);
            fprintf(F, "\n...................................\n");
        }
        ptr_aux = ptr_aux->siguiente;
    }
}

void listaTerminadas(Tarea **tareas, cadena usuario, FILE *F)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(usuario, ptr_aux->propietario) == 0
                && strcmp(ptr_aux->estado, "terminada") == 0) {
            debugTareaPorFichero(ptr_aux, F);
            fprintf(F, "\n...................................\n");
        }
        ptr_aux = ptr_aux->siguiente;
    }
}

void listaTodas(Tarea **tareas, cadena usuario, FILE *F)
{
    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(usuario, ptr_aux->propietario) == 0) {
            debugTareaPorFichero(ptr_aux, F);
            fprintf(F, "\n...................................\n");
        }
        ptr_aux = ptr_aux->siguiente;
    }
}

int cantidadDeTareasPendientes(Tarea **tareas, cadena usuario)
{
    int contador;
    Tarea *ptr_aux;
    contador = 0;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->estado, "pendiente") == 0
                && strcmp(ptr_aux->propietario, usuario) == 0) {
            ++contador;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return contador;
}

int cantidadDeTareasTerminadas(Tarea **tareas, cadena usuario)
{
    int contador;
    Tarea *ptr_aux;
    contador = 0;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->estado, "terminada") == 0
                && strcmp(ptr_aux->propietario, usuario) == 0) {
            ++contador;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return contador;
}

int cantidadDeTareas(Tarea **tareas, cadena usuario)
{
    int contador;
    Tarea *ptr_aux;
    contador = 0;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        if (strcmp(ptr_aux->propietario, usuario) == 0) {
            ++contador;
        }
        ptr_aux = ptr_aux->siguiente;
    }
    return contador;
}

void guardarCamios(Tarea **tareas)
{
    FILE *F;
    F = fopen(FICHERO_PRINCIPAL, "w");
    if (F == NULL) {
        fprintf(stderr, "Error: No se han podido guardar los cambios\n");
        return;
    }

    Tarea *ptr_aux;
    ptr_aux = *tareas;
    while (ptr_aux != NULL) {
        debugTareaPorFichero(ptr_aux, F);
        fprintf(F, "\n*\n");
        ptr_aux = ptr_aux->siguiente;
    }
    fflush(F);
    fclose(F);
}

int contiene(cadena cad, char c)
{
    int i;
    const int tam = strlen(cad);
    for (i = 0; i < tam; ++i) {
        if (cad[i] == c) {
            return 1;
        }
    }
    return 0;
}

void debugTareaSinDependencias(Tarea *tarea)
{
    puts(tarea->tipo);
    puts(tarea->id);
    puts(tarea->descripcion);
    puts(tarea->propietario);
    puts(tarea->prioridad);
    puts(tarea->estado);
    puts(tarea->plazo);
}

void debugTarea(Tarea *tarea)
{
    Dependencia *iDepNodo;
    puts(tarea->tipo);
    puts(tarea->id);
    puts(tarea->descripcion);
    puts(tarea->propietario);
    puts(tarea->prioridad);
    puts(tarea->estado);
    puts(tarea->plazo);
    iDepNodo = tarea->dependencias;
    if (iDepNodo == NULL) {
        printf("-");
    }
    while (iDepNodo != NULL) {
        printf("%s ", iDepNodo->valor);
        iDepNodo = iDepNodo->siguiente;
    }
}

void debugTareas(Tarea *tareas)
{
    Tarea *iNodo;
    iNodo = tareas;
    while (iNodo != NULL) {
        debugTarea(iNodo);
        puts("\n...................................\n");
        iNodo = iNodo->siguiente;
    }
}

void debugTareaPorFichero(Tarea *tarea, FILE *F)
{
    Dependencia *iDepNodo;
    fprintf(F, "%s\n", tarea->tipo);
    fprintf(F, "%s\n", tarea->id);
    fprintf(F, "%s\n", tarea->descripcion);
    fprintf(F, "%s\n", tarea->propietario);
    fprintf(F, "%s\n", tarea->prioridad);
    fprintf(F, "%s\n", tarea->estado);
    fprintf(F, "%s\n", tarea->plazo);
    iDepNodo = tarea->dependencias;
    if (iDepNodo == NULL) {
        fprintf(F, "-");
        return;
    }
    fprintf(F, "%s", iDepNodo->valor);
    while (iDepNodo != NULL) {
        iDepNodo = iDepNodo->siguiente;
        if (iDepNodo != NULL) {
            fprintf(F, ",%s", iDepNodo->valor);
        }
    }
}

void debugDependencias(Tarea *tarea)
{
    Dependencia *iDepNodo;
    iDepNodo = tarea->dependencias;
    if (iDepNodo == NULL) {
        printf("-");
    }
    while (iDepNodo != NULL) {
        printf("%s ", iDepNodo->valor);
        iDepNodo = iDepNodo->siguiente;
    }
}

void remueveEnter(cadena cad, int nChars)
{
    const int pos = strlen(cad) - nChars;
    cad[pos] = '\0';
}

void lee(cadena cad, FILE *F, int nChars)
{
    fgets(cad, TAM, F);
    remueveEnter(cad, nChars);
}

int cargaTareas(Tarea **tareas)
{
    FILE *F;
    F = fopen(FICHERO_PRINCIPAL, "r");
    if (F == NULL) {
        return 0;
    }

    const Tarea valorDeReinicio = { "", "", "", "", "", "", "", NULL, NULL };
    cadena linea;
    Tarea *ptr_aux;
    Tarea T;

    T = valorDeReinicio;
    while (!feof(F)) {
        fgets(linea, TAM, F);
        if (!contiene(linea, '*')) { // no es asterisco
            remueveEnter(linea, 1);
            if (strcmp(T.tipo, "") == 0) {
                strcpy(T.tipo, linea);
            } else if (strcmp(T.id, "") == 0) {
                strcpy(T.id, linea);
            } else if (strcmp(T.descripcion, "") == 0) {
                strcpy(T.descripcion, linea);
            } else if (strcmp(T.propietario, "") == 0) {
                strcpy(T.propietario, linea);
            } else if (strcmp(T.prioridad, "") == 0) {
                strcpy(T.prioridad, linea);
            } else if (strcmp(T.estado, "") == 0) {
                strcpy(T.estado, linea);
            } else if (strcmp(T.plazo, "") == 0) {
                strcpy(T.plazo, linea);
            } else if (T.dependencias == NULL) {
                asignaDependencias(&T, linea);
                // agregala a la lista y reiniciala (la tarea)
                if (*tareas == NULL) {
                    // directamente
                    *tareas = (Tarea *) malloc(sizeof(Tarea));
                    ptr_aux = *tareas;
                    *ptr_aux = T;
                } else {
                    // hasta el final
                    ptr_aux = *tareas;
                    while (ptr_aux->siguiente != NULL) {
                        ptr_aux = ptr_aux->siguiente;
                    }
                    ptr_aux->siguiente = (Tarea *) malloc(sizeof(Tarea));
                    ptr_aux = ptr_aux->siguiente;
                    *ptr_aux = T;
                }
                T = valorDeReinicio;
            }
        }
    }
    fclose(F);
    return 1;
}

int asignaDependencias(Tarea *tarea, cadena dependencias)
{
    if (atoi(dependencias) == 0) {
        tarea->dependencias = NULL;
        return 0;
    }

    char *token;
    Dependencia *ptr_aux;
    cadena copiaDependencias;

    strcpy(copiaDependencias, dependencias);
    tarea->dependencias = NULL;
    token = strtok(copiaDependencias, ",");
    if (token == NULL) {
        return 0;
    }
    tarea->dependencias = (Dependencia *) malloc(sizeof(Dependencia));
    ptr_aux = tarea->dependencias;
    strcpy(ptr_aux->valor, token);
    ptr_aux->siguiente = NULL;
    while (token != NULL) {
        token = strtok(NULL, ",");
        if (token != NULL) {
            ptr_aux->siguiente = (Dependencia *) malloc(sizeof(Dependencia));
            ptr_aux = ptr_aux->siguiente;
            strcpy(ptr_aux->valor, token);
            ptr_aux->siguiente = NULL;
        }
    }
    return 1;
}

void imprimeMenu(void)
{
    printf("\tAgenda:\n");
    printf("\t1)  Crear tarea.\n");
    printf("\t2)  Ver tarea.\n");
    printf("\t3)  Ver lista de tareas.\n");
    printf("\t0)  Salir del programa.\n");
    printf("\n\tSiguiente operacion?\n");
}

void asteriscos(int lineas, int asteriscos)
{
    int i, k;
    for (i = 0; i < lineas; ++i) {
        for (k = 0; k < asteriscos; ++k) {
            printf("*");
        }
        printf("\n");
    }
}

void imprimeCabecera(void)
{
    int i;
    const int num_lineas = 2;
    const int num_asteriscos = 61;

    asteriscos(num_lineas, num_asteriscos);
    for (i = 0; i < 15; ++i) {
        printf("*");
    }
    printf(" GESTOR DE TAREAS DE UNA AGENDA ");
    for (i = 0; i < 14; ++i) {
        printf("*");
    }
    printf("\n");
    asteriscos(num_lineas, num_asteriscos);
}

int diasPara(cadena fecha)
{
    char c;
    int anyoDias;
    int restoDias1;
    int restoDias2;
    int diasTotales;
    int dia1, mes1, anyo1, hora1, min1;
    int dia2, mes2, anyo2, hora2, min2;
    cadena fechaActual;

    sTiempoActualPrint(fechaActual);
    sscanf(fechaActual, "%d%c%d%c%d%c%d%c%d", &dia1, &c, &mes1, &c, &anyo1, &c,
           &hora1, &c, &min1);
    sscanf(fecha, "%d%c%d%c%d%c%d%c%d", &dia2, &c, &mes2, &c, &anyo2, &c,
           &hora2, &c, &min2);
    anyoDias = diferenciaDeAnyos(anyo1, anyo2);
    restoDias1 = diasAnyo1(dia1, mes1);
    restoDias2 = diasAnyoDos(dia2, mes2);
    diasTotales = anyoDias + (restoDias2 - restoDias1);
    return diasTotales;
}

int diferenciaDeAnyos(int a1, int a2)
{
    int i;
    int aniodif;
    int cta;
    int aniodias;

    aniodif = a2 - a1; // Para sacar la diferencia de anios
    // Para contar los anios bisiestos en un intervalo de fechas:
    cta = 0; // Levara la cuenta de anios bisiestos
//    Recorrera del a1 al a2 comprobando si existen anios bisiestos:
//    los anios bisiestos son divisibles por 4 y 400, excluyendo los que sean
//    divisibles por 10
    for (i = a1; i <= a2; i++) {
        if ((i % 4 == 0) && (i % 100 != 0 || i % 400 == 0)) {
            cta = cta + 1;
        }
    }
//  La difrerencia de anios multiplicada por 365 mas la cuenta de numero de
//  anios bisiestos nos dara el numero de dias que har entre los anios
//  evaluados
    aniodias = (aniodif * 365) + cta;
    return aniodias;
}

//Al primer anio se le quitaran los dias del intervalo del primer dia del anio
//a la primer fecha ingresada
int diasAnyo1(int d1, int me1)
{
    int m1;
    int restodias1;
//    Cada numero de mes tiene asignado numeros pre calculados ya que no todos
//    los meses tienen el mismo numero de dias.
//    Se empieza en el dia 1 y aumenta de acuerdo al numero de dias que tiene
//    cada mes hasta completar el anio
    switch (me1) {
    case 1:
        m1 = 0;
        break;
    case 2:
        m1 = 31;
        break;
    case 3:
        m1 = 59;
        break;
    case 4:
        m1 = 90;
        break;
    case 5:
        m1 = 120;
        break;
    case 6:
        m1 = 151;
        break;
    case 7:
        m1 = 181;
        break;
    case 8:
        m1 = 212;
        break;
    case 9:
        m1 = 243;
        break;
    case 10:
        m1 = 273;
        break;
    case 11:
        m1 = 304;
        break;
    case 12:
        m1 = 334;
        break;
    }

//    Dependiendo del mes asignado sus dias se sumaran con los dias ingresados
//    de la primera fecha
    restodias1 = m1 + d1;
    // Devuelve el valor que se pretendia calcular
    return restodias1;
}

// Al segundo anio se le quitaran los dias del intervalo de la segunda fecha
// ingresada hasta el ultimo dia de ese mismo anio
int diasAnyoDos(int d2, int me2)
{
    int m2, restodias2;
//    Cada numero de mes tiene asignado numeros precalculados ya que no todos
//    los meses tienen el mismo numero de dias.
//    Se empieza en el dia 1 y aumenta de acuerdo al numero de dias que tiene
//    cada mes hasta completar el anio
    switch (me2) {
    case 1:
        m2 = 0;
        break;
    case 2:
        m2 = 31;
        break;
    case 3:
        m2 = 59;
        break;
    case 4:
        m2 = 90;
        break;
    case 5:
        m2 = 120;
        break;
    case 6:
        m2 = 151;
        break;
    case 7:
        m2 = 181;
        break;
    case 8:
        m2 = 212;
        break;
    case 9:
        m2 = 243;
        break;
    case 10:
        m2 = 273;
        break;
    case 11:
        m2 = 304;
        break;
    case 12:
        m2 = 334;
        break;
    }
    restodias2 = m2 + d2;
    // devuelve el valor que se pretendia calcular
    return restodias2;
}
