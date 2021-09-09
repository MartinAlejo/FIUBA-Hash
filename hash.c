#define _POSIX_C_SOURCE 200809L
#include "hash.h"
#include "lista.h"
#include <stdlib.h>
#include <string.h>
#include<stdio.h>

#define LARGO_INICIAL 37
#define CARGA_MIN 1.0
#define CARGA_MAX 3.0
#define FACTOR_REDIMENSION 2

typedef struct campo_hash {
	char *clave;
	void *dato;
} campo_hash_t;

struct hash {
	lista_t **tabla;
	size_t cantidad;
	size_t largo;
	hash_destruir_dato_t destruir_dato;
};

// Hay que pasarle el string y la longitud del mismo
size_t hashing(const char *string, size_t length) {	
	// (FNV) Hash
	static const size_t InitialFNV = 2166136261U;
	static const size_t FNVMultiple = 16777619;
	size_t hash = InitialFNV;
	for(size_t i = 0; i < length; i++) {
	    //XOR the lower 8 bits
	    hash = hash ^ (string[i]);

	    //Multiply by the multiple
	    hash = hash * FNVMultiple;
	}
	return hash;
}

// Busca la clave en el hash, y devuelve un iterador lista, parado en la posicion del campo pertinente
// Si existe, devuelve un iterador de lista posicionado en el campo de la clave
// Si no existe, devuelve un NULL 
lista_iter_t *buscar_clave(const hash_t *hash, const char *clave) {
	size_t pos = hashing(clave, strlen(clave)) % hash->largo;
	lista_iter_t *iter = lista_iter_crear(hash->tabla[pos]);

    while (!lista_iter_al_final(iter)) {
		campo_hash_t *campo_actual = lista_iter_ver_actual(iter);

		if (!strcmp(campo_actual->clave, clave)) {
			return iter;
		}
		lista_iter_avanzar(iter);
	}
	// No se encontro la clave
	lista_iter_destruir(iter);

    return NULL;
}

// Redimensiona la lista dado su nuevo largo (calculado a traves de la funcion "calcular_largo_redimension")
// Devuelve true si se redimensiono correctamente, caso contrario devuelve false
bool redimensionar(hash_t *hash, size_t nuevo_largo) {
	lista_t **nueva_tabla = calloc(nuevo_largo, sizeof(lista_t*));
	if (!nueva_tabla) {
		return false;
	}

	for (size_t i = 0; i < nuevo_largo; i++){
        nueva_tabla[i] = lista_crear();
    }

	for (int i = 0; i < hash->largo; i++) {
		lista_iter_t *iter = lista_iter_crear(hash->tabla[i]);
		while (!lista_iter_al_final(iter)) {
			campo_hash_t *campo_actual = lista_iter_ver_actual(iter);
			size_t nueva_pos = hashing(campo_actual->clave, strlen(campo_actual->clave)) % nuevo_largo;

			campo_actual->clave = campo_actual->clave;
			campo_actual->dato = campo_actual->dato;
			lista_insertar_ultimo(nueva_tabla[nueva_pos], campo_actual);

			lista_iter_avanzar(iter);
		}
		lista_iter_destruir(iter);
		lista_destruir(hash->tabla[i], NULL);
	}
	free(hash->tabla);
	hash->largo = nuevo_largo;
	hash->tabla = nueva_tabla;

	return true;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
	hash_t *hash = malloc(sizeof(hash_t));
    if (!hash){
        return NULL;
    }

    hash->tabla = calloc(LARGO_INICIAL, sizeof(lista_t*));
    if (!hash->tabla){
        free(hash);
        return NULL;
    }

    for (size_t i = 0; i < LARGO_INICIAL; i++){
        hash->tabla[i] = lista_crear();
        if (!hash->tabla[i]) {
        	free(hash->tabla);
        	free(hash);
        	return NULL;
        }
    }

    hash->cantidad = 0;
    hash->largo = LARGO_INICIAL;
    hash->destruir_dato = destruir_dato;

    return hash;
}

