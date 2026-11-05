/*
 * ============================================================================
 *  Arquivo : persistencia/banco.c
 *  Módulo  : PERSISTÊNCIA (SQLite) - implementação
 * ----------------------------------------------------------------------------
 *  Principais funções da API do SQLite usadas aqui:
 *
 *    sqlite3_open()          abre/cria o arquivo .db e devolve a conexão
 *    sqlite3_close()         encerra a conexão
 *    sqlite3_exec()          executa SQL simples, sem parâmetros
 *    sqlite3_prepare_v2()    compila um SQL com marcadores "?"
 *    sqlite3_bind_int/
 *      _double/_text()       preenche cada "?" com um valor do C
 *    sqlite3_step()          executa o comando (ou avança para a próxima
 *                            linha do resultado de um SELECT)
 *    sqlite3_column_*()      lê as colunas da linha atual do SELECT
 *    sqlite3_finalize()      libera o comando preparado
 *    sqlite3_errmsg()        texto do último erro ocorrido
 *
 *  Por que "prepared statements" (prepare + bind)?
 *    Os valores são enviados separados do texto SQL. Assim um nome como
 *    "Caixa d'agua" não quebra o comando e não há risco de SQL Injection.
 * ============================================================================
 */

#include <stdio.h>
#include <string.h>

#include "banco.h"

/* ---------------------------------------------------------------------------
 *  SQL de criação das tabelas (mesmo conteúdo de sql/criar_tabelas.sql)
 * ------------------------------------------------------------------------- */
static const char *SQL_CRIAR_TABELAS =
    "CREATE TABLE IF NOT EXISTS produtos ("
    "    codigo     INTEGER PRIMARY KEY,"
    "    nome       TEXT    NOT NULL,"
    "    preco      REAL    NOT NULL,"
    "    quantidade INTEGER NOT NULL"
    ");"
    "CREATE TABLE IF NOT EXISTS setores ("
    "    linha          INTEGER NOT NULL,"
    "    coluna         INTEGER NOT NULL,"
    "    codigo_produto INTEGER NOT NULL,"
    "    PRIMARY KEY (linha, coluna),"
    "    FOREIGN KEY (codigo_produto) REFERENCES produtos(codigo)"
    ");";

/* ---------------------------------------------------------------------------
 *  Funções internas (static = visíveis apenas neste arquivo)
 * ------------------------------------------------------------------------- */

/*
 * executarSQL
 * Executa um ou mais comandos SQL sem parâmetros (CREATE, BEGIN, COMMIT...).
 */
static int executarSQL(sqlite3 *db, const char *sql)
{
    char *mensagemErro = NULL;

    if (sqlite3_exec(db, sql, NULL, NULL, &mensagemErro) != SQLITE_OK)
    {
        printf("  [BANCO] Erro ao executar SQL: %s\n",
               mensagemErro != NULL ? mensagemErro : "desconhecido");
        sqlite3_free(mensagemErro);    /* memória alocada pelo SQLite */
        return 0;
    }
    return 1;
}

/*
 * prepararComando
 * Compila o SQL em um "statement" pronto para receber parâmetros.
 * 'stmt' é ponteiro para ponteiro: a função preenche o sqlite3_stmt*
 * de quem chamou.
 */
static int prepararComando(sqlite3 *db, const char *sql, sqlite3_stmt **stmt)
{
    if (sqlite3_prepare_v2(db, sql, -1, stmt, NULL) != SQLITE_OK)
    {
        printf("  [BANCO] Erro ao preparar comando: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    return 1;
}

/*
 * executarComando
 * Executa um INSERT/UPDATE/DELETE já preparado e com parâmetros ligados,
 * e libera o statement. Retorna 1 se o banco respondeu SQLITE_DONE.
 */
static int executarComando(sqlite3 *db, sqlite3_stmt *stmt)
{
    int resultado = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE)
    {
        printf("  [BANCO] Erro ao executar comando: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    return 1;
}

/* ===========================================================================
 *  CONEXÃO
 * ========================================================================= */

/*
 * abrirBanco
 * sqlite3_open() cria o arquivo automaticamente se ele não existir.
 * Também liga a verificação de chaves estrangeiras (desligada por padrão
 * no SQLite), garantindo que a tabela 'setores' só referencie produtos
 * existentes.
 */
int abrirBanco(sqlite3 **db, const char *arquivo)
{
    if (sqlite3_open(arquivo, db) != SQLITE_OK)
    {
        printf("  [BANCO] Não foi possível abrir '%s': %s\n", arquivo, sqlite3_errmsg(*db));
        sqlite3_close(*db);
        *db = NULL;
        return 0;
    }

    if (!executarSQL(*db, "PRAGMA foreign_keys = ON;"))
    {
        sqlite3_close(*db);
        *db = NULL;
        return 0;
    }
    return 1;
}

void fecharBanco(sqlite3 *db)
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }
}

int criarTabelas(sqlite3 *db)
{
    return executarSQL(db, SQL_CRIAR_TABELAS);
}

/* ===========================================================================
 *  CRUD DE PRODUTOS
 * ========================================================================= */

/*
 * salvarProduto  ->  INSERT
 * Cada "?" é numerado a partir de 1 e preenchido com sqlite3_bind_*.
 */
int salvarProduto(sqlite3 *db, const Produto *p)
{
    sqlite3_stmt *stmt;
    const char   *sql =
        "INSERT INTO produtos (codigo, nome, preco, quantidade) VALUES (?, ?, ?, ?);";

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, p->codigo);
    sqlite3_bind_text(stmt, 2, p->nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, p->preco);
    sqlite3_bind_int(stmt, 4, p->quantidade);

    return executarComando(db, stmt);
}

