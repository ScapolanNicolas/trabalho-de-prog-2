/*
 * ============================================================================
 *  SISTEMA DE CONTROLE DE ESTOQUE
 *  Projeto Integrador - Algoritmos e Programação II - 2026
 * ----------------------------------------------------------------------------
 *  Arquivo : main.c
 *  Módulo  : PROGRAMA PRINCIPAL (interface com o usuário)
 *
 *  Responsabilidades:
 *    - declarar as estruturas principais em memória:
 *        Produto produtos[MAX_PRODUTOS]  -> vetor de structs
 *        int setores[5][5]               -> matriz de setores
 *        sqlite3 *db                     -> conexão com o banco
 *    - abrir o banco e CARREGAR os dados para a memória ao iniciar;
 *    - exibir o menu e chamar as funções de cada módulo;
 *    - SINCRONIZAR o banco logo após cada alteração feita na memória;
 *    - fechar a conexão ao sair.
 *
 *  O main() apenas coordena: cada opção do menu é tratada por uma função
 *  própria (opcaoXxx), mantendo o código modularizado.
 * ============================================================================
 */

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>     /* apenas para exibir acentos (UTF-8) no console */
#endif

#include "models/produto.h"
#include "algoritmos/busca.h"
#include "algoritmos/ordenacao.h"
#include "persistencia/banco.h"

/* ---------------------------------------------------------------------------
 *  Protótipos das funções deste arquivo
 * ------------------------------------------------------------------------- */
static void configurarConsole(void);
static void exibirMenu(void);
static int  carregarDados(sqlite3 *db, Produto produtos[], int *quantidade,
                          int setores[][SETOR_COLUNAS]);

static void opcaoCadastrar(sqlite3 *db, Produto produtos[], int *quantidade);
static void opcaoAlterar(sqlite3 *db, Produto produtos[], int quantidade);
static void opcaoExcluir(sqlite3 *db, Produto produtos[], int *quantidade,
                         int setores[][SETOR_COLUNAS]);
static void opcaoPesquisarNome(const Produto produtos[], int quantidade);
static void opcaoPesquisarCodigo(Produto produtos[], int quantidade);
static void opcaoOrdenar(Produto produtos[], int quantidade, CriterioOrdenacao criterio);
static void opcaoMovimentarEstoque(sqlite3 *db, Produto produtos[], int quantidade, int entrada);
static void opcaoSalvarBanco(sqlite3 *db, const Produto produtos[], int quantidade,
                             int setores[][SETOR_COLUNAS]);
static void opcaoCarregarBanco(sqlite3 *db, Produto produtos[], int *quantidade,
                               int setores[][SETOR_COLUNAS]);

static void menuSetores(sqlite3 *db, const Produto produtos[], int quantidade,
                        int setores[][SETOR_COLUNAS]);
static void setorInserir(sqlite3 *db, const Produto produtos[], int quantidade,
                         int setores[][SETOR_COLUNAS]);
static void setorRemover(sqlite3 *db, int setores[][SETOR_COLUNAS]);

/* ===========================================================================
 *  FUNÇÃO PRINCIPAL
 * ========================================================================= */
