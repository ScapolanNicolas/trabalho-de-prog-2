/*
 * ============================================================================
 *  Arquivo : models/produto.h
 *  Módulo  : MODELO DE DADOS
 * ----------------------------------------------------------------------------
 *  Define a entidade principal do sistema (struct Produto), as constantes
 *  globais do projeto e declara as funções que manipulam:
 *
 *    - a entrada de dados pelo teclado (com validação);
 *    - o vetor de produtos em memória (cadastrar, alterar, excluir, listar);
 *    - o controle de estoque (entrada e saída);
 *    - os relatórios (baixo estoque e valor total);
 *    - a MATRIZ de setores do estoque (mapa físico do armazém).
 *
 *  Este módulo NÃO conhece o banco de dados. Ele trabalha apenas com a
 *  memória. Quem sincroniza memória e banco é o main.c, chamando o módulo
 *  persistencia/banco.c logo após cada alteração.
 * ============================================================================
 */

#ifndef PRODUTO_H
#define PRODUTO_H

/* ---------------------------------------------------------------------------
 *  Constantes do sistema
 * ------------------------------------------------------------------------- */
#define MAX_PRODUTOS        100      /* capacidade máxima do vetor            */
#define TAM_NOME            100      /* tamanho do campo nome (com o '\0')    */
#define ESTOQUE_MINIMO      10       /* abaixo disso = "baixo estoque"        */
#define CODIGO_MAXIMO       999999   /* maior código de produto aceito        */
#define QUANTIDADE_MAXIMA   1000000  /* limite de unidades por produto        */
#define PRECO_MAXIMO        1000000.0f /* maior preço unitário aceito (R$)    */

/* Matriz de setores: 5 setores (linhas) x 5 prateleiras (colunas) */
#define SETOR_LINHAS        5
#define SETOR_COLUNAS       5
#define POSICAO_VAZIA       0        /* código 0 = posição livre na matriz    */

/* ---------------------------------------------------------------------------
 *  Entidade principal
 * ------------------------------------------------------------------------- */
typedef struct
{
    int   codigo;            /* identificador único (chave primária)         */
    char  nome[TAM_NOME];    /* descrição do produto                         */
    float preco;             /* preço unitário em R$                         */
    int   quantidade;        /* unidades disponíveis em estoque              */
} Produto;

/* ===========================================================================
 *  1) FUNÇÕES AUXILIARES DE ENTRADA E TEXTO
 *     Toda leitura é feita linha a linha com fgets(), evitando os problemas
 *     clássicos do scanf() (lixo no buffer, laço infinito com letras etc.).
 * ========================================================================= */

/* Lê uma linha do teclado para 'destino' (sem o '\n'). */
void  lerLinha(char *destino, int tamanho);

/* Remove espaços do início e do fim do texto (altera o próprio texto). */
void  aparar(char *texto);

/* Retorna 1 se o texto estiver vazio ou contiver apenas espaços. */
int   textoVazio(const char *texto);

/* Converte UM caractere ASCII para minúsculo (implementação manual). */
char  paraMinusculo(char c);

/* Lê um texto obrigatório (não aceita vazio). */
void  lerTexto(const char *mensagem, char *destino, int tamanho);

/* Lê um inteiro dentro do intervalo [minimo, maximo]. Repete até ser válido. */
int   lerInteiro(const char *mensagem, int minimo, int maximo);

/* Lê um número real (aceita vírgula ou ponto). Repete até ser válido. */
float lerFloat(const char *mensagem);

/* Pergunta S/N. Retorna 1 para "sim" e 0 para "não". */
int   confirmar(const char *mensagem);

/* Aguarda o usuário pressionar ENTER. */
void  pausar(void);

/* ===========================================================================
 *  2) EXIBIÇÃO
 * ========================================================================= */
void  exibirCabecalhoTabela(void);
void  exibirProduto(const Produto *p);
void  listarProdutos(const Produto produtos[], int quantidade);

/* ===========================================================================
 *  3) CRUD EM MEMÓRIA (vetor de structs)
 *     'quantidade' é passada por REFERÊNCIA (int *) quando a função precisa
 *     alterar o número de produtos cadastrados.
 *     Retorno: índice do produto afetado no vetor, ou -1 em caso de falha.
 * ========================================================================= */
int   cadastrarProduto(Produto produtos[], int *quantidade);
int   alterarProduto(Produto produtos[], int quantidade);
int   excluirProduto(Produto produtos[], int *quantidade, int *codigoExcluido);

/* ===========================================================================
 *  4) CONTROLE DE ESTOQUE
 *     Retorno: índice do produto movimentado, ou -1 se nada foi alterado.
 * ========================================================================= */
int   entradaEstoque(Produto produtos[], int quantidade);
int   saidaEstoque(Produto produtos[], int quantidade);

/* ===========================================================================
 *  5) RELATÓRIOS
 * ========================================================================= */
int    relatorioBaixoEstoque(const Produto produtos[], int quantidade);
double calcularValorTotal(const Produto produtos[], int quantidade);
void   relatorioValorTotal(const Produto produtos[], int quantidade);

/* ===========================================================================
 *  6) MATRIZ DE SETORES  -  int setores[SETOR_LINHAS][SETOR_COLUNAS]
 *     Cada posição guarda o CÓDIGO do produto armazenado ali
 *     (POSICAO_VAZIA = 0 significa posição livre).
 *     Linhas e colunas recebidas por parâmetro são 0-based (0 a 4).
 * ========================================================================= */
void  inicializarSetores(int setores[][SETOR_COLUNAS]);

/* Retorna 1 = sucesso | 0 = posição inválida | -1 = posição já ocupada */
int   inserirProdutoSetor(int setores[][SETOR_COLUNAS], int linha, int coluna, int codigo);

/* Retorna 1 = sucesso | 0 = posição inválida | -1 = posição já estava vazia */
int   removerProdutoSetor(int setores[][SETOR_COLUNAS], int linha, int coluna);

/* Libera TODAS as posições ocupadas por um código. Retorna quantas liberou. */
int   removerProdutoDosSetores(int setores[][SETOR_COLUNAS], int codigo);

/* Mostra em quais posições um produto está guardado. Retorna quantas achou. */
int   localizarProdutoNosSetores(int setores[][SETOR_COLUNAS], int codigo);

void  mostrarMapaSetores(int setores[][SETOR_COLUNAS]);
void  consultarOcupacaoSetores(int setores[][SETOR_COLUNAS]);

#endif /* PRODUTO_H */
