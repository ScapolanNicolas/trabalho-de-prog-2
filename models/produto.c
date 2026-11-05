/*
 * ============================================================================
 *  Arquivo : models/produto.c
 *  Módulo  : MODELO DE DADOS - implementação
 * ----------------------------------------------------------------------------
 *  Implementa as funções declaradas em produto.h:
 *    1) leitura e validação de dados digitados pelo usuário;
 *    2) exibição de produtos em formato de tabela;
 *    3) cadastro, alteração e exclusão de produtos no VETOR de structs;
 *    4) entrada e saída de estoque;
 *    5) relatórios (baixo estoque e valor total);
 *    6) manipulação da MATRIZ de setores do estoque.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "produto.h"
#include "../algoritmos/busca.h"   /* busca linear usada para validar códigos */

#define TAM_BUFFER 256             /* buffer para leitura de números/opções   */

/* ===========================================================================
 *  1) FUNÇÕES AUXILIARES DE ENTRADA E TEXTO
 * ========================================================================= */

/*
 * lerLinha
 * Lê uma linha inteira do teclado com fgets() e remove o '\n' final.
 * Se o usuário digitar mais caracteres do que cabem no buffer, o restante
 * da linha é descartado para não "sobrar" para a próxima leitura.
 * Se a entrada padrão for encerrada (EOF, ex.: Ctrl+Z / Ctrl+D), o
 * programa termina de forma controlada.
 */
void lerLinha(char *destino, int tamanho)
{
    size_t comprimento;
    int c;

    if (fgets(destino, tamanho, stdin) == NULL)
    {
        printf("\nEntrada encerrada. Finalizando o programa.\n");
        exit(EXIT_SUCCESS);
    }

    comprimento = strlen(destino);

    if (comprimento > 0 && destino[comprimento - 1] == '\n')
    {
        destino[comprimento - 1] = '\0';          /* remove o '\n'          */
    }
    else
    {
        /* linha maior que o buffer: descarta o que sobrou no stdin */
        while ((c = getchar()) != '\n' && c != EOF)
        {
            /* apenas consome os caracteres */
        }
    }

    /* Em alguns terminais Windows pode sobrar um '\r' no fim da linha */
    comprimento = strlen(destino);
    if (comprimento > 0 && destino[comprimento - 1] == '\r')
    {
        destino[comprimento - 1] = '\0';
    }
}

/*
 * aparar
 * Remove espaços/tabs do início e do fim do texto.
 * Recebe um PONTEIRO para o texto e o modifica diretamente.
 */
void aparar(char *texto)
{
    int inicio = 0;
    int fim;
    int i;

    /* avança enquanto houver espaços no começo */
    while (texto[inicio] == ' ' || texto[inicio] == '\t')
    {
        inicio++;
    }

    /* desloca o texto para a esquerda, eliminando os espaços iniciais */
    if (inicio > 0)
    {
        i = 0;
        while (texto[inicio + i] != '\0')
        {
            texto[i] = texto[inicio + i];
            i++;
        }
        texto[i] = '\0';
    }

    /* remove espaços do final */
    fim = (int) strlen(texto) - 1;
    while (fim >= 0 && (texto[fim] == ' ' || texto[fim] == '\t'))
    {
        texto[fim] = '\0';
        fim--;
    }
}

/*
 * textoVazio
 * Retorna 1 se o texto não tiver nenhum caractere diferente de espaço.
 */
int textoVazio(const char *texto)
{
    int i;

    for (i = 0; texto[i] != '\0'; i++)
    {
        if (texto[i] != ' ' && texto[i] != '\t')
        {
            return 0;
        }
    }
    return 1;
}

/*
 * paraMinusculo
 * Converte uma letra maiúscula ASCII (A-Z) em minúscula (a-z).
 * Implementação manual: na tabela ASCII, 'a' - 'A' = 32.
 * Usada pela busca linear e pela ordenação para comparar nomes
 * sem diferenciar maiúsculas de minúsculas.
 */
char paraMinusculo(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (char) (c + ('a' - 'A'));
    }
    return c;
}

/*
 * lerTexto
 * Lê um texto obrigatório. Enquanto o usuário digitar algo vazio
 * (ou só espaços), exibe erro e pede novamente.
 */
