# Sistema de Controle de Estoque

Projeto Integrador de **Algoritmos e Programação II** (Engenharia da Computação, 2º semestre de 2026).

Aplicação de console em **linguagem C** que gerencia produtos e movimentações de estoque, com persistência em **SQLite**.

## Integrantes

| Nome | RA |
|------|----|
| _Nicolas De Souza Scapolan Lamas_ | _10738039_ |
| _Enzo Legat Vianna_ | _10737544_ |
| _Mateus Di Petta Andrade_ | _10772062_ |
| _Caua Tavares De Macedo_ | _10771927_ |
| _Vinicius Cutolo Silva_ | _10738875_ |
| _Pedro Henrique Souza Chaves_ | _10370835_ |


---

## 1. Objetivo e descrição do problema

Pequenos comércios precisam saber **o que têm em estoque, quanto vale esse estoque, o que está acabando e onde cada item está guardado**. O sistema resolve isso com:

- cadastro, alteração, exclusão e listagem de produtos;
- pesquisa por nome (**busca linear**) e por código (**busca binária**);
- ordenação por nome, preço ou quantidade (**Bubble Sort**);
- entrada e saída de estoque com validação de regras;
- relatório de produtos com **baixo estoque** (menos de 10 unidades);
- cálculo do **valor total do estoque** (`preço × quantidade`);
- **mapa do armazém** em uma matriz 5×5 (setores × prateleiras);
- **persistência em banco SQLite**: os dados continuam disponíveis depois que o programa é fechado.

---

## 2. Estrutura do projeto

```
projeto/
├── main.c                    → menu, fluxo do programa e sincronização memória ↔ banco
├── models/
│   ├── produto.h             → struct Produto, constantes e protótipos
│   └── produto.c             → entrada validada, CRUD em memória, estoque, relatórios, matriz
├── algoritmos/
│   ├── busca.h / busca.c     → busca linear (nome e código) e busca binária (código)
│   └── ordenacao.h / .c      → Bubble Sort com critérios (código, nome, preço, quantidade)
├── persistencia/
│   ├── banco.h / banco.c     → conexão SQLite e INSERT / SELECT / UPDATE / DELETE
├── sql/
│   └── criar_tabelas.sql     → script de criação das tabelas (+ dados de exemplo opcionais)
└── README.md
```

### Responsabilidade de cada módulo

| Módulo | O que faz | Conhece o banco? |
|--------|-----------|:---:|
| `main.c` | Exibe os menus, chama os módulos e, após cada alteração na memória, chama o banco para gravar. Se o banco falhar, desfaz a alteração ou avisa. | Sim |
| `models/produto` | Define a `struct Produto`. Lê dados do teclado com validação, manipula o **vetor** (cadastrar, alterar, excluir), controla entrada/saída, gera relatórios e manipula a **matriz** de setores. | Não |
| `algoritmos/busca` | Busca linear e busca binária implementadas à mão. | Não |
| `algoritmos/ordenacao` | Bubble Sort implementado à mão. | Não |
| `persistencia/banco` | Toda a comunicação com o SQLite. | Sim |

Os algoritmos não dependem do banco, e o banco não decide nada sobre buscas ou ordenação. O SQL serve só para guardar e recuperar os dados.

### Menu principal

```
  1 - Cadastrar Produto              9 - Ordenar por Quantidade
  2 - Alterar Produto               10 - Entrada de Estoque
  3 - Excluir Produto               11 - Saída de Estoque
  4 - Listar Produtos               12 - Produtos com Baixo Estoque
  5 - Pesquisar por Nome            13 - Valor Total do Estoque
  6 - Pesquisar por Código          14 - Gerenciar Setores (submenu)
  7 - Ordenar por Nome              15 - Salvar no Banco
  8 - Ordenar por Preço             16 - Carregar do Banco
                                     0 - Sair
```

---

## 3. Algoritmos implementados

