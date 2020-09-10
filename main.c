/**
 * Autor - Esteban Manrique de Lara Sirvent
 * Fecha - 10/09/2020
 * Actividad 4: Procesos
 *
*/
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>

struct elementoHistograma //Estructura utilziada para almacenar los elementos a imprimir en el histograma
{
    int pidHijo;
    int promedio;
};

void numeroProcesos(int*, int, char* const*);
void accionesProcesos(int*, struct elementoHistograma*);
void escribirPromedio(int*, int);
int leerPromedio(int*);
void imprimirHistograma(struct elementoHistograma*, int, int);

int main(int argc, char* const* argv)
{
    int* numeroHijos; //Numero de procesos a crearse. Determinado por usuario
    numeroHijos = (int*)malloc(sizeof(int)); 
    *numeroHijos = -1; //Valor por default
    numeroProcesos(numeroHijos, argc, argv); //Se introduce el numero de procesos como parametro -n
    if (*(numeroHijos) == -1) //Si la introduccion no es valida, se imprime un mensaje de cierre de programa y se termina
    {
        printf("Acabando el programa.\n");
        free(numeroHijos);
        return 1; //Acaba programa si numero de Procesos hijos a crearse fue invalido
    }
    struct elementoHistograma* histograma; //Aqui se almacenara toda la informacion del histograma
    histograma = (struct elementoHistograma*)malloc(*(numeroHijos) *  sizeof(struct elementoHistograma));
    accionesProcesos(numeroHijos, histograma); //Creacion de hijos, impresiones de promedios, almacenamiento en histograma y mensaje en caso de pid == -1
    free(numeroHijos); //Libera variable de numero de hijos
    free(histograma); //Libera memoria de histograma
    return 0;
}
/**
 * Funcion encargada de recibir y analizar el numero de procesos hijos a ser creados, el cual se introduce como paramtero de -n.
 * 
 * @param numeroHij, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear. Si no es entera, manda error y termina programa
 * @param numeroArgumentos, recibe la variable argc del metodo main()
 * @param sizeArca, recibe la variable argv del metodo main()
 * 
 **/ 
void numeroProcesos(int* numeroHij, int numeroArgumentos, char* const* argumentos)
{
    int aux;
    int numberDigits;
    while((aux = getopt(numeroArgumentos, argumentos, "n:")) != -1)
    {
        switch (aux)
        {
        case 'n':
            numberDigits = (atoi(optarg) == 0) ? 1  : (log10((atoi(optarg))) + 1); //Checa que opcion introducida no tenga letras y numeros mezclados (ej. 9hola)
            if(atoi(optarg)>0 && strstr(optarg, ".") == NULL && strlen(optarg) == numberDigits) //Checa que numero de procesos sea mayor a 0 y NO DECIMAL
            {
                *(numeroHij) = atoi(optarg);
                printf("%d\n", *(numeroHij));
                return;
            }
            else
            {
                printf("Se ha introducido no ENTERO o MENOR/IGUAL a CERO.\n"); //Se introdujo un valor No Entero o menor/igual a 0
                return;
            }
            break;

        case '?': //Opciones en caso de recepcion de opciones invalidas o de -n sin argumento
            if (optopt == 'n')
            {
                fprintf (stderr, "Opción -%c requiere un argumento.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Opción desconocida '-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Opción desconocida '\\x%x'.\n",optopt);
            }
            break;

        default:
            return;
        }
    }
    return;
}

/**
 * Funcion encargada de crear procesos hijos mediante ciclo, ir obteniendo promedios, llamar funcion para imprimir histograma y checar que fork() funcione correctamente
 * 
 * @param cantidadHijos, Apuntador a variable ubicada en main(), encargada de almacenar el numero de procesos hijos a crear.
 * @param histograma, la cual es arreglo en donde se almacenaran pid y promedios de los procesos hijos a ser impresos en el histograma
 * 
 **/ 