void lerTexto(const char *mensagem, char *destino, int tamanho)
{
    while (1)
    {
        printf("%s", mensagem);
        lerLinha(destino, tamanho);
        aparar(destino);

        if (textoVazio(destino))
        {
            printf("  [ERRO] O campo não pode ficar vazio.\n");
        }
        else
        {
            return;
        }
    }
}

/*
 * lerInteiro
 * Lê um número inteiro e valida:
 *   - se algo foi digitado;
 *   - se é realmente um número inteiro (rejeita "12abc", "3.5", "abc");
 *   - se está dentro do intervalo [minimo, maximo].
 * Repete a pergunta até receber um valor válido.
 */
int lerInteiro(const char *mensagem, int minimo, int maximo)
{
    char  buffer[TAM_BUFFER];
    char *fim;
    long  valor;

    while (1)
    {
        printf("%s", mensagem);
        lerLinha(buffer, TAM_BUFFER);
        aparar(buffer);

        if (buffer[0] == '\0')
        {
            printf("  [ERRO] Nenhum valor foi digitado.\n");
            continue;
        }

        errno = 0;
        valor = strtol(buffer, &fim, 10);   /* converte texto -> long */

        /* *fim != '\0' significa que sobrou texto não numérico */
        if (*fim != '\0' || errno == ERANGE)
        {
            printf("  [ERRO] Digite um número inteiro válido.\n");
            continue;
        }

        if (valor < minimo || valor > maximo)
        {
            printf("  [ERRO] O valor deve estar entre %d e %d.\n", minimo, maximo);
            continue;
        }

        return (int) valor;
    }
}

/*
 * lerFloat
 * Lê um número real. Aceita vírgula (padrão brasileiro) ou ponto como
 * separador decimal. Rejeita textos que não sejam números.
 * A validação de faixa (ex.: preço negativo) é feita por quem chama.
 */
float lerFloat(const char *mensagem)
{
    char   buffer[TAM_BUFFER];
    char  *fim;
    double valor;
    int    i;

    while (1)
    {
        printf("%s", mensagem);
        lerLinha(buffer, TAM_BUFFER);
        aparar(buffer);

        if (buffer[0] == '\0')
        {
            printf("  [ERRO] Nenhum valor foi digitado.\n");
            continue;
        }

        /* troca vírgula por ponto: "12,50" -> "12.50" */
        for (i = 0; buffer[i] != '\0'; i++)
        {
            if (buffer[i] == ',')
            {
                buffer[i] = '.';
            }
        }

        errno = 0;
        valor = strtod(buffer, &fim);

        /* valor != valor só é verdadeiro para NaN ("nan") */
        if (*fim != '\0' || errno == ERANGE || valor != valor ||
            valor > 1e12 || valor < -1e12)
        {
            printf("  [ERRO] Digite um número válido (ex.: 12,50).\n");
            continue;
        }

        return (float) valor;
    }
}

/*
 * confirmar
 * Faz uma pergunta de Sim/Não. Aceita S/s/N/n.
 */
int confirmar(const char *mensagem)
{
    char buffer[TAM_BUFFER];

    while (1)
    {
        printf("%s (S/N): ", mensagem);
        lerLinha(buffer, TAM_BUFFER);
        aparar(buffer);

        if (buffer[0] != '\0' && buffer[1] == '\0')
        {
            if (buffer[0] == 'S' || buffer[0] == 's') return 1;
            if (buffer[0] == 'N' || buffer[0] == 'n') return 0;
        }
        printf("  [ERRO] Responda apenas S ou N.\n");
    }
}

/*
 * pausar
 * Segura a tela até o usuário pressionar ENTER.
 */
void pausar(void)
{
    char buffer[TAM_BUFFER];

    printf("\nPressione ENTER para continuar...");
    lerLinha(buffer, TAM_BUFFER);
}

/*
 * lerPreco (função interna deste arquivo -> static)
 * Lê o preço e aplica as regras de negócio:
 *   - não pode ser negativo;
 *   - não pode ser zero;
 *   - não pode ultrapassar PRECO_MAXIMO.
 */
static float lerPreco(const char *mensagem)
{
    float preco;

    while (1)
    {
        preco = lerFloat(mensagem);

        if (preco < 0)
        {
            printf("  [ERRO] Preço negativo não é permitido.\n");
        }
        else if (preco == 0)
        {
            printf("  [ERRO] O preço deve ser maior que zero.\n");
        }
        else if (preco > PRECO_MAXIMO)
        {
            printf("  [ERRO] Preço acima do limite permitido (R$ %.2f).\n", PRECO_MAXIMO);
        }
        else
        {
            return preco;
        }
    }
}