Todos foram escritos em C pelo grupo. Não usamos `qsort`, `bsearch`, `strstr`, `ORDER BY` nem `WHERE nome LIKE` para essas tarefas.

### 3.1 Busca linear: `buscaLinearNome()` e `buscaLinearCodigo()` em `algoritmos/busca.c`

Percorre o vetor **do primeiro ao último elemento**, comparando um por um.

- **Por nome (opção 5):** encontra **todos** os produtos cujo nome *contém* o texto digitado, sem diferenciar maiúsculas de minúsculas (`"gam"` encontra `"Mouse Gamer"`). A comparação de substring é feita pela função manual `contemTexto()`.
- **Por código:** usada para validar **código duplicado** no cadastro e para localizar produtos em alterar, excluir, entrada e saída. A busca linear funciona com o vetor em qualquer ordem, e o usuário pode ter ordenado por nome ou preço antes.
- **Complexidade:** O(n). Com 100 produtos, até 100 comparações.

### 3.2 Busca binária: `buscaBinariaCodigo()` em `algoritmos/busca.c`

Usada na **opção 6 (Pesquisar por Código)**.

1. **Pré-condição:** o vetor precisa estar **ordenado por código**. Antes da busca, `estaOrdenadoPorCodigo()` confere a ordem e, se ela não estiver correta, o vetor é ordenado com Bubble Sort.
2. O algoritmo usa dois limites, `inicio` e `fim`, e compara o código procurado com o elemento do **meio**:
   - se for igual, encontrou;
   - se o código do meio for **menor**, o alvo só pode estar à direita, então `inicio = meio + 1`;
   - se for **maior**, o alvo só pode estar à esquerda, então `fim = meio - 1`.
3. Quando `inicio > fim`, o código não existe.

**Por que precisa de dados ordenados?** Ao descartar uma metade, o algoritmo assume que *todos* os elementos daquela metade são menores (ou maiores) que o do meio. Isso só vale se o vetor estiver ordenado. Em um vetor desordenado, ele descartaria a metade onde o produto está.

- **Complexidade:** O(log n). Com 100 produtos, **no máximo 7 comparações**.
- O programa mostra quantas comparações foram feitas, o que ajuda a demonstrar a diferença em relação à busca linear.

### 3.3 Ordenação com Bubble Sort: `bubbleSort()` em `algoritmos/ordenacao.c`

Usada nas opções 7, 8 e 9 e antes da busca binária.

- Compara **pares vizinhos** (`j` e `j+1`) e **troca** os dois quando estão fora de ordem.
- A cada passada, o maior elemento ainda desordenado "borbulha" até o fim. Por isso o laço interno vai até `quantidade - 1 - i`.
- **Otimização:** se uma passada termina **sem nenhuma troca**, o vetor já está ordenado e o algoritmo para (flag `trocou`).
- **Um único algoritmo atende todos os critérios.** A função interna `deveTrocar()` recebe o critério (`ORDENAR_POR_NOME`, `_PRECO`, `_QUANTIDADE` ou `_CODIGO`) e decide se os dois produtos estão fora de ordem.
- A ordenação por nome usa `compararNomes()`, uma comparação alfabética manual que ignora maiúsculas e minúsculas.
- **Complexidade:** O(n²) no pior caso e O(n) no melhor caso (vetor já ordenado). O algoritmo é **estável**.

```
Exemplo: quantidades [5, 3, 8, 1]
passada 1: [3, 5, 8, 1] → [3, 5, 1, 8]     (8 chegou ao fim)
passada 2: [3, 1, 5, 8]                     (5 no lugar)
passada 3: [1, 3, 5, 8]                     (ordenado)
```

---

## 4. Uso dos conceitos da disciplina

### 4.1 Structs

A entidade principal é representada por uma struct (`models/produto.h`):

```c
typedef struct
{
    int   codigo;
    char  nome[100];
    float preco;
    int   quantidade;
} Produto;
```

O `enum CriterioOrdenacao` (`ordenacao.h`) define os critérios de ordenação de forma legível.

### 4.2 Arrays

