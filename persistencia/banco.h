/*
 * ============================================================================
 *  Arquivo : persistencia/banco.h
 *  Módulo  : PERSISTÊNCIA (SQLite)
 * ----------------------------------------------------------------------------
 *  Declara as funções que conectam o programa ao banco de dados SQLite
 *  (arquivo "estoque.db") por meio da biblioteca oficial libsqlite3.
 *
 *  O banco serve APENAS para guardar os dados entre execuções. Toda busca
 *  e ordenação é feita em C, sobre o vetor em memória (requisito do PDF).
 *
 *  Fluxo de uso:
 *    abrirBanco()  -> criarTabelas() -> listarBanco()/carregarSetores()
 *    ... a cada alteração: salvarProduto() / atualizarProduto() /
 *        removerProduto() / salvarPosicaoSetor() / removerPosicaoSetor()
 *    fecharBanco() ao sair.
 *
 *  Convenção de retorno: 1 = sucesso | 0 = falha (mensagem já exibida).
 * ============================================================================
 */

#ifndef BANCO_H
#define BANCO_H

#include <sqlite3.h>
#include "../models/produto.h"

#define ARQUIVO_BANCO "estoque.db"

/* ---- Conexão --------------------------------------------------------- */

/* Abre (ou cria) o arquivo do banco. 'db' é um PONTEIRO PARA PONTEIRO:
 * a função preenche a variável sqlite3* que pertence ao main(). */
int  abrirBanco(sqlite3 **db, const char *arquivo);

/* Encerra a conexão e libera os recursos da biblioteca. */
void fecharBanco(sqlite3 *db);

/* Executa o CREATE TABLE IF NOT EXISTS das tabelas do sistema. */
int  criarTabelas(sqlite3 *db);

/* ---- CRUD de produtos ------------------------------------------------ */

int  salvarProduto(sqlite3 *db, const Produto *p);       /* INSERT */
int  atualizarProduto(sqlite3 *db, const Produto *p);    /* UPDATE */
int  removerProduto(sqlite3 *db, int codigo);            /* DELETE */

/* SELECT: carrega todos os produtos do banco para o vetor em memória.
 * 'quantidade' é passada por referência e recebe o total carregado. */
int  listarBanco(sqlite3 *db, Produto produtos[], int *quantidade);

/* ---- Matriz de setores ----------------------------------------------- */

/* linha e coluna são 0-based (índices da matriz em C) */
int  salvarPosicaoSetor(sqlite3 *db, int linha, int coluna, int codigo);
int  removerPosicaoSetor(sqlite3 *db, int linha, int coluna);
int  carregarSetores(sqlite3 *db, int setores[][SETOR_COLUNAS]);

/* ---- Sincronização completa ----------------------------------------- */

/* Regrava TODO o conteúdo da memória no banco dentro de uma transação
 * (BEGIN/COMMIT). Se algo falhar, faz ROLLBACK e o banco fica intacto. */
int  sincronizarTudo(sqlite3 *db, const Produto produtos[], int quantidade,
                     int setores[][SETOR_COLUNAS]);

#endif /* BANCO_H */
