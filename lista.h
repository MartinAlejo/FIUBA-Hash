#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* La lista está planteada como una lista de punteros genéricos. */

struct lista;
typedef struct lista lista_t;

/* El iterador externo. */

struct lista_iter;
typedef struct lista_iter lista_iter_t;

/* ******************************************************************
 *                    PRIMITIVAS DE LA COLA
 * *****************************************************************/

// Crea una lista.
// Post: devuelve una nueva lista vacía.
lista_t *lista_crear(void);

// Devuelve verdadero si la lista no tiene elementos encolados, false en caso contrario.
// Pre: la lista fue creada.
bool lista_esta_vacia(const lista_t *lista);

// Agrega un elemento al principio de lista. Devuelve false en caso de error.
// Pre: la lista fue creada.
// Post: se agregó un nuevo elemento al principio de la lista.
bool lista_insertar_primero(lista_t *lista, void *dato);

// Agrega un elemento al final de lista. Devuelve false en caso de error.
// Pre: la lista fue creada.
// Post: se agregó un nuevo elemento al final de la lista.
bool lista_insertar_ultimo(lista_t *lista, void *dato);

// Elimina el primer elemento de la lista. Si la lista está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se eliminó y devolvió el valor del primer elemento de la lista.
void *lista_borrar_primero(lista_t *lista);

// Obtiene el valor del primer elemento de la lista. Si la lista tiene
// elementos, se devuelve el valor del primero, si está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el valor del primer elemento de la lista, cuando no está vacía.
void *lista_ver_primero(const lista_t *lista);

// Obtiene el valor del ultimo elemento de la lista. Si la lista tiene
// elementos, se devuelve el valor del ultimo, si está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el valor del ultimo elemento de la lista, cuando no está vacía.
void *lista_ver_ultimo(const lista_t* lista);

// Obtiene el valor del largo de la lista.
// Pre: la lista fue creada.
// Post: devuelve la cantidad de elementos en la lista, 0 si está vacía.
size_t lista_largo(const lista_t *lista);

// Destruye la lista. Si se recibe la función destruir_dato por parámetro,
// para cada uno de los elementos de la lista llama a destruir_dato.
// Pre: la lista fue creada. destruir_dato es una función capaz de destruir
// los datos de la cola, o NULL en caso de que no se la utilice.
// Post: se eliminaron todos los elementos de la lista.
void lista_destruir(lista_t *lista, void (*destruir_dato)(void *));

/* *****************************************************************
 *                 PRIMITIVAS DEL ITERADOR INTERNO
 * *****************************************************************/

// Itera todos los elementos de la lista, aplicandole la funcion "visitar" a cada elemento, cuando
// esta devuelve false, se deja de iterar. Se tiene por parametro un "extra", el cual puede ser NULL.
// Pre: la lista fue creada.
// Post: aplica la funcion recibida a los elementos de la lista hasta devolver false, o hasta el final. 
void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra);

/* *****************************************************************
 *                 PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

// Crea el iterador externo para la lista.
// Pre: la lista fue creada.
// Post: se devolvió el iterador para la lista.
lista_iter_t *lista_iter_crear(lista_t *lista);

// Avanza el iterador al siguiente elemento de la lista. 
// Pre: el iterador fue creado.
// Post: avanza el iterador al siguiente elemento, devuelve false si esta al final, true si pudo avanzar. 
bool lista_iter_avanzar(lista_iter_t *iter);

// Devuelve el elemento sobre el que esta parado el iterador.
// Pre: el iterador fue creado.
// Post: se devuelve el valor del elemento sobre el que esta parado el iterador
void *lista_iter_ver_actual(const lista_iter_t *iter);

// Devuelve true si el iterador esta parado en el ultimo elemento de la lista, false en caso contrario.
// Pre: el iterador fue creado.
// Post: se devuelve true si el iterador esta al final, false en caso contrario.
bool lista_iter_al_final(const lista_iter_t *iter);

// Destruye el iterador.
// Pre: el iterador fue creado.
// Post: el iterador fue destruido.
void lista_iter_destruir(lista_iter_t *iter);

// Inserta un elemento donde esta parado el iterador.
// Pre: el iterador fue creado.
// Post: en la posicion donde estaba el iterador se creo un nuevo elemento. Devuelve true si se
// pudo crear, false en caso contrario.
bool lista_iter_insertar(lista_iter_t *iter, void *dato);

// Borra el elemento donde esta parado el iterador.
// Pre: el iterador fue creado.
// Post: se elimina el elemento en la posicion donde esta parado el iterador, y lo devuelve.
void *lista_iter_borrar(lista_iter_t *iter);

/* *****************************************************************
 *                      PRUEBAS UNITARIAS
 * *****************************************************************/

// Realiza pruebas sobre la implementación realizada.
//
// Las pruebas deben emplazarse en el archivo ‘pruebas_lista.c’, y
// solamente pueden emplear la interfaz pública tal y como aparece en lista.h
// (esto es, las pruebas no pueden acceder a los miembros del struct lista).
//
// Para la implementación de las pruebas se debe emplear la función
// print_test(), como se ha visto en TPs anteriores.
void pruebas_lista_estudiante(void);

#endif  // LISTA_H