size_t hash_cantidad(const hash_t *hash) {
	return hash->cantidad;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato) {
	lista_iter_t *iterador = buscar_clave(hash, clave);

    if (iterador) {
    	// La clave ya existia, reemplazamos su dato asociado
        campo_hash_t *campo_actual = lista_iter_ver_actual(iterador);
        if (hash->destruir_dato) {
			hash->destruir_dato(campo_actual->dato);
		}
		campo_actual->dato = dato;
        lista_iter_destruir(iterador);

        return true;
    }

    // La clave no existia, creamos un nuevo campo
	campo_hash_t *nuevo_campo = malloc(sizeof(campo_hash_t));
	if (!nuevo_campo) {
		return false;
	}

	nuevo_campo->clave = strdup(clave);
	nuevo_campo->dato = dato;

	size_t pos = hashing(clave, strlen(clave)) % hash->largo;
	lista_insertar_ultimo(hash->tabla[pos], nuevo_campo);
	hash->cantidad++;
	
	// Vemos si hace falta redimensionar
	float factor_carga = (float)(hash->cantidad / hash->largo); 
	if (factor_carga > CARGA_MAX) {
		// Redimensionamos hacia arriba (en promedio hay mas de "CARGA_MAX" campos por lista)
		size_t nuevo_largo = hash->largo * FACTOR_REDIMENSION;
		redimensionar(hash, nuevo_largo);
	}
	
	return true;
}

void *hash_borrar(hash_t *hash, const char *clave) {
	lista_iter_t *iterador = buscar_clave(hash, clave);
	void *dato_aux;

	if (!iterador) return NULL; // La clave no existe, no hay nada que borrar

	// La clave existe, borramos
    campo_hash_t *campo_actual = lista_iter_ver_actual(iterador);
    dato_aux = campo_actual->dato; // Encontro la clave, guardamos el dato
    free(campo_actual->clave);
	free(campo_actual); // Elimina el campo
	lista_iter_borrar(iterador); // Eliminamos el nodo
    lista_iter_destruir(iterador); // Eliminamos el iterador
    hash->cantidad--;
        
	// Vemos si hace falta redimensionar
	float factor_carga = (float)(hash->cantidad / hash->largo); 
	if (factor_carga < CARGA_MIN) {
		// Redimensionamos hacia abajo (en promedio hay menos de "CARGA_MIN" campos por lista)
		// Pero solo si el "nuevo_largo" es mayor que el "LARGO_INICIAL"
		size_t nuevo_largo = hash->largo / FACTOR_REDIMENSION;
		if (nuevo_largo > LARGO_INICIAL) {
			redimensionar(hash, nuevo_largo);
		}
	}
		
	return dato_aux;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    lista_iter_t *iter = buscar_clave(hash, clave);
    if (!iter) return false; // No se encontro la clave

    // En este caso, el iter se creo y encontro la clave
    lista_iter_destruir(iter); 

    return true;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    lista_iter_t *iter = buscar_clave(hash, clave);

    if (iter) {
    	void *dato_aux = ((campo_hash_t*)lista_iter_ver_actual(iter))->dato;
    	lista_iter_destruir(iter);

    	return dato_aux;
    }

    return NULL; // No se pudo obtener el dato (la clave no existe)
}

void hash_destruir(hash_t *hash) {
	for (int i = 0; i < hash->largo; i++) {
		lista_iter_t *iterador = lista_iter_crear(hash->tabla[i]);
		while (!lista_iter_al_final(iterador)) {
			campo_hash_t *campo_actual = lista_iter_ver_actual(iterador);
			if (hash->destruir_dato) {
				hash->destruir_dato(campo_actual->dato);
			}
			free(campo_actual->clave);
			free(campo_actual);
			lista_iter_avanzar(iterador);
		}
		lista_iter_destruir(iterador);
		lista_destruir(hash->tabla[i], NULL);
	}
	free(hash->tabla);
	free(hash);
}

/* Iterador del hash */

struct hash_iter {
    const hash_t *hash;
    size_t indice;
    lista_iter_t *iter_actual;
};

// Crea iterador
hash_iter_t *hash_iter_crear(const hash_t *hash){
	hash_iter_t* iter_hash = malloc(sizeof(hash_iter_t));
	if (!iter_hash) {
        return NULL;
    }
    iter_hash->hash = hash;
	bool ok = false;
	size_t i = 0;
	while (i < hash->largo){
		if(!lista_esta_vacia(hash->tabla[i])){
			iter_hash->indice = i;
			ok = true;
			break;
		}
		i++;
	}
	if(!ok){
		iter_hash->indice = hash->largo;
		iter_hash->iter_actual = NULL;
	}else{
		lista_iter_t *iter_lista = lista_iter_crear(hash->tabla[iter_hash->indice]);
		if (!iter_lista) {
			return NULL;
		}
		iter_hash->iter_actual = iter_lista; 
	}
	return iter_hash;
}