- **Vetor de structs:** `Produto produtos[MAX_PRODUTOS];` em `main.c` guarda todos os produtos em memória (limite de 100).
- **Strings** (`char nome[100]`) são vetores de caracteres percorridos manualmente na busca e na comparação de nomes.
- **Vetor de índices:** `int indices[MAX_PRODUTOS]` recebe as posições encontradas pela busca linear por nome.
- **Exclusão:** os elementos à direita do excluído são deslocados uma posição para a esquerda.

### 4.3 Ponteiros e passagem por referência

| Onde | Para quê |
|------|----------|
| `cadastrarProduto(Produto produtos[], int *quantidade)` | Incrementa o contador que pertence ao `main` com `(*quantidade)++`. |
| `excluirProduto(..., int *quantidade, int *codigoExcluido)` | Decrementa o contador e **devolve** o código excluído ao `main`. |
| `alterarProduto`, `entradaEstoque`, `saidaEstoque` | `Produto *p = &produtos[indice];` altera o produto direto no vetor com `p->quantidade += ...`. |
| `trocarProdutos(Produto *a, Produto *b)` | Troca dois elementos do vetor no Bubble Sort. |
| `deveTrocar(const Produto *a, const Produto *b, ...)` | Compara sem copiar as structs inteiras. |
| `exibirProduto(const Produto *p)` | Leitura somente com `const`, sem cópia. |
| `buscaBinariaCodigo(..., int *comparacoes)` | Devolve um segundo resultado (número de comparações) por referência. |
| `abrirBanco(sqlite3 **db, ...)` | **Ponteiro para ponteiro**: a função preenche a variável `sqlite3 *db` do `main`. |
| `listarBanco(..., int *quantidade)` | Informa ao `main` quantos produtos vieram do banco. |
| Vetores e matrizes como parâmetro | Em C, um vetor passado para uma função é o endereço do primeiro elemento. Por isso o `bubbleSort` ordena o vetor original. |

### 4.4 Matriz do estoque: `int setores[5][5]`

O tema Estoque não exige matriz naturalmente, então ela representa o **mapa físico do armazém**:

- **linhas** são os **setores** S1 a S5;
- **colunas** são as **prateleiras** P1 a P5;
- cada posição guarda o **código do produto** ali armazenado (`0` = posição livre).

Submenu **14 - Gerenciar Setores**:

| Opção | Função | O que demonstra |
|-------|--------|-----------------|
| 1 - Inserir produto em um setor | `inserirProdutoSetor()` | Valida se o produto existe, se a posição é válida e se está livre. |
| 2 - Remover produto de uma posição | `removerProdutoSetor()` | Libera uma posição ocupada. |
| 3 - Mostrar mapa | `mostrarMapaSetores()` | Percorre a matriz com dois `for` aninhados e desenha a grade. |
| 4 - Consultar ocupação | `consultarOcupacaoSetores()` | Conta posições ocupadas **por linha** e mostra o % de ocupação de cada setor e do total. |
| 5 - Localizar produto | `localizarProdutoNosSetores()` | Varre a matriz procurando o código. |

Exemplo de mapa:

```
=============== MAPA DOS SETORES DO ESTOQUE ===============
            P1       P2       P3       P4       P5
  S1   |     105      101    ----     ----     ----  |
  S2   |     110     ----    ----     ----     ----  |
  ...
```

Ao excluir um produto, `removerProdutoDosSetores()` libera automaticamente todas as posições que ele ocupava. A matriz também é **salva no banco** (tabela `setores`).

### 4.5 Funções e modularização

O `main()` só inicializa, mostra o menu e chama funções. Cada opção do menu tem a sua própria função (`opcaoCadastrar`, `opcaoPesquisarCodigo`, ...). Funções auxiliares internas de cada módulo são `static`, ou seja, visíveis apenas no próprio arquivo.

### 4.6 Validações