void accionesProcesos(int* cantidadHijos, struct elementoHistograma* histograma)
{
    int promedioMasAlto = 0; //Variable auxliar a la hora de escalar el numero de asteriscos a ser impresos por el histograma
    pid_t pid;
    struct elementoHistograma* aux = histograma;
    for(int i = 0; i<*(cantidadHijos);i++) //Ciclo que crea numero de procesos hijos determinado por usuario
    {
        int promedio;
        int tuberia[2]; //Arreglo para pipe
        pipe(tuberia); //Se crea pipe para pasar entre procesos promedios que superen el valor de 255
        pid = fork(); //Generacion de procesos hijos
        aux->pidHijo = pid;
        if(pid == -1) //Falla de fork()
        {
            printf("Se ha detectado un error en proceso de creacion. No habra mas procesos.\n");
            printf("Se han creado %d procesos.\n", i);
            *cantidadHijos = i;
            break; //Se acaba el ciclo y la funcion
        }
        else if(pid == 0) //Fork funciona correctamente
        {
            sleep(1);
            promedio = (getpid() + getppid()) / 2; //Promedio entre pid de hijo y de apdre
            printf("Soy el proceso hijo con PID %d, con PPID %d y mi promedio es = %d.\n", getpid(), getppid() ,promedio);
            escribirPromedio(tuberia, promedio); //Se escribe en Pipe para pasar promedio a proceso padre
            exit(promedio);
        }  
        aux->promedio = leerPromedio(tuberia);
        aux++;
    }
    aux = histograma;
    for(int i = 0; i<*(cantidadHijos);i++) //Proceso padre va acabando a los procesos hijos
    {
        int estado;
        if (waitpid(aux->pidHijo, &estado, 0) != -1) //Si padre esta esperando a hijo
        {
            if(aux->promedio>promedioMasAlto)
            {
                promedioMasAlto = aux->promedio; //Sirve para escala a la hora de imprimir asteriscos en el histograma.
            }
            if (WIFEXITED(estado))
            {
                printf("Ya termino el hijo con PID %d con valor de retorno (promedio) %d \n", aux->pidHijo, aux->promedio); //Hijo acaba de terminar
            }
        }
        aux++;            
    }
    imprimirHistograma(histograma, *(cantidadHijos), promedioMasAlto); //Se imprime histograma cuando se han creado y culimado TODOS los procesos hijos solicitados
    return;
}

/**
 * Funcion encargada de escribir en pipe el promedio recien calculado para que proceso padre pueda leerlo e introducirlo en arreglo para ser impreso en histograma
 *  
 * @param fd, Arreglo utilizado por el pipe.
 * @param promedio, valor de promedio a ser escrito en el pipe por parte de este metodo
 * 
 **/ 
void escribirPromedio(int* fd, int promedio)
{
    close(fd[0]); //Se cierra extremo de lectura
    write(fd[1], &promedio, sizeof(int)); //Promedio es escrito en el pipe
    return;
}

/**
 * Funcion encargada de permitir la lectura, por parte del proceso padre, del promedio recien obtenido por el proceso hijo
 *  
 * @param fd, Arreglo utilizado por el pipe.
 * 
 * return promedio, el cual pasa a ser incluido en el elemento del histograma correspondiente
 * 
 **/
int leerPromedio(int* fd)
{
    int promedio;
    close(fd[1]); //Se cierra extremo de escritura
    read(fd[0], &promedio, sizeof(int)); //Promedio es leido
    return promedio;
}

/**
 * Funcion encargada de imprimir el histograma que contiene pid y promedios de los procesos hijos creados previamente
 * 
 * @param histograma, Arreglo el cual contiene pid y promedios a ser impresos en el histograma
 * @param procesos, numero de procesos hijos generados por el programa previamente
 * @param promedioMasAlto, valor que sirve para determina rproporcionalidad de asteriscos a ser impresos en el histograma
 **/
void imprimirHistograma(struct elementoHistograma* histograma, int procesos, int promedioMasAlto)
{
    struct elementoHistograma* aux = histograma;
    printf("%10s %10s %13s\n", "PID hijo", "Promedio", "Histograma");
    for(; aux<(histograma + procesos); aux++)
    {
        int numeroAsteriscos = aux->promedio;
        printf("%8d  %8d       ", aux->pidHijo, aux->promedio);
        if(promedioMasAlto>100 && promedioMasAlto<1000) //Promedio mas alto entre 100 y 1000
        {
            numeroAsteriscos = (aux->promedio)/10; //Proporcion 1 * a 10
        }
        else if(promedioMasAlto>=1000) //Promedio mas alto igual o mayor a 1000
        {
            numeroAsteriscos = (aux->promedio)/40; //Proporcion 1 * a 40
        }
        for (int count = 0; count <numeroAsteriscos; count++)
        {
            printf("*");
        }
        printf("\n");
    }
}