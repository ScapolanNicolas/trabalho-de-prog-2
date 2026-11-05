/*
 * ============================================================================
 *  Arquivo : algoritmos/ordenacao.h
 *  Módulo  : ALGORITMO DE ORDENAÇÃO
 * ----------------------------------------------------------------------------
 *  Declara o BUBBLE SORT implementado manualmente em C (sem qsort()).
 *  Um único algoritmo atende a todos os critérios de ordenação; o que
 *  muda é apenas a regra de comparação entre dois produtos.
 * ============================================================================
 */

#ifndef ORDENACAO_H
#define ORDENACAO_H

#include "../models/produto.h"

/* Critérios de ordenação disponíveis (todos em ordem crescente) */
typedef enum
{
    ORDENAR_POR_CODIGO,       /* usado antes da busca binária     */
    ORDENAR_POR_NOME,         /* ordem alfabética (A -> Z)        */
    ORDENAR_POR_PRECO,        /* do mais barato ao mais caro      */
    ORDENAR_POR_QUANTIDADE    /* do menor ao maior estoque        */
} CriterioOrdenacao;

/*
 * Ordena o vetor 'produtos' (in-place) segundo o critério escolhido.
 * Retorna o número de trocas realizadas (útil para a apresentação).
 */
int bubbleSort(Produto produtos[], int quantidade, CriterioOrdenacao criterio);

/*
 * Compara dois nomes sem diferenciar maiúsculas/minúsculas.
 * Retorna < 0 se a < b, 0 se iguais, > 0 se a > b (como strcmp).
 */
int compararNomes(const char *a, const char *b);

/* Retorna o nome do critério em texto (para exibir ao usuário). */
const char *nomeCriterio(CriterioOrdenacao criterio);

#endif /* ORDENACAO_H */