| Situação | Como é tratada |
|----------|----------------|
| Texto digitado onde se espera número (`abc`, `12x`) | `lerInteiro()` e `lerFloat()` usam `strtol`/`strtod` e rejeitam qualquer sobra de texto. |
| Opção de menu fora do intervalo | `lerInteiro(msg, minimo, maximo)` repete a pergunta. |
| **Código duplicado** | Busca linear antes de cadastrar. A chave primária no banco também impede duplicatas. |
| **Código inexistente / operação em produto inexistente** | Alterar, excluir, entrada, saída, setores e pesquisa avisam "Produto não encontrado". |
| **Nome vazio** (ou só espaços) | `lerTexto()` não aceita. |
| **Preço negativo** ou zero | `lerPreco()` não aceita. Também aceita vírgula: `12,50`. |
| **Quantidade inválida** (negativa) | Cadastro aceita só ≥ 0. Entrada e saída aceitam só > 0 (0 cancela). |
| **Saída maior que o estoque** | Bloqueada com mensagem mostrando o estoque disponível. |
| Estoque estourando o limite | A entrada é limitada para o total não passar de 1.000.000. |
| Vetor cheio | O cadastro é bloqueado ao atingir 100 produtos. |
| Posição da matriz ocupada / já vazia | Mensagem de erro, sem alteração. |
| Exclusão acidental | Pede confirmação (S/N). |

---

## 5. Persistência: banco de dados SQLite

### 5.1 Tecnologia escolhida

- **Banco:** SQLite 3
- **Biblioteca/driver:** `libsqlite3`, a biblioteca C oficial do SQLite (cabeçalho `<sqlite3.h>`)
- **Arquivo do banco:** `estoque.db`, criado automaticamente na pasta onde o programa é executado

### 5.2 Justificativa da escolha

1. **Não precisa de servidor.** MySQL e PostgreSQL exigem instalar, configurar e manter um serviço rodando, com usuário e senha. O SQLite é uma biblioteca: o banco inteiro é **um arquivo** (`estoque.db`).
2. **A API é em C.** O próprio SQLite é escrito em C, então não é preciso nenhum conector intermediário.
3. **Portável.** Funciona em Windows, Linux e macOS. Para entregar o projeto basta levar o `.db` junto, ou deixar o programa criá-lo.
4. **Suficiente para o problema.** Um sistema de estoque de console tem um único usuário por vez. O SQLite ainda oferece SQL completo, chave primária, chave estrangeira e **transações**.
5. **Fácil de corrigir.** O professor consegue compilar e rodar em outro computador sem configurar servidor.

### 5.3 Estrutura das tabelas

**Tabela `produtos`** (espelho da `struct Produto`)

| Coluna | Tipo | Restrição | Descrição |
|--------|------|-----------|-----------|
| `codigo` | INTEGER | PRIMARY KEY | Código único do produto |
| `nome` | TEXT | NOT NULL | Descrição |
| `preco` | REAL | NOT NULL | Preço unitário (R$) |
| `quantidade` | INTEGER | NOT NULL | Unidades em estoque |

**Tabela `setores`** (persistência da matriz `setores[5][5]`; guarda só as posições ocupadas)

| Coluna | Tipo | Restrição | Descrição |
|--------|------|-----------|-----------|
| `linha` | INTEGER | PK (linha, coluna) | Setor (1 a 5) |
| `coluna` | INTEGER | PK (linha, coluna) | Prateleira (1 a 5) |
| `codigo_produto` | INTEGER | FOREIGN KEY → `produtos(codigo)` | Produto guardado na posição |

O script completo está em [`sql/criar_tabelas.sql`](sql/criar_tabelas.sql).

### 5.4 Como o C conversa com o banco (`persistencia/banco.c`)

