#include "lista.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

typedef struct nodo {
    void *dato;
    struct nodo *proximo;
} nodo_t;

nodo_t *crear_nodo(void *elem) {
    nodo_t *nodo = malloc(sizeof(nodo_t));
    if (!nodo) {
        return NULL;
    }
    nodo->dato = elem;
    nodo->proximo = NULL;
    return nodo;
}

struct lista {
	nodo_t *primero;
	nodo_t *ultimo;
	size_t largo;
};

lista_t *lista_crear(void) {
	lista_t *lista = malloc(sizeof(lista_t));
	if (!lista) {
		return NULL;
	}
	lista->primero = NULL;
	lista->ultimo = NULL;
	lista->largo = 0;
	return lista;
}

bool lista_esta_vacia(const lista_t *lista) {
	return lista->largo == 0;
}

bool lista_insertar_primero(lista_t *lista, void *dato) {
	nodo_t *nodo_aux = crear_nodo(dato);
	if (!nodo_aux) {
		return false;
	}
	if (lista_esta_vacia(lista)) {
		lista->ultimo = nodo_aux;
	}
	else {
		nodo_aux->proximo = lista->primero;
	}
	lista->primero = nodo_aux;
	lista->largo++;
	return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato) {
	nodo_t *nodo_aux = crear_nodo(dato);
	if (!nodo_aux) {
		return false;
	}
	if (lista_esta_vacia(lista)) {
		lista->primero = nodo_aux;
	}
	else {
		lista->ultimo->proximo = nodo_aux;
	}
	lista->ultimo = nodo_aux;
	lista->largo++;
	return true;	
}

void *lista_borrar_primero(lista_t *lista) {
	if (lista_esta_vacia(lista)) {
		return NULL;
	}
	nodo_t *ant_primero = lista->primero;
	void *dato_ant_primero = lista->primero->dato;
	lista->primero = lista->primero->proximo;
	if (lista->primero == NULL) {
		lista->ultimo = NULL;
	}
	free(ant_primero);
	lista->largo--;
	return dato_ant_primero;
}

void *lista_ver_primero(const lista_t *lista) {
	return lista_esta_vacia(lista) ? NULL : lista->primero->dato;
}

void *lista_ver_ultimo(const lista_t* lista) {
	return lista_esta_vacia(lista) ? NULL : lista->ultimo->dato;
}

size_t lista_largo(const lista_t *lista) {
	return lista->largo;
}

void lista_destruir(lista_t *lista, void (*destruir_dato)(void *)) {
	while (!lista_esta_vacia(lista)) {
		void *dato_ant_prim = lista_borrar_primero(lista);
		if (destruir_dato) {
			destruir_dato(dato_ant_prim);
		}
	}
	free(lista);
}

void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra) {
	nodo_t *act = lista->primero;
	while (act && visitar(act->dato, extra)) {
		act = act->proximo;
	}
}

struct lista_iter {
	lista_t *lista;
	nodo_t *anterior;
	nodo_t *actual;
};

lista_iter_t *lista_iter_crear(lista_t *lista) {
	lista_iter_t *iter = malloc(sizeof(lista_iter_t));
	if (!iter) {
		return NULL;
	}
	iter->lista = lista;
	iter->anterior = NULL;
	iter->actual = lista->primero;
	return iter;
}

bool lista_iter_avanzar(lista_iter_t *iter) {
	if (!iter->actual) {
		return false;
	} 
	iter->anterior = iter->actual;
	iter->actual = iter->actual->proximo;
	return true;
}

void *lista_iter_ver_actual(const lista_iter_t *iter) {
	return iter->actual ? iter->actual->dato : NULL;
}

bool lista_iter_al_final(const lista_iter_t *iter) {
	return iter->actual == NULL;
}

void lista_iter_destruir(lista_iter_t *iter) {
	free(iter);
}

bool lista_iter_insertar(lista_iter_t *iter, void *dato) {
	nodo_t *nuevo_nodo = crear_nodo(dato);
	if (!nuevo_nodo) {
		return false;
	}
	if (iter->anterior) {
		iter->anterior->proximo = nuevo_nodo;
	}
	if (iter->anterior == iter->lista->ultimo) {
		iter->lista->ultimo = nuevo_nodo;
	}
	if (!iter->anterior) {
		iter->lista->primero = nuevo_nodo;
	}
	nuevo_nodo->proximo = iter->actual;
	iter->actual = nuevo_nodo;
	iter->lista->largo++;
	return true;
}

void *lista_iter_borrar(lista_iter_t *iter) {
	if (lista_iter_al_final(iter)) {
		return NULL;
	}
	nodo_t *prox_actual = iter->actual->proximo;
	void *dato_act = iter->actual->dato;
	free(iter->actual);

	if (!prox_actual) {
		iter->lista->ultimo = iter->anterior;
	}
	iter->actual = prox_actual;
	if (iter->anterior) {
		iter->anterior->proximo = iter->actual;
	}
	else {
		iter->lista->primero = iter->actual;
	}
	iter->lista->largo--;

	return dato_act;
}