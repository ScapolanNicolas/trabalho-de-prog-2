/*
 * ============================================================================
 *  Arquivo : algoritmos/busca.h
 *  Módulo  : ALGORITMOS DE BUSCA
 * ----------------------------------------------------------------------------
 *  Declara as funções de pesquisa implementadas MANUALMENTE em C:
 *
 *    - Busca LINEAR por nome   (percorre o vetor elemento a elemento);
 *    - Busca LINEAR por código (usada para validar códigos duplicados
 *                               e localizar produtos em qualquer ordem);
 *    - Busca BINÁRIA por código (exige vetor ORDENADO por código).
 *
 *  Nenhuma função de biblioteca de busca (bsearch, strstr etc.) é usada.
 *  O parâmetro 'comparacoes' (ponteiro) é opcional: se não for NULL, a
 *  função devolve por referência quantas comparações foram feitas,
 *  o que ajuda a demonstrar a eficiência de cada algoritmo.
 * ============================================================================
 */

#ifndef BUSCA_H
#define BUSCA_H

#include "../models/produto.h"

/*
 * Verifica (manualmente) se 'trecho' aparece dentro de 'texto',
 * sem diferenciar maiúsculas de minúsculas. Retorna 1 ou 0.
 */
int contemTexto(const char *texto, const char *trecho);

/*
 * BUSCA LINEAR POR NOME
 * Preenche 'indicesEncontrados' com as posições de TODOS os produtos cujo
 * nome contém 'termo'. Retorna a quantidade de produtos encontrados.
 */
int buscaLinearNome(const Produto produtos[], int quantidade,
                    const char *termo, int indicesEncontrados[]);

/*
 * BUSCA LINEAR POR CÓDIGO
 * Retorna o índice do produto com o código informado ou -1.
 */
int buscaLinearCodigo(const Produto produtos[], int quantidade,
                      int codigo, int *comparacoes);

/*
 * BUSCA BINÁRIA POR CÓDIGO
 * PRÉ-CONDIÇÃO: o vetor deve estar ordenado por código (crescente).
 * Retorna o índice do produto com o código informado ou -1.
 */
int buscaBinariaCodigo(const Produto produtos[], int quantidade,
                       int codigo, int *comparacoes);

/*
 * Retorna 1 se o vetor estiver em ordem crescente de código.
 * Usada para garantir a pré-condição da busca binária.
 */
int estaOrdenadoPorCodigo(const Produto produtos[], int quantidade);

#endif /* BUSCA_H */