/*
 * localizarPorCodigo (função interna)
 * Pede um código ao usuário e procura o produto usando BUSCA LINEAR.
 * Retorna o índice no vetor ou -1 se o produto não existir.
 * A busca linear é usada aqui porque funciona com o vetor em qualquer
 * ordem (o usuário pode ter ordenado por nome, preço etc.).
 */
static int localizarPorCodigo(const Produto produtos[], int quantidade, const char *mensagem)
{
    int codigo;
    int indice;

    codigo = lerInteiro(mensagem, 1, CODIGO_MAXIMO);
    indice = buscaLinearCodigo(produtos, quantidade, codigo, NULL);

    if (indice == -1)
    {
        printf("  [ERRO] Produto com código %d não encontrado.\n", codigo);
    }
    return indice;
}

/* ===========================================================================
 *  2) EXIBIÇÃO
 * ========================================================================= */

/*
 * Observação: em UTF-8 as letras acentuadas (Ó, Ç, Ã) ocupam 2 bytes, mas
 * aparecem como 1 caractere na tela. Por isso as larguras dos títulos
 * acentuados têm +1 por letra acentuada, mantendo as colunas alinhadas.
 */
void exibirCabecalhoTabela(void)
{
    printf("\n%-9s %-40s %15s %12s\n", "CÓDIGO", "NOME", "PREÇO (R$)", "QUANTIDADE");
    printf("-------- ---------------------------------------- -------------- ------------\n");
}

/*
 * exibirProduto
 * Recebe um PONTEIRO constante: lê o produto sem copiá-lo e sem
 * poder alterá-lo. Acessa os campos com o operador seta (->).
 */
void exibirProduto(const Produto *p)
{
    printf("%-8d %-40.40s %14.2f %12d\n", p->codigo, p->nome, p->preco, p->quantidade);
}

void listarProdutos(const Produto produtos[], int quantidade)
{
    int i;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    exibirCabecalhoTabela();
    for (i = 0; i < quantidade; i++)
    {
        exibirProduto(&produtos[i]);       /* passa o ENDEREÇO do elemento */
    }
    printf("\nTotal de produtos: %d\n", quantidade);
}

/* ===========================================================================
 *  3) CRUD EM MEMÓRIA
 * ========================================================================= */

/*
 * cadastrarProduto
 * Lê os dados de um novo produto, valida e o insere no fim do vetor.
 * 'quantidade' é passada POR REFERÊNCIA: a função incrementa o contador
 * de produtos que pertence ao main().
 * Retorna o índice do novo produto ou -1 se o cadastro falhar.
 */
int cadastrarProduto(Produto produtos[], int *quantidade)
{
    Produto novo;

    if (*quantidade >= MAX_PRODUTOS)
    {
        printf("  [ERRO] Limite de %d produtos atingido.\n", MAX_PRODUTOS);
        return -1;
    }

    printf("\n--- CADASTRO DE PRODUTO ---\n");

    /* Validação: código duplicado (busca linear no vetor) */
    novo.codigo = lerInteiro("Código (1 a 999999): ", 1, CODIGO_MAXIMO);
    if (buscaLinearCodigo(produtos, *quantidade, novo.codigo, NULL) != -1)
    {
        printf("  [ERRO] Já existe um produto com o código %d.\n", novo.codigo);
        return -1;
    }

    /* Validação: nome vazio é tratado dentro de lerTexto() */
    lerTexto("Nome: ", novo.nome, TAM_NOME);

    /* Validação: preço negativo/zero é tratado dentro de lerPreco() */
    novo.preco = lerPreco("Preço unitário (R$): ");

    /* Validação: quantidade negativa é barrada pelo intervalo mínimo 0 */
    novo.quantidade = lerInteiro("Quantidade inicial: ", 0, QUANTIDADE_MAXIMA);

    produtos[*quantidade] = novo;   /* copia a struct para o vetor */
    (*quantidade)++;                /* altera a variável do main via ponteiro */

    return *quantidade - 1;
}