// Avanza iterador
bool hash_iter_avanzar(hash_iter_t *iter) {
	if (iter->indice >= iter->hash->largo) { 
		return false; // No hay mas listas por iterar
	}

	lista_iter_avanzar(iter->iter_actual); // El iterador de la lista actual no esta al final, lo avanzamos
	if (!lista_iter_al_final(iter->iter_actual)) {
		return true;
	}
	
	// La lista llego al final, por lo tanto hay que pasar a la siguiente lista
	iter->indice++; // El iterador de la lista actual esta al final, hay que pasar a la siguiente lista
	if (iter->indice >= iter->hash->largo) {
		return false; // Termino de iterar por todas las listas
	}

	lista_iter_destruir(iter->iter_actual); // Destruimos el iterador de la lista anterior
	lista_iter_t *iter_lista = lista_iter_crear(iter->hash->tabla[iter->indice]); // Creamos el iterador para la nueva lista

	while (lista_iter_al_final(iter_lista)) {
		// Se creo el iterador en una lista vacia
		iter->indice++;
		if (iter->indice >= iter->hash->largo) {
			iter->iter_actual = iter_lista;	
			return false; // Termino de iterar por todas las listas
		}
		lista_iter_destruir(iter_lista);
		iter_lista = lista_iter_crear(iter->hash->tabla[iter->indice]);
	}
		
	if (!iter_lista) {
		return false;
	}
	iter->iter_actual = iter_lista;	
	
	return true;
}


// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t *iter){
	return iter->indice >= iter->hash->largo;
}

// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char *hash_iter_ver_actual(const hash_iter_t *iter){
	return hash_iter_al_final(iter) ? NULL : ((campo_hash_t*)lista_iter_ver_actual(iter->iter_actual))->clave;
}


// Destruye iterador
void hash_iter_destruir(hash_iter_t *iter) {
	if (iter->iter_actual) lista_iter_destruir(iter->iter_actual);
	free(iter);
}

/* Pruebas generales internas */

/*
void prueba_funcion_hashing(void) {
	char *palabra = "Hola";
	printf("Aplicando funcion de hash: %lu\n", hashing(palabra, strlen(palabra)) % LARGO_INICIAL);
}


void pruebas_generales(void) {
	hash_t *hash = hash_crear(free);
	int *dato1 = malloc(sizeof(int));
	*dato1 = 3;
	int *dato2 = malloc(sizeof(int));
	*dato2 = 5;
	int *dato3 = malloc(sizeof(int));
	*dato3 = 1;

	hash_guardar(hash,"Pedro",dato1);
	printf("Pertenece Pedro al hash: %d\n", hash_pertenece(hash,"Pedro"));

	printf("Pertenece Abigail al hash: %d\n", hash_pertenece(hash,"Abigail"));
	hash_guardar(hash,"Abigail",dato2);
	printf("Pertenece Abigail al hash: %d\n", hash_pertenece(hash,"Abigail"));

	printf("Lo que habia en Pedro: %d\n", *(int*)hash_obtener(hash,"Pedro"));

	printf("Lo que habia en Abigail: %d\n", *(int*)hash_obtener(hash,"Abigail"));

	printf("Lo que habia en una clave inexistente (Jorge): %d\n", (int)hash_obtener(hash,"Jorge")); // Tira warning porque piensa que es un puntero, pero en realidad es un NULL. Deberia devolver 0
	hash_guardar(hash,"Jorge",dato3);
	printf("Pertenece Jorge al hash (despues de guardar): %d\n", hash_pertenece(hash,"Jorge"));
	printf("Lo que hay en Jorge (despues de guardar): %d\n", *(int*)hash_obtener(hash,"Jorge"));
	void *dato_jorge = hash_borrar(hash,"Jorge");
	printf("Pertenece Jorge al hash (despues de borrar): %d\n", hash_pertenece(hash,"Jorge"));
	printf("Lo que hay en Jorge (despues de borrar): %d\n", (int)hash_obtener(hash,"Jorge")); // Tira warning porque piensa que es un puntero, pero en realidad es un NULL. Deberia devolver 0
	printf("Lo que habia en Jorge: %d\n", *(int*)dato_jorge);
	
	int *dato5 = malloc(sizeof(int));
	*dato5 = 1;
	int *dato6 = malloc(sizeof(int));
	*dato6 = 6;
	hash_guardar(hash,"Maria",dato5);
	printf("Lo que hay en Maria: %d\n", *(int*)hash_obtener(hash,"Maria"));
	printf("La cantidad de elementos que hay hasta ahora: %zu\n", hash_cantidad(hash));
	hash_guardar(hash,"Maria",dato6);
	printf("Lo que hay en Maria (despues de reemplazar): %d\n", *(int*)hash_obtener(hash,"Maria"));
	printf("La cantidad de elementos que hay hasta ahora: %zu\n", hash_cantidad(hash)); // No deberia cambiar ya que reemplazamos

	free(dato_jorge);
	hash_destruir(hash);
}

int main(void) {
	pruebas_generales();
	return 0;
}
*/