/*
 * atualizarProduto  ->  UPDATE
 * Atualiza nome, preço e quantidade do produto identificado pelo código.
 */
int atualizarProduto(sqlite3 *db, const Produto *p)
{
    sqlite3_stmt *stmt;
    const char   *sql =
        "UPDATE produtos SET nome = ?, preco = ?, quantidade = ? WHERE codigo = ?;";

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, p->nome, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, p->preco);
    sqlite3_bind_int(stmt, 3, p->quantidade);
    sqlite3_bind_int(stmt, 4, p->codigo);

    if (!executarComando(db, stmt))
    {
        return 0;
    }

    /* sqlite3_changes() informa quantas linhas o último comando alterou */
    if (sqlite3_changes(db) == 0)
    {
        printf("  [BANCO] Produto %d não existe no banco.\n", p->codigo);
        return 0;
    }
    return 1;
}

/*
 * removerProduto  ->  DELETE
 * Primeiro libera as posições da matriz de setores que apontam para o
 * produto (por causa da chave estrangeira) e depois apaga o produto.
 * As duas exclusões ficam numa transação: ou ambas acontecem, ou nenhuma.
 */
int removerProduto(sqlite3 *db, int codigo)
{
    sqlite3_stmt *stmt;

    if (!executarSQL(db, "BEGIN TRANSACTION;"))
    {
        return 0;
    }

    /* 1) DELETE nos setores */
    if (!prepararComando(db, "DELETE FROM setores WHERE codigo_produto = ?;", &stmt))
    {
        executarSQL(db, "ROLLBACK;");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, codigo);
    if (!executarComando(db, stmt))
    {
        executarSQL(db, "ROLLBACK;");
        return 0;
    }

    /* 2) DELETE do produto */
    if (!prepararComando(db, "DELETE FROM produtos WHERE codigo = ?;", &stmt))
    {
        executarSQL(db, "ROLLBACK;");
        return 0;
    }
    sqlite3_bind_int(stmt, 1, codigo);
    if (!executarComando(db, stmt))
    {
        executarSQL(db, "ROLLBACK;");
        return 0;
    }

    return executarSQL(db, "COMMIT;");
}

/*
 * listarBanco  ->  SELECT
 * Lê linha por linha do resultado e copia cada uma para uma struct
 * Produto dentro do vetor. Não usamos ORDER BY de propósito: a ordenação
 * é responsabilidade do Bubble Sort implementado em C.
 */
int listarBanco(sqlite3 *db, Produto produtos[], int *quantidade)
{
    sqlite3_stmt        *stmt;
    const unsigned char *nome;
    Produto             *p;
    int                  resultado;
    const char          *sql = "SELECT codigo, nome, preco, quantidade FROM produtos;";

    *quantidade = 0;

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    /* sqlite3_step devolve SQLITE_ROW enquanto houver linhas para ler */
    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (*quantidade >= MAX_PRODUTOS)
        {
            printf("  [BANCO] Aviso: o banco possui mais de %d produtos; "
                   "apenas os primeiros foram carregados.\n", MAX_PRODUTOS);
            break;
        }

        p = &produtos[*quantidade];   /* ponteiro para a próxima posição livre */

        p->codigo = sqlite3_column_int(stmt, 0);

        nome = sqlite3_column_text(stmt, 1);
        strncpy(p->nome, nome != NULL ? (const char *) nome : "", TAM_NOME - 1);
        p->nome[TAM_NOME - 1] = '\0'; /* garante o terminador da string */

        p->preco      = (float) sqlite3_column_double(stmt, 2);
        p->quantidade = sqlite3_column_int(stmt, 3);

        (*quantidade)++;
    }

    if (resultado != SQLITE_ROW && resultado != SQLITE_DONE)
    {
        printf("  [BANCO] Erro ao ler produtos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

/* ===========================================================================
 *  MATRIZ DE SETORES
 *  No banco, linha e coluna são gravadas de 1 a 5 (mais legível para
 *  quem consulta o .db). Em C, a matriz usa índices de 0 a 4.
 * ========================================================================= */

int salvarPosicaoSetor(sqlite3 *db, int linha, int coluna, int codigo)
{
    sqlite3_stmt *stmt;
    const char   *sql =
        "INSERT OR REPLACE INTO setores (linha, coluna, codigo_produto) VALUES (?, ?, ?);";

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, linha + 1);
    sqlite3_bind_int(stmt, 2, coluna + 1);
    sqlite3_bind_int(stmt, 3, codigo);

    return executarComando(db, stmt);
}

int removerPosicaoSetor(sqlite3 *db, int linha, int coluna)
{
    sqlite3_stmt *stmt;
    const char   *sql = "DELETE FROM setores WHERE linha = ? AND coluna = ?;";

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    sqlite3_bind_int(stmt, 1, linha + 1);
    sqlite3_bind_int(stmt, 2, coluna + 1);

    return executarComando(db, stmt);
}

/*
 * carregarSetores  ->  SELECT
 * Reconstrói a matriz em memória a partir da tabela 'setores'.
 * Posições fora do intervalo válido são ignoradas por segurança.
 */
int carregarSetores(sqlite3 *db, int setores[][SETOR_COLUNAS])
{
    sqlite3_stmt *stmt;
    int           linha, coluna, codigo;
    int           resultado;
    const char   *sql = "SELECT linha, coluna, codigo_produto FROM setores;";

    inicializarSetores(setores);

    if (!prepararComando(db, sql, &stmt))
    {
        return 0;
    }

    while ((resultado = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        linha  = sqlite3_column_int(stmt, 0) - 1;   /* 1-based -> 0-based */
        coluna = sqlite3_column_int(stmt, 1) - 1;
        codigo = sqlite3_column_int(stmt, 2);

        if (linha >= 0 && linha < SETOR_LINHAS && coluna >= 0 && coluna < SETOR_COLUNAS)
        {
            setores[linha][coluna] = codigo;
        }
    }

    sqlite3_finalize(stmt);

    if (resultado != SQLITE_DONE)
    {
        printf("  [BANCO] Erro ao ler setores: %s\n", sqlite3_errmsg(db));
        return 0;
    }
    return 1;
}

/* ===========================================================================
 *  SINCRONIZAÇÃO COMPLETA
 * ========================================================================= */

/*
 * sincronizarTudo
 * Faz o banco ficar idêntico à memória:
 *   BEGIN -> apaga tudo -> INSERT de cada produto -> INSERT de cada
 *   posição ocupada da matriz -> COMMIT.
 * Se qualquer passo falhar, ROLLBACK desfaz tudo e o banco volta ao
 * estado anterior (propriedade de ATOMICIDADE das transações).
 */
int sincronizarTudo(sqlite3 *db, const Produto produtos[], int quantidade,
                    int setores[][SETOR_COLUNAS])
{
    int i, j;

    if (!executarSQL(db, "BEGIN TRANSACTION;"))
    {
        return 0;
    }

    /* a ordem importa: 'setores' referencia 'produtos' (chave estrangeira) */
    if (!executarSQL(db, "DELETE FROM setores; DELETE FROM produtos;"))
    {
        executarSQL(db, "ROLLBACK;");
        return 0;
    }

    for (i = 0; i < quantidade; i++)
    {
        if (!salvarProduto(db, &produtos[i]))
        {
            executarSQL(db, "ROLLBACK;");
            return 0;
        }
    }

    for (i = 0; i < SETOR_LINHAS; i++)
    {
        for (j = 0; j < SETOR_COLUNAS; j++)
        {
            if (setores[i][j] != POSICAO_VAZIA &&
                !salvarPosicaoSetor(db, i, j, setores[i][j]))
            {
                executarSQL(db, "ROLLBACK;");
                return 0;
            }
        }
    }

    return executarSQL(db, "COMMIT;");
}