int main(void)
{
    Produto  produtos[MAX_PRODUTOS];               /* vetor de structs      */
    int      quantidade = 0;                       /* produtos cadastrados  */
    int      setores[SETOR_LINHAS][SETOR_COLUNAS]; /* matriz do armazém     */
    sqlite3 *db = NULL;                            /* conexão com o SQLite  */
    int      opcao;

    configurarConsole();
    inicializarSetores(setores);

    /* 1) Conecta ao banco e garante que as tabelas existam */
    if (!abrirBanco(&db, ARQUIVO_BANCO) || !criarTabelas(db))
    {
        printf("Falha ao inicializar o banco de dados. Encerrando.\n");
        fecharBanco(db);
        return 1;
    }

    /* 2) Recupera os dados salvos em execuções anteriores */
    if (carregarDados(db, produtos, &quantidade, setores))
    {
        printf("Banco '%s' conectado. %d produto(s) carregado(s).\n",
               ARQUIVO_BANCO, quantidade);
    }

    /* 3) Laço principal do menu */
    do
    {
        exibirMenu();
        opcao = lerInteiro("Escolha uma opção: ", 0, 16);

        switch (opcao)
        {
            case 1:  opcaoCadastrar(db, produtos, &quantidade);                     break;
            case 2:  opcaoAlterar(db, produtos, quantidade);                        break;
            case 3:  opcaoExcluir(db, produtos, &quantidade, setores);              break;
            case 4:  listarProdutos(produtos, quantidade);                          break;
            case 5:  opcaoPesquisarNome(produtos, quantidade);                      break;
            case 6:  opcaoPesquisarCodigo(produtos, quantidade);                    break;
            case 7:  opcaoOrdenar(produtos, quantidade, ORDENAR_POR_NOME);          break;
            case 8:  opcaoOrdenar(produtos, quantidade, ORDENAR_POR_PRECO);         break;
            case 9:  opcaoOrdenar(produtos, quantidade, ORDENAR_POR_QUANTIDADE);    break;
            case 10: opcaoMovimentarEstoque(db, produtos, quantidade, 1);           break;
            case 11: opcaoMovimentarEstoque(db, produtos, quantidade, 0);           break;
            case 12: relatorioBaixoEstoque(produtos, quantidade);                   break;
            case 13: relatorioValorTotal(produtos, quantidade);                     break;
            case 14: menuSetores(db, produtos, quantidade, setores);                break;
            case 15: opcaoSalvarBanco(db, produtos, quantidade, setores);           break;
            case 16: opcaoCarregarBanco(db, produtos, &quantidade, setores);        break;
            case 0:  printf("\nEncerrando o sistema...\n");                         break;
        }

        if (opcao != 0 && opcao != 14)
        {
            pausar();
        }
    } while (opcao != 0);

    /* 4) Encerra a conexão com o banco */
    fecharBanco(db);
    printf("Conexão com o banco encerrada. Até logo!\n");
    return 0;
}

/* ===========================================================================
 *  CONFIGURAÇÃO E MENU
 * ========================================================================= */

/*
 * configurarConsole
 * No Windows, o console usa por padrão uma página de código antiga e os
 * acentos aparecem errados. Esta função muda a página para UTF-8 (65001).
 * Em Linux/macOS o terminal já é UTF-8 e nada precisa ser feito.
 */
