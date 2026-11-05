/*
 * ============================================================================
 *  Arquivo : algoritmos/ordenacao.c
 *  Módulo  : ALGORITMO DE ORDENAÇÃO - implementação (BUBBLE SORT)
 * ----------------------------------------------------------------------------
 *  COMO O BUBBLE SORT FUNCIONA
 *  Percorre o vetor comparando cada par de vizinhos (j e j+1). Se estiverem
 *  fora de ordem, troca os dois de lugar. Ao final de cada passada, o MAIOR
 *  elemento ainda não ordenado "borbulha" até o fim do vetor, ficando na
 *  posição definitiva. Por isso, a cada passada, o laço interno pode
 *  percorrer uma posição a menos (quantidade - 1 - i).
 *
 *  Exemplo (ordenando quantidades 5 3 8 1):
 *    passada 1: [3 5 8 1] -> [3 5 8 1] -> [3 5 1 8]   (8 chegou ao fim)
 *    passada 2: [3 5 1 8] -> [3 1 5 8]                (5 no lugar)
 *    passada 3: [1 3 5 8]                             (ordenado)
 *
 *  OTIMIZAÇÃO: se uma passada inteira terminar sem nenhuma troca, o vetor
 *  já está ordenado e o algoritmo para antes (flag 'trocou').
 *
 *  Complexidade: pior caso O(n^2) | melhor caso O(n) (vetor já ordenado).
 *  É ESTÁVEL: produtos "empatados" mantêm a ordem relativa original.
 * ============================================================================
 */

#include "ordenacao.h"

/*
 * compararNomes
 * Comparação alfabética manual, caractere por caractere, ignorando
 * maiúsculas/minúsculas ("arroz" e "Arroz" são considerados iguais).
 */
int compararNomes(const char *a, const char *b)
{
    int  i = 0;
    char ca, cb;

    while (a[i] != '\0' && b[i] != '\0')
    {
        ca = paraMinusculo(a[i]);
        cb = paraMinusculo(b[i]);

        if (ca != cb)
        {
            return (unsigned char) ca - (unsigned char) cb;
        }
        i++;
    }

    /* um dos textos acabou: o mais curto vem primeiro ("Cabo" < "Cabos") */
    return (unsigned char) paraMinusculo(a[i]) - (unsigned char) paraMinusculo(b[i]);
}

/*
 * deveTrocar (função interna)
 * Recebe PONTEIROS para dois produtos vizinhos e decide se eles estão
 * fora de ordem segundo o critério. Retorna 1 se 'a' deve ir depois de 'b'.
 * Usar ponteiros evita copiar as structs inteiras a cada comparação.
 */
static int deveTrocar(const Produto *a, const Produto *b, CriterioOrdenacao criterio)
{
    switch (criterio)
    {
        case ORDENAR_POR_NOME:
            return compararNomes(a->nome, b->nome) > 0;

        case ORDENAR_POR_PRECO:
            return a->preco > b->preco;

        case ORDENAR_POR_QUANTIDADE:
            return a->quantidade > b->quantidade;

        case ORDENAR_POR_CODIGO:
        default:
            return a->codigo > b->codigo;
    }
}

/*
 * trocarProdutos (função interna)
 * Clássica troca com variável auxiliar, feita por REFERÊNCIA:
 * recebe os endereços de dois elementos do vetor e troca seus conteúdos.
 */
static void trocarProdutos(Produto *a, Produto *b)
{
    Produto auxiliar = *a;
    *a = *b;
    *b = auxiliar;
}

/*
 * bubbleSort
 * Ordena o vetor no próprio lugar (in-place). O vetor é recebido como
 * ponteiro para o primeiro elemento, então as trocas alteram o vetor
 * original do main().
 */
int bubbleSort(Produto produtos[], int quantidade, CriterioOrdenacao criterio)
{
    int i, j;
    int trocou;
    int totalTrocas = 0;

    for (i = 0; i < quantidade - 1; i++)              /* cada passada        */
    {
        trocou = 0;

        for (j = 0; j < quantidade - 1 - i; j++)      /* compara vizinhos    */
        {
            if (deveTrocar(&produtos[j], &produtos[j + 1], criterio))
            {
                trocarProdutos(&produtos[j], &produtos[j + 1]);
                trocou = 1;
                totalTrocas++;
            }
        }

        if (!trocou)          /* nenhuma troca: vetor já está ordenado */
        {
            break;
        }
    }
    return totalTrocas;
}

const char *nomeCriterio(CriterioOrdenacao criterio)
{
    switch (criterio)
    {
        case ORDENAR_POR_NOME:       return "nome";
        case ORDENAR_POR_PRECO:      return "preço";
        case ORDENAR_POR_QUANTIDADE: return "quantidade";
        case ORDENAR_POR_CODIGO:
        default:                     return "código";
    }
}
