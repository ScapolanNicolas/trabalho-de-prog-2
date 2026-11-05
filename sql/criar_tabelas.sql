-- ============================================================================
--  Sistema de Controle de Estoque - Script de criação do banco (SQLite)
--  Arquivo: sql/criar_tabelas.sql
-- ----------------------------------------------------------------------------
--  O programa executa estes mesmos comandos automaticamente ao iniciar
--  (função criarTabelas() em persistencia/banco.c). Este script serve para
--  documentação e para criar/inspecionar o banco manualmente:
--
--      sqlite3 estoque.db < sql/criar_tabelas.sql
-- ============================================================================

PRAGMA foreign_keys = ON;

-- ----------------------------------------------------------------------------
--  Tabela de produtos (entidade principal - struct Produto)
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS produtos (
    codigo     INTEGER PRIMARY KEY,   -- código único do produto
    nome       TEXT    NOT NULL,      -- descrição
    preco      REAL    NOT NULL,      -- preço unitário (R$)
    quantidade INTEGER NOT NULL       -- unidades em estoque
);

-- ----------------------------------------------------------------------------
--  Tabela de setores (persistência da matriz int setores[5][5])
--  Cada linha representa UMA posição ocupada da matriz.
--  linha  = setor do armazém   (1 a 5)
--  coluna = prateleira          (1 a 5)
-- ----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS setores (
    linha          INTEGER NOT NULL,
    coluna         INTEGER NOT NULL,
    codigo_produto INTEGER NOT NULL,
    PRIMARY KEY (linha, coluna),
    FOREIGN KEY (codigo_produto) REFERENCES produtos(codigo)
);

-- ----------------------------------------------------------------------------
--  Dados de exemplo (OPCIONAL) - descomente para popular o banco
-- ----------------------------------------------------------------------------
-- INSERT INTO produtos (codigo, nome, preco, quantidade) VALUES
--     (105, 'Teclado Mecanico',     249.90,  15),
--     (101, 'Mouse Gamer',          129.90,   8),
--     (110, 'Monitor 24 polegadas', 899.00,   4),
--     (103, 'Cabo HDMI 2m',          29.90,  50),
--     (108, 'Headset USB',          189.50,  12),
--     (102, 'Webcam Full HD',       219.00,   3);
--
-- INSERT INTO setores (linha, coluna, codigo_produto) VALUES
--     (1, 1, 105),
--     (1, 2, 101),
--     (2, 1, 110),
--     (3, 4, 103);

-- ----------------------------------------------------------------------------
--  Consultas úteis para conferir os dados
-- ----------------------------------------------------------------------------
-- SELECT * FROM produtos;
-- SELECT * FROM setores;
-- SELECT s.linha, s.coluna, p.nome
--   FROM setores s JOIN produtos p ON p.codigo = s.codigo_produto;