static void configurarConsole(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

static void exibirMenu(void)
{
    printf("\n==================================================\n");
    printf("         SISTEMA DE CONTROLE DE ESTOQUE\n");
    printf("==================================================\n");
    printf("  1 - Cadastrar Produto\n");
    printf("  2 - Alterar Produto\n");
    printf("  3 - Excluir Produto\n");
    printf("  4 - Listar Produtos\n");
    printf("  5 - Pesquisar por Nome        (busca linear)\n");
    printf("  6 - Pesquisar por Código      (busca binária)\n");
    printf("  7 - Ordenar por Nome          (bubble sort)\n");
    printf("  8 - Ordenar por Preço         (bubble sort)\n");
    printf("  9 - Ordenar por Quantidade    (bubble sort)\n");
    printf(" 10 - Entrada de Estoque\n");
    printf(" 11 - Saída de Estoque\n");
    printf(" 12 - Produtos com Baixo Estoque\n");
    printf(" 13 - Valor Total do Estoque\n");
    printf(" 14 - Gerenciar Setores         (matriz)\n");
    printf(" 15 - Salvar no Banco\n");
    printf(" 16 - Carregar do Banco\n");
    printf("  0 - Sair\n");
    printf("==================================================\n");
}

/*
 * carregarDados
 * Lê produtos e setores do banco para a memória (vetor e matriz).
 * É chamada ao iniciar o programa e pela opção 16.
 */
static int carregarDados(sqlite3 *db, Produto produtos[], int *quantidade,
                         int setores[][SETOR_COLUNAS])
{
    if (!listarBanco(db, produtos, quantidade))
    {
        printf("  [ERRO] Não foi possível carregar os produtos do banco.\n");
        return 0;
    }
    if (!carregarSetores(db, setores))
    {
        printf("  [ERRO] Não foi possível carregar os setores do banco.\n");
        return 0;
    }
    return 1;
}

/* ===========================================================================
 *  OPÇÕES DO MENU PRINCIPAL
 *  Padrão adotado: 1) altera a MEMÓRIA  2) grava no BANCO  3) se o banco
 *  falhar, desfaz ou avisa, para que memória e banco não fiquem diferentes.
 * ========================================================================= */

/* Opção 1 - cadastro + INSERT */
static void opcaoCadastrar(sqlite3 *db, Produto produtos[], int *quantidade)
{
    int indice = cadastrarProduto(produtos, quantidade);

    if (indice == -1)
    {
        return;
    }

    if (salvarProduto(db, &produtos[indice]))
    {
        printf("\nProduto cadastrado e salvo no banco com sucesso!\n");
    }
    else
    {
        /* desfaz o cadastro em memória para manter a sincronia */
        (*quantidade)--;
        printf("\n[ERRO] Falha ao salvar no banco. Cadastro desfeito.\n");
    }
}

/* Opção 2 - alteração + UPDATE */
static void opcaoAlterar(sqlite3 *db, Produto produtos[], int quantidade)
{
    int indice = alterarProduto(produtos, quantidade);

    if (indice == -1)
    {
        return;
    }

    if (atualizarProduto(db, &produtos[indice]))
    {
        printf("\nProduto alterado com sucesso!\n");
        exibirCabecalhoTabela();
        exibirProduto(&produtos[indice]);
    }
    else
    {
        printf("\n[AVISO] Alterado na memória, mas não no banco. "
               "Use a opção 15 para tentar salvar novamente.\n");
    }
}

/* Opção 3 - exclusão + DELETE (produto e suas posições nos setores) */
static void opcaoExcluir(sqlite3 *db, Produto produtos[], int *quantidade,
                         int setores[][SETOR_COLUNAS])
{
    int codigo;
    int posicoesLiberadas;

    if (excluirProduto(produtos, quantidade, &codigo) == -1)
    {
        return;
    }

    posicoesLiberadas = removerProdutoDosSetores(setores, codigo);

    if (removerProduto(db, codigo))
    {
        printf("\nProduto %d excluído com sucesso!", codigo);
        if (posicoesLiberadas > 0)
        {
            printf(" (%d posição(ões) de setor liberada(s))", posicoesLiberadas);
        }
        printf("\n");
    }
    else
    {
        printf("\n[AVISO] Excluído da memória, mas não do banco. "
               "Use a opção 15 para tentar salvar novamente.\n");
    }
}

/* Opção 5 - BUSCA LINEAR por nome (aceita parte do nome) */
static void opcaoPesquisarNome(const Produto produtos[], int quantidade)
{
    char termo[TAM_NOME];
    int  indices[MAX_PRODUTOS];
    int  encontrados;
    int  i;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    printf("\n--- PESQUISA POR NOME (BUSCA LINEAR) ---\n");
    lerTexto("Digite o nome ou parte dele: ", termo, TAM_NOME);

    encontrados = buscaLinearNome(produtos, quantidade, termo, indices);

    if (encontrados == 0)
    {
        printf("\nNenhum produto encontrado com \"%s\".\n", termo);
        return;
    }

    exibirCabecalhoTabela();
    for (i = 0; i < encontrados; i++)
    {
        exibirProduto(&produtos[indices[i]]);
    }
    printf("\n%d produto(s) encontrado(s). %d produto(s) verificado(s) pela busca linear.\n",
           encontrados, quantidade);
}

/*
 * Opção 6 - BUSCA BINÁRIA por código
 * Pré-condição da busca binária: vetor ordenado por código.
 * Se não estiver, o Bubble Sort ordena o vetor antes da busca.
 */
static void opcaoPesquisarCodigo(Produto produtos[], int quantidade)
{
    int codigo;
    int indice;
    int comparacoes = 0;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    printf("\n--- PESQUISA POR CÓDIGO (BUSCA BINÁRIA) ---\n");

    if (!estaOrdenadoPorCodigo(produtos, quantidade))
    {
        bubbleSort(produtos, quantidade, ORDENAR_POR_CODIGO);
        printf("(Vetor ordenado por código com Bubble Sort para permitir a busca binária.)\n");
    }

    codigo = lerInteiro("Código: ", 1, CODIGO_MAXIMO);
    indice = buscaBinariaCodigo(produtos, quantidade, codigo, &comparacoes);

    if (indice == -1)
    {
        printf("\n[ERRO] Produto com código %d não encontrado (%d comparação(ões)).\n",
               codigo, comparacoes);
        return;
    }

    exibirCabecalhoTabela();
    exibirProduto(&produtos[indice]);
    printf("\nEncontrado na posição %d do vetor com %d comparação(ões) "
           "(busca linear poderia precisar de até %d).\n",
           indice, comparacoes, quantidade);
}

/* Opções 7, 8 e 9 - BUBBLE SORT pelo critério escolhido */
static void opcaoOrdenar(Produto produtos[], int quantidade, CriterioOrdenacao criterio)
{
    int trocas;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    trocas = bubbleSort(produtos, quantidade, criterio);
    printf("\nProdutos ordenados por %s (Bubble Sort - %d troca(s) realizada(s)):\n",
           nomeCriterio(criterio), trocas);
    listarProdutos(produtos, quantidade);
}

/* Opções 10 e 11 - entrada/saída de estoque + UPDATE */
static void opcaoMovimentarEstoque(sqlite3 *db, Produto produtos[], int quantidade, int entrada)
{
    int indice;

    if (entrada)
    {
        indice = entradaEstoque(produtos, quantidade);
    }
    else
    {
        indice = saidaEstoque(produtos, quantidade);
    }

    if (indice == -1)
    {
        return;
    }

    if (atualizarProduto(db, &produtos[indice]))
    {
        printf("Movimentação gravada no banco.\n");
        if (produtos[indice].quantidade < ESTOQUE_MINIMO)
        {
            printf("[ATENÇÃO] Produto com baixo estoque (menos de %d unidades).\n",
                   ESTOQUE_MINIMO);
        }
    }
    else
    {
        printf("[AVISO] Movimentação feita na memória, mas não no banco. "
               "Use a opção 15 para tentar salvar novamente.\n");
    }
}

/* Opção 15 - grava toda a memória no banco (transação) */
static void opcaoSalvarBanco(sqlite3 *db, const Produto produtos[], int quantidade,
                             int setores[][SETOR_COLUNAS])
{
    if (sincronizarTudo(db, produtos, quantidade, setores))
    {
        printf("\nBanco sincronizado: %d produto(s) gravado(s) em '%s'.\n",
               quantidade, ARQUIVO_BANCO);
    }
    else
    {
        printf("\n[ERRO] Falha na sincronização. O banco foi mantido como estava.\n");
    }
}

/* Opção 16 - descarta a memória e recarrega do banco */
static void opcaoCarregarBanco(sqlite3 *db, Produto produtos[], int *quantidade,
                               int setores[][SETOR_COLUNAS])
{
    printf("\nOs dados em memória serão substituídos pelos dados do banco.\n");
    if (!confirmar("Deseja continuar?"))
    {
        printf("Operação cancelada.\n");
        return;
    }

    if (carregarDados(db, produtos, quantidade, setores))
    {
        printf("%d produto(s) carregado(s) do banco.\n", *quantidade);
    }
}

/* ===========================================================================
 *  SUBMENU 14 - GERENCIAR SETORES (MATRIZ 5x5)
 * ========================================================================= */
static void menuSetores(sqlite3 *db, const Produto produtos[], int quantidade,
                        int setores[][SETOR_COLUNAS])
{
    int opcao;
    int codigo;

    do
    {
        printf("\n------------- GERENCIAR SETORES -------------\n");
        printf("  1 - Inserir produto em um setor\n");
        printf("  2 - Remover produto de uma posição\n");
        printf("  3 - Mostrar mapa dos setores\n");
        printf("  4 - Consultar ocupação dos setores\n");
        printf("  5 - Localizar produto nos setores\n");
        printf("  0 - Voltar ao menu principal\n");
        printf("---------------------------------------------\n");
        opcao = lerInteiro("Opção: ", 0, 5);

        switch (opcao)
        {
            case 1:
                setorInserir(db, produtos, quantidade, setores);
                break;
            case 2:
                setorRemover(db, setores);
                break;
            case 3:
                mostrarMapaSetores(setores);
                break;
            case 4:
                consultarOcupacaoSetores(setores);
                break;
            case 5:
                codigo = lerInteiro("Código do produto: ", 1, CODIGO_MAXIMO);
                if (buscaLinearCodigo(produtos, quantidade, codigo, NULL) == -1)
                {
                    printf("  [ERRO] Produto com código %d não encontrado.\n", codigo);
                }
                else
                {
                    localizarProdutoNosSetores(setores, codigo);
                }
                break;
            case 0:
                break;
        }

        if (opcao != 0)
        {
            pausar();
        }
    } while (opcao != 0);
}

/* Guarda um produto existente em uma posição livre da matriz + INSERT */
static void setorInserir(sqlite3 *db, const Produto produtos[], int quantidade,
                         int setores[][SETOR_COLUNAS])
{
    int codigo, linha, coluna, resultado;

    if (quantidade == 0)
    {
        printf("\nNenhum produto cadastrado.\n");
        return;
    }

    mostrarMapaSetores(setores);

    codigo = lerInteiro("\nCódigo do produto: ", 1, CODIGO_MAXIMO);
    if (buscaLinearCodigo(produtos, quantidade, codigo, NULL) == -1)
    {
        printf("  [ERRO] Produto com código %d não encontrado.\n", codigo);
        return;
    }

    linha  = lerInteiro("Setor (1 a 5): ", 1, SETOR_LINHAS) - 1;      /* -> 0-based */
    coluna = lerInteiro("Prateleira (1 a 5): ", 1, SETOR_COLUNAS) - 1;

    resultado = inserirProdutoSetor(setores, linha, coluna, codigo);

    if (resultado == -1)
    {
        printf("  [ERRO] Posição S%d-P%d já está ocupada pelo produto %d.\n",
               linha + 1, coluna + 1, setores[linha][coluna]);
        return;
    }
    if (resultado == 0)
    {
        printf("  [ERRO] Posição inválida.\n");
        return;
    }

    if (salvarPosicaoSetor(db, linha, coluna, codigo))
    {
        printf("Produto %d armazenado em S%d-P%d.\n", codigo, linha + 1, coluna + 1);
    }
    else
    {
        removerProdutoSetor(setores, linha, coluna);   /* desfaz na memória */
        printf("[ERRO] Falha ao gravar no banco. Operação desfeita.\n");
    }
}

/* Libera uma posição ocupada da matriz + DELETE */
static void setorRemover(sqlite3 *db, int setores[][SETOR_COLUNAS])
{
    int linha, coluna, codigoAnterior, resultado;

    mostrarMapaSetores(setores);

    linha  = lerInteiro("\nSetor (1 a 5): ", 1, SETOR_LINHAS) - 1;
    coluna = lerInteiro("Prateleira (1 a 5): ", 1, SETOR_COLUNAS) - 1;

    codigoAnterior = setores[linha][coluna];
    resultado = removerProdutoSetor(setores, linha, coluna);

    if (resultado == -1)
    {
        printf("  [ERRO] A posição S%d-P%d já está vazia.\n", linha + 1, coluna + 1);
        return;
    }
    if (resultado == 0)
    {
        printf("  [ERRO] Posição inválida.\n");
        return;
    }

    if (removerPosicaoSetor(db, linha, coluna))
    {
        printf("Produto %d removido da posição S%d-P%d.\n",
               codigoAnterior, linha + 1, coluna + 1);
    }
    else
    {
        setores[linha][coluna] = codigoAnterior;       /* desfaz na memória */
        printf("[ERRO] Falha ao gravar no banco. Operação desfeita.\n");
    }
}