| Etapa | Função do projeto | Funções da API SQLite |
|-------|-------------------|-----------------------|
| **Abrir conexão** | `abrirBanco(&db, "estoque.db")` | `sqlite3_open()` cria o arquivo se ele não existir. Em seguida, `PRAGMA foreign_keys = ON`. |
| **Criar tabelas** | `criarTabelas(db)` | `sqlite3_exec()` com `CREATE TABLE IF NOT EXISTS` |
| **INSERT** | `salvarProduto(db, &produto)` | `sqlite3_prepare_v2()` → `sqlite3_bind_int/text/double()` → `sqlite3_step()` → `sqlite3_finalize()` |
| **SELECT** | `listarBanco(db, produtos, &qtd)` | `sqlite3_step()` em laço enquanto retornar `SQLITE_ROW`, lendo com `sqlite3_column_int/text/double()` |
| **UPDATE** | `atualizarProduto(db, &produto)` | prepare/bind/step + `sqlite3_changes()` para confirmar que a linha existia |
| **DELETE** | `removerProduto(db, codigo)` | apaga as posições em `setores` e depois o produto, dentro de uma transação |
| **Fechar conexão** | `fecharBanco(db)` | `sqlite3_close()` |

**Prepared statements:** os comandos usam `?` como marcador, e os valores são ligados com `sqlite3_bind_*`. Os dados vão separados do SQL, então um nome como `Caixa d'agua` não quebra o comando e não há risco de **SQL Injection**.

**Transações:** a opção 15 (`sincronizarTudo`) e a exclusão usam `BEGIN TRANSACTION` / `COMMIT`. Se algum passo falhar, `ROLLBACK` devolve o banco ao estado anterior.

### 5.5 Sincronização memória ↔ banco

```
INÍCIO DO PROGRAMA
  abrirBanco → criarTabelas → listarBanco (SELECT) → vetor produtos[]
                            → carregarSetores (SELECT) → matriz setores[5][5]

DURANTE O USO (sincronização automática a cada alteração)
  Cadastrar          → altera o vetor → salvarProduto     (INSERT)
  Alterar            → altera o vetor → atualizarProduto  (UPDATE)
  Entrada / Saída    → altera o vetor → atualizarProduto  (UPDATE)
  Excluir            → altera o vetor → removerProduto    (DELETE)
  Setores            → altera a matriz → salvarPosicaoSetor / removerPosicaoSetor
  Se o banco falhar  → a alteração em memória é desfeita ou o usuário é avisado

  Ordenar / Pesquisar → só memória (o banco não guarda ordem)

  15 - Salvar no Banco   → regrava tudo em uma transação (sincronização forçada)
  16 - Carregar do Banco → descarta a memória e relê o banco

FIM DO PROGRAMA
  fecharBanco (sqlite3_close)
```

**Como os dados continuam disponíveis depois que o programa é encerrado?** Cada alteração é gravada no arquivo `estoque.db` no momento em que acontece. Na próxima execução, `listarBanco()` faz um `SELECT` e reconstrói o vetor de structs.

---

## 6. Configuração, compilação e execução

Todos os comandos abaixo devem ser executados **dentro da pasta `projeto/`**.

### 6.1 Linux (Ubuntu/Debian)

```bash
# 1. Instalar o compilador e a biblioteca de desenvolvimento do SQLite
sudo apt update
sudo apt install gcc libsqlite3-dev

# 2. Compilar
gcc main.c models/produto.c algoritmos/busca.c algoritmos/ordenacao.c persistencia/banco.c -lsqlite3 -o estoque

# 3. Executar
./estoque
```

> Fedora: `sudo dnf install gcc sqlite-devel` · macOS: `xcode-select --install` (o SQLite já vem no sistema).

### 6.2 Windows: opção A, MSYS2 (recomendada)