/*
 * alterarProduto
 * Localiza o produto pelo código e permite alterar nome, preço,
 * quantidade ou todos os campos. O código (chave primária) não muda.
 * Usa um PONTEIRO para o elemento do vetor (Produto *p) e altera o
 * produto diretamente na memória, sem cópias.
 */
int alterarProduto(Produto produtos[], int quantidade)
{
    int      indice;
    int      opcao;
    Produto *p;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return -1;
    }

    printf("\n--- ALTERAÇÃO DE PRODUTO ---\n");
    indice = localizarPorCodigo(produtos, quantidade, "Código do produto a alterar: ");
    if (indice == -1)
    {
        return -1;
    }

    p = &produtos[indice];          /* p aponta para o produto dentro do vetor */

    printf("\nDados atuais:");
    exibirCabecalhoTabela();
    exibirProduto(p);

    printf("\nO que deseja alterar?\n");
    printf("  1 - Nome\n");
    printf("  2 - Preço\n");
    printf("  3 - Quantidade\n");
    printf("  4 - Todos os campos\n");
    printf("  0 - Cancelar\n");
    opcao = lerInteiro("Opção: ", 0, 4);

    if (opcao == 0)
    {
        printf("Alteração cancelada.\n");
        return -1;
    }

    if (opcao == 1 || opcao == 4)
    {
        lerTexto("Novo nome: ", p->nome, TAM_NOME);
    }
    if (opcao == 2 || opcao == 4)
    {
        p->preco = lerPreco("Novo preço (R$): ");
    }
    if (opcao == 3 || opcao == 4)
    {
        p->quantidade = lerInteiro("Nova quantidade: ", 0, QUANTIDADE_MAXIMA);
    }

    return indice;
}

/*
 * excluirProduto
 * Remove um produto do vetor deslocando os elementos seguintes uma
 * posição para a esquerda (mantém a ordem dos demais).
 * - 'quantidade' por referência: o total de produtos diminui.
 * - 'codigoExcluido' por referência: devolve ao main o código removido,
 *   para que ele também seja apagado do banco e da matriz de setores.
 * Retorna 1 se excluiu ou -1 caso contrário.
 */
int excluirProduto(Produto produtos[], int *quantidade, int *codigoExcluido)
{
    int indice;
    int i;

    if (*quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return -1;
    }

    printf("\n--- EXCLUSÃO DE PRODUTO ---\n");
    indice = localizarPorCodigo(produtos, *quantidade, "Código do produto a excluir: ");
    if (indice == -1)
    {
        return -1;
    }

    exibirCabecalhoTabela();
    exibirProduto(&produtos[indice]);

    if (!confirmar("\nConfirma a exclusão deste produto?"))
    {
        printf("Exclusão cancelada.\n");
        return -1;
    }

    *codigoExcluido = produtos[indice].codigo;

    /* desloca todos os produtos à direita do excluído uma casa para trás */
    for (i = indice; i < *quantidade - 1; i++)
    {
        produtos[i] = produtos[i + 1];
    }
    (*quantidade)--;

    return 1;
}

/* ===========================================================================
 *  4) CONTROLE DE ESTOQUE
 * ========================================================================= */

/*
 * entradaEstoque
 * Soma unidades ao estoque de um produto existente.
 * Regras: quantidade deve ser positiva e o total não pode ultrapassar
 * QUANTIDADE_MAXIMA. Digitar 0 cancela a operação.
 */
int entradaEstoque(Produto produtos[], int quantidade)
{
    int      indice;
    int      qtdEntrada;
    int      limite;
    Produto *p;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return -1;
    }

    printf("\n--- ENTRADA DE ESTOQUE ---\n");
    indice = localizarPorCodigo(produtos, quantidade, "Código do produto: ");
    if (indice == -1)
    {
        return -1;
    }

    p = &produtos[indice];
    printf("Produto: %s | Estoque atual: %d\n", p->nome, p->quantidade);

    limite = QUANTIDADE_MAXIMA - p->quantidade;
    if (limite <= 0)
    {
        printf("  [ERRO] Estoque já está no limite máximo (%d).\n", QUANTIDADE_MAXIMA);
        return -1;
    }

    qtdEntrada = lerInteiro("Quantidade de entrada (0 para cancelar): ", 0, limite);
    if (qtdEntrada == 0)
    {
        printf("Operação cancelada.\n");
        return -1;
    }

    p->quantidade += qtdEntrada;
    printf("Entrada registrada. Novo estoque: %d\n", p->quantidade);
    return indice;
}

