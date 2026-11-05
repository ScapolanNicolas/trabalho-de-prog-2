/*
 * ============================================================================
 *  Arquivo : algoritmos/busca.c
 *  Módulo  : ALGORITMOS DE BUSCA - implementação
 * ----------------------------------------------------------------------------
 *  BUSCA LINEAR  -> O(n): testa um elemento de cada vez, do primeiro ao
 *                   último. Funciona com o vetor em QUALQUER ordem.
 *
 *  BUSCA BINÁRIA -> O(log n): compara com o elemento do MEIO e descarta
 *                   metade do vetor a cada passo. Só funciona se o vetor
 *                   estiver ORDENADO pela chave pesquisada (o código).
 *
 *  Exemplo com 100 produtos:
 *    - busca linear  : até 100 comparações;
 *    - busca binária : no máximo 7 comparações (2^7 = 128 >= 100).
 * ============================================================================
 */

#include <stddef.h>      /* NULL */

#include "busca.h"

/*
 * contemTexto
 * Implementação manual de "procurar substring", sem usar strstr().
 * Para cada posição i do texto, tenta casar o trecho inteiro a partir dali,
 * comparando caractere a caractere em minúsculo.
 *
 * Ex.: texto = "Mouse Gamer", trecho = "gam"
 *      i=0 "Mou..." x "gam" -> falha
 *      ...
 *      i=6 "Gam"    x "gam" -> casou todos os caracteres -> retorna 1
 */
int contemTexto(const char *texto, const char *trecho)
{
    int i, j;

    if (trecho[0] == '\0')
    {
        return 1;                 /* trecho vazio está contido em qualquer texto */
    }

    for (i = 0; texto[i] != '\0'; i++)
    {
        j = 0;
        while (trecho[j] != '\0' &&
               texto[i + j] != '\0' &&
               paraMinusculo(texto[i + j]) == paraMinusculo(trecho[j]))
        {
            j++;
        }

        if (trecho[j] == '\0')    /* chegou ao fim do trecho: encontrou */
        {
            return 1;
        }
    }
    return 0;
}

/*
 * buscaLinearNome
 * Percorre TODO o vetor (do índice 0 ao último) e guarda o índice de cada
 * produto cujo nome contém o termo pesquisado. Não para no primeiro
 * resultado, pois vários produtos podem ter nomes parecidos.
 */
int buscaLinearNome(const Produto produtos[], int quantidade,
                    const char *termo, int indicesEncontrados[])
{
    int i;
    int total = 0;

    for (i = 0; i < quantidade; i++)
    {
        if (contemTexto(produtos[i].nome, termo))
        {
            indicesEncontrados[total] = i;
            total++;
        }
    }
    return total;
}

/*
 * buscaLinearCodigo
 * Compara o código de cada produto, em sequência, até achar o procurado.
 * Retorna assim que encontra (não precisa ver o resto do vetor).
 */
int buscaLinearCodigo(const Produto produtos[], int quantidade,
                      int codigo, int *comparacoes)
{
    int i;
    int contador = 0;

    for (i = 0; i < quantidade; i++)
    {
        contador++;
        if (produtos[i].codigo == codigo)
        {
            if (comparacoes != NULL)
            {
                *comparacoes = contador;   /* devolve por referência */
            }
            return i;
        }
    }

    if (comparacoes != NULL)
    {
        *comparacoes = contador;
    }
    return -1;
}

/*
 * buscaBinariaCodigo
 * Mantém dois limites, 'inicio' e 'fim', que delimitam a parte do vetor
 * onde o código ainda pode estar.
 *
 * A cada repetição:
 *   1. calcula o meio;
 *   2. se o código do meio for o procurado -> encontrou;
 *   3. se o código do meio for MENOR que o procurado, o alvo só pode estar
 *      à direita -> inicio = meio + 1;
 *   4. senão, só pode estar à esquerda -> fim = meio - 1.
 * Quando inicio > fim, a faixa ficou vazia: o código não existe.
 *
 * O cálculo  meio = inicio + (fim - inicio) / 2  evita estouro de inteiro
 * que poderia ocorrer com (inicio + fim) / 2 em vetores muito grandes.
 */
int buscaBinariaCodigo(const Produto produtos[], int quantidade,
                       int codigo, int *comparacoes)
{
    int inicio = 0;
    int fim = quantidade - 1;
    int meio;
    int contador = 0;

    while (inicio <= fim)
    {
        meio = inicio + (fim - inicio) / 2;
        contador++;

        if (produtos[meio].codigo == codigo)
        {
            if (comparacoes != NULL)
            {
                *comparacoes = contador;
            }
            return meio;
        }

        if (produtos[meio].codigo < codigo)
        {
            inicio = meio + 1;     /* descarta a metade da esquerda */
        }
        else
        {
            fim = meio - 1;        /* descarta a metade da direita  */
        }
    }

    if (comparacoes != NULL)
    {
        *comparacoes = contador;
    }
    return -1;
}

/*
 * estaOrdenadoPorCodigo
 * Confere se cada código é menor que o seguinte.
 */
int estaOrdenadoPorCodigo(const Produto produtos[], int quantidade)
{
    int i;

    for (i = 0; i < quantidade - 1; i++)
    {
        if (produtos[i].codigo > produtos[i + 1].codigo)
        {
            return 0;
        }
    }
    return 1;
}