1. Instale o [MSYS2](https://www.msys2.org/) e abra o terminal **MSYS2 UCRT64**.
2. Instale o compilador e o SQLite:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sqlite3
   ```
3. Compile e execute:
   ```bash
   gcc main.c models/produto.c algoritmos/busca.c algoritmos/ordenacao.c persistencia/banco.c -lsqlite3 -o estoque.exe
   ./estoque.exe
   ```

### 6.3 Windows: opção B, SQLite "amalgamation" (sem instalar a biblioteca)

O SQLite pode ser compilado junto com o projeto a partir de **um único arquivo `.c`**.

1. Baixe o arquivo **sqlite-amalgamation-XXXXXXX.zip** em <https://www.sqlite.org/download.html>.
2. Copie `sqlite3.c` e `sqlite3.h` para dentro da pasta `projeto/`.
3. Compile com qualquer GCC (MinGW, MSYS2 etc.):
   ```bash
   gcc -c sqlite3.c -o sqlite3.o
   gcc -I. main.c models/produto.c algoritmos/busca.c algoritmos/ordenacao.c persistencia/banco.c sqlite3.o -o estoque.exe
   estoque.exe
   ```
   O `sqlite3.c` é grande, e compilá-lo separado (`-c`) evita recompilá-lo a cada alteração do projeto.

> Os acentos aparecem corretamente no console porque o programa ativa UTF-8 (`SetConsoleOutputCP`) no Windows.

### 6.4 Testando a persistência

1. Execute o programa, cadastre alguns produtos e escolha `0 - Sair`.
2. Execute de novo: a mensagem `N produto(s) carregado(s)` confirma que os dados vieram do `estoque.db`.
3. (Opcional) Inspecione o banco com o [DB Browser for SQLite](https://sqlitebrowser.org/) ou com o terminal:
   ```bash
   sqlite3 estoque.db "SELECT * FROM produtos;"
   ```
4. (Opcional) Para começar com dados de exemplo, descomente os `INSERT` de `sql/criar_tabelas.sql` e rode:
   ```bash
   sqlite3 estoque.db < sql/criar_tabelas.sql
   ```

---

## 7. Roteiro rápido para a apresentação

| Pergunta provável | Onde mostrar no código |
|-------------------|------------------------|
| Como funciona a busca linear? | `buscaLinearNome()` e `contemTexto()`, em `algoritmos/busca.c` |
| Por que a busca binária precisa de dados ordenados? | `buscaBinariaCodigo()` + `opcaoPesquisarCodigo()` em `main.c`, que ordena antes |
| Qual algoritmo de ordenação e como funciona? | `bubbleSort()`, `deveTrocar()` e `trocarProdutos()`, em `algoritmos/ordenacao.c` |
| Onde aparecem os ponteiros? | Tabela da seção 4.3 (`int *quantidade`, `Produto *p`, `sqlite3 **db`) |
| Como o C se conecta ao banco? | `abrirBanco()` e `salvarProduto()`, em `persistencia/banco.c` |
| Como os dados permanecem após encerrar? | Seção 5.5 + `carregarDados()` no início do `main()` |
| Onde está a matriz? | `int setores[5][5]` em `main.c`, funções na seção 6 de `models/produto.c` |

---

## 8. Referências

- SQLite. *An Introduction to the SQLite C/C++ Interface*. <https://www.sqlite.org/cintro.html>
- SQLite. *C/C++ Interface for SQLite Version 3* (referência da API). <https://www.sqlite.org/c3ref/intro.html>
- SQLite. *Prepared statements: `sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`*. <https://www.sqlite.org/c3ref/stmt.html>
- SQLite. *SQLite Foreign Key Support*. <https://www.sqlite.org/foreignkeys.html>
- SQLite. *The SQLite Amalgamation*. <https://www.sqlite.org/amalgamation.html>
- SQLite. *Appropriate Uses For SQLite*. <https://www.sqlite.org/whentouse.html>
- CORMEN, T. H. et al. *Algoritmos: Teoria e Prática*. 3. ed. Rio de Janeiro: Elsevier, 2012.
- FEOFILOFF, P. *Algoritmos em Linguagem C*. Rio de Janeiro: Elsevier, 2009.
- KERNIGHAN, B. W.; RITCHIE, D. M. *C: A Linguagem de Programação*. 2. ed. Rio de Janeiro: Campus, 1989.
- MSYS2. *Getting Started*. <https://www.msys2.org/>