/*
 * saidaEstoque
 * Retira unidades do estoque de um produto existente.
 * Regras: quantidade deve ser positiva e NÃO pode ser maior que o
 * estoque disponível (o estoque nunca fica negativo).
 */
int saidaEstoque(Produto produtos[], int quantidade)
{
    int      indice;
    int      qtdSaida;
    Produto *p;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return -1;
    }

    printf("\n--- SAÍDA DE ESTOQUE ---\n");
    indice = localizarPorCodigo(produtos, quantidade, "Código do produto: ");
    if (indice == -1)
    {
        return -1;
    }

    p = &produtos[indice];
    printf("Produto: %s | Estoque atual: %d\n", p->nome, p->quantidade);

    if (p->quantidade == 0)
    {
        printf("  [ERRO] Produto sem estoque. Não é possível registrar saída.\n");
        return -1;
    }

    while (1)
    {
        qtdSaida = lerInteiro("Quantidade de saída (0 para cancelar): ", 0, QUANTIDADE_MAXIMA);

        if (qtdSaida == 0)
        {
            printf("Operação cancelada.\n");
            return -1;
        }
        if (qtdSaida > p->quantidade)
        {
            printf("  [ERRO] Saída (%d) maior que o estoque disponível (%d).\n",
                   qtdSaida, p->quantidade);
            continue;
        }
        break;
    }

    p->quantidade -= qtdSaida;
    printf("Saída registrada. Novo estoque: %d\n", p->quantidade);
    return indice;
}

/* ===========================================================================
 *  5) RELATÓRIOS
 * ========================================================================= */

/*
 * relatorioBaixoEstoque
 * Percorre o vetor e exibe os produtos com quantidade menor que
 * ESTOQUE_MINIMO (10). Retorna quantos produtos foram encontrados.
 */
int relatorioBaixoEstoque(const Produto produtos[], int quantidade)
{
    int i;
    int encontrados = 0;

    printf("\n--- PRODUTOS COM BAIXO ESTOQUE (menos de %d unidades) ---\n", ESTOQUE_MINIMO);

    for (i = 0; i < quantidade; i++)
    {
        if (produtos[i].quantidade < ESTOQUE_MINIMO)
        {
            if (encontrados == 0)
            {
                exibirCabecalhoTabela();
            }
            exibirProduto(&produtos[i]);
            encontrados++;
        }
    }

    if (encontrados == 0)
    {
        printf("\nNenhum produto com baixo estoque.\n");
    }
    else
    {
        printf("\n%d produto(s) precisam de reposição.\n", encontrados);
    }
    return encontrados;
}

/*
 * calcularValorTotal
 * Acumula preço x quantidade de todos os produtos.
 * Usa double para reduzir erros de arredondamento na soma.
 */
double calcularValorTotal(const Produto produtos[], int quantidade)
{
    int    i;
    double valor_total = 0.0;

    for (i = 0; i < quantidade; i++)
    {
        valor_total += (double) produtos[i].preco * produtos[i].quantidade;
    }
    return valor_total;
}

/*
 * relatorioValorTotal
 * Mostra o subtotal de cada produto e o valor total do estoque.
 */
void relatorioValorTotal(const Produto produtos[], int quantidade)
{
    int i;

    printf("\n--- VALOR TOTAL DO ESTOQUE ---\n");

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    printf("\n%-9s %-30s %13s %8s %16s\n", "CÓDIGO", "NOME", "PREÇO", "QTD", "SUBTOTAL (R$)");
    printf("-------- ------------------------------ ------------ -------- ----------------\n");
    for (i = 0; i < quantidade; i++)
    {
        printf("%-8d %-30.30s %12.2f %8d %16.2f\n",
               produtos[i].codigo, produtos[i].nome, produtos[i].preco,
               produtos[i].quantidade,
               (double) produtos[i].preco * produtos[i].quantidade);
    }
    printf("\nVALOR TOTAL DO ESTOQUE: R$ %.2f\n", calcularValorTotal(produtos, quantidade));
}

/* ===========================================================================
 *  6) MATRIZ DE SETORES
 *     Linhas  = setores do armazém  (S1 a S5)
 *     Colunas = prateleiras         (P1 a P5)
 * ========================================================================= */

/* Marca todas as 25 posições da matriz como vazias. */
void inicializarSetores(int setores[][SETOR_COLUNAS])
{
    int i, j;

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            setores[i][j] = POSICAO_VAZIA;
        }
    }
}

/* Função interna: verifica se (linha, coluna) está dentro da matriz. */
static int posicaoValida(int linha, int coluna)
{
    return linha >= 0 && linha < SETOR_LINHAS && coluna >= 0 && coluna < SETOR_COLUNAS;
}

int inserirProdutoSetor(int setores[][SETOR_COLUNAS], int linha, int coluna, int codigo)
{
    if (!posicaoValida(linha, coluna))
    {
        return 0;
    }
    if (setores[linha][coluna] != POSICAO_VAZIA)
    {
        return -1;
    }
    setores[linha][coluna] = codigo;
    return 1;
}

int removerProdutoSetor(int setores[][SETOR_COLUNAS], int linha, int coluna)
{
    if (!posicaoValida(linha, coluna))
    {
        return 0;
    }
    if (setores[linha][coluna] == POSICAO_VAZIA)
    {
        return -1;
    }
    setores[linha][coluna] = POSICAO_VAZIA;
    return 1;
}

int removerProdutoDosSetores(int setores[][SETOR_COLUNAS], int codigo)
{
    int i, j;
    int liberadas = 0;

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            if (setores[i][j] == codigo)
            {
                setores[i][j] = POSICAO_VAZIA;
                liberadas++;
            }
        }
    }
    return liberadas;
}

int localizarProdutoNosSetores(int setores[][SETOR_COLUNAS], int codigo)
{
    int i, j;
    int encontradas = 0;

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            if (setores[i][j] == codigo)
            {
                printf("  -> Setor S%d, Prateleira P%d\n", i + 1, j + 1);
                encontradas++;
            }
        }
    }

    if (encontradas == 0)
    {
        printf("  Produto %d não está armazenado em nenhum setor.\n", codigo);
    }
    return encontradas;
}

/*
 * mostrarMapaSetores
 * Desenha a matriz em forma de grade. Posições vazias aparecem como "----".
 */
void mostrarMapaSetores(int setores[][SETOR_COLUNAS])
{
    int i, j;

    printf("\n=============== MAPA DOS SETORES DO ESTOQUE ===============\n");
    printf("        ");
    for (j = 0; j < SETOR_COLUNAS; j++)
    {
        printf("    P%d   ", j + 1);
    }
    printf("\n");

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        printf("  S%d   |", i + 1);
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            if (setores[i][j] == POSICAO_VAZIA)
            {
                printf("   ----  ");
            }
            else
            {
                printf(" %7d ", setores[i][j]);
            }
        }
        printf("|\n");
    }
    printf("\nS = Setor | P = Prateleira | ---- = posição livre\n");
}

/*
 * consultarOcupacaoSetores
 * Percorre a matriz linha por linha (setor por setor), conta as posições
 * ocupadas e mostra o percentual de ocupação de cada setor e do total.
 */
void consultarOcupacaoSetores(int setores[][SETOR_COLUNAS])
{
    int i, j;
    int ocupadasSetor;
    int ocupadasTotal = 0;
    int totalPosicoes = SETOR_LINHAS * SETOR_COLUNAS;

    printf("\n--- OCUPAÇÃO DOS SETORES ---\n\n");
    printf("%-8s %10s %10s %14s\n", "SETOR", "OCUPADAS", "LIVRES", "OCUPAÇÃO");
    printf("-------- ---------- ---------- ------------\n");

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        ocupadasSetor = 0;
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            if (setores[i][j] != POSICAO_VAZIA)
            {
                ocupadasSetor++;
            }
        }
        ocupadasTotal += ocupadasSetor;

        printf("S%-7d %10d %10d %11.1f%%\n", i + 1, ocupadasSetor,
               SETOR_COLUNAS - ocupadasSetor,
               100.0 * ocupadasSetor / SETOR_COLUNAS);
    }

    printf("-------- ---------- ---------- ------------\n");
    printf("%-8s %10d %10d %11.1f%%\n", "TOTAL", ocupadasTotal,
           totalPosicoes - ocupadasTotal,
           100.0 * ocupadasTotal / totalPosicoes);
}
