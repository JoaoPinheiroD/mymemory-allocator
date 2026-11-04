# mymemory-allocator

Biblioteca de gerenciamento de memória em C, feita para o **Trabalho 2** de Programação de Software Básico (PUCRS). O programa pede um bloco grande de memória (o *pool*) uma única vez e depois entrega e recebe pedaços dele, sem chamar `malloc` a cada alocação do usuário.

## Estado atual

| Item | Situação |
|---|---|
| `mymemory_init`, `mymemory_alloc`, `mymemory_free`, `mymemory_display`, `mymemory_stats`, `mymemory_cleanup` | implementadas |
| Estratégia **First Fit** | implementada |
| Estratégias **Best Fit** e **Worst Fit** | ainda não implementadas |
| Teste com ~50 blocos de tamanhos variados (seção 2.3 do enunciado) | ainda não incluído no repositório |

## Estrutura do repositório

```
mymemory.h   structs allocation_t e mymemory_t + protótipos das funções
mymemory.c   implementação da biblioteca
main.c       programa de demonstração
```

## Como compilar e executar

```
gcc -g -Wall -Wextra -o teste main.c mymemory.c
./teste
```

Para procurar vazamentos e acessos inválidos com o AddressSanitizer:

```
gcc -g -Wall -Wextra -fsanitize=address,undefined -o teste main.c mymemory.c
./teste
```

Com o Valgrind (compile sem `-fsanitize`):

```
valgrind --leak-check=full ./teste
```

## Como funciona

### Estruturas

```c
typedef struct allocation {
    void *start;              // início do bloco entregue ao usuário (dentro do pool)
    size_t size;              // tamanho do bloco em bytes
    struct allocation *next;  // próximo bloco alocado
} allocation_t;

typedef struct {
    void *pool;               // primeiro byte do pool
    size_t total_size;        // tamanho do pool
    allocation_t *head;       // primeiro nó da lista
} mymemory_t;
```

### Ideia central

- O **pool** é um único bloco contíguo obtido com `malloc` em `mymemory_init`.
- A **lista encadeada** guarda apenas os blocos **ocupados**, ordenada por endereço. Os metadados ficam fora do pool, nos nós `allocation_t`.
- O **espaço livre não é armazenado**: ele é calculado como as *lacunas* entre os blocos ocupados.

```
pool                                                    pool + total_size
 |--lacuna--|[ bloco A ]|--lacuna--|[ bloco B ]|--lacuna--|
```

Com *n* alocações existem sempre *n + 1* lacunas (algumas de tamanho zero):

1. do início do pool até `head->start`;
2. do fim de cada bloco até o início do próximo;
3. do fim do último bloco até `pool + total_size`.

### Funções

| Função | O que faz |
|---|---|
| `mymemory_init(size)` | `malloc` da struct e do pool; `head = NULL`. Retorna `NULL` se algum `malloc` falhar. |
| `mymemory_alloc(m, size)` | Percorre as lacunas (início, meio, final) e usa a **primeira** que comporta `size` (First Fit). Cria um nó e o insere na lista mantendo a ordem. Retorna o endereço dentro do pool, ou `NULL` se `size == 0`, se for maior que o pool ou se nenhuma lacuna comportar. |
| `mymemory_free(m, ptr)` | Procura o nó com `start == ptr`, remove da lista e libera **só o nó** (o pool não é tocado). Se `ptr` for `NULL` ou não for início de uma alocação, não faz nada. |
| `mymemory_display(m)` | Imprime cada bloco alocado e cada lacuna, com endereço de início e tamanho. |
| `mymemory_stats(m)` | Imprime número de alocações, memória alocada, memória livre, maior bloco livre contíguo e número de fragmentos livres. |
| `mymemory_cleanup(m)` | Libera todos os nós, depois o pool e por fim a struct. |

### Decisões de projeto

- **Ordem da lista por endereço:** é o que permite calcular as lacunas com uma única passada.
- **Coalescência automática:** quando um nó sai da lista, as duas lacunas vizinhas viram uma só, sem código extra.
- **Sem fragmentação interna:** cada bloco ocupa exatamente o tamanho pedido, sem arredondamento nem cabeçalho dentro do pool. Consequência: não há alinhamento garantido para blocos de tamanho ímpar.
- **Fragmentação externa:** ocorre, como em qualquer alocador contíguo. Aparece em `mymemory_stats` quando a memória livre total é grande, mas o maior bloco livre e o número de fragmentos mostram que ela está dividida.
- **Restrição 2.4 do enunciado:** `malloc`/`free` são usados somente para o pool, para `mymemory_t` e para os nós `allocation_t`. Os blocos devolvidos ao usuário são sempre `pool + deslocamento`.

### Complexidade

Sendo *n* o número de alocações ativas: `alloc`, `free`, `display`, `stats` e `cleanup` são O(n); `init` é O(1).

## Exemplo de saída

Saída do `main.c` incluído (os endereços mudam a cada execução):

```
--- 3 alocacoes ---
(alocado) start 0x519000000080 [  128  ]
(alocado) start 0x519000000100 [  256  ]
(alocado) start 0x519000000200 [  100  ]
(nao alocado) start 0x519000000264 [  516  ]
Stats:
Alocacoes ativas      : 3
Memoria alocada       : 484 bytes
Memoria livre         : 516 bytes
Maior bloco livre     : 516 bytes
Fragmentos livres     : 1
--- free(b): lacuna no meio ---
(alocado) start 0x519000000080 [  128  ]
(nao alocado) start 0x519000000100 [  256  ]
(alocado) start 0x519000000200 [  100  ]
(nao alocado) start 0x519000000264 [  516  ]
--- realocacoes ---
(alocado) start 0x519000000080 [  100  ]
(nao alocado) start 0x5190000000e4 [  28  ]
(alocado) start 0x519000000100 [  200  ]
(nao alocado) start 0x5190000001c8 [  56  ]
(alocado) start 0x519000000200 [  100  ]
(nao alocado) start 0x519000000264 [  516  ]
--- free(c) ---
Stats:
Alocacoes ativas      : 2
Memoria alocada       : 300 bytes
Memoria livre         : 700 bytes
Maior bloco livre     : 672 bytes
Fragmentos livres     : 2
```

O que a demonstração mostra:

1. Três alocações de 128, 256 e 100 bytes ficam coladas no início do pool.
2. `free(b)` abre uma lacuna de 256 bytes no meio.
3. Um `alloc(200)` reaproveita essa lacuna (First Fit), e depois de `free(a)` um `alloc(100)` usa a lacuna **antes** do primeiro bloco.
4. Depois de `free(c)`, as lacunas se unem e o maior bloco livre passa a 672 bytes.

## Verificação

A implementação First Fit foi conferida com testes que não fazem parte deste repositório:

- um autoteste com casos de borda (`alloc(0)`, pedido maior que o pool, pool cheio, `free(NULL)`, ponteiro inválido, `free` duplo, `cleanup` com lista vazia e com alocações ativas) e um teste de estresse de 200 mil operações;
- um teste diferencial que compara o alocador com um modelo independente (mapa byte a byte do pool), incluindo todas as sequências de 6 operações em pools de 5 e 8 bytes, as 720 ordens de `free` de 6 blocos e 450 execuções aleatórias.

Esses testes encontraram e levaram à correção de dois bugs: `mymemory_cleanup` quebrava com a lista vazia e deixava o último nó vazar, e `mymemory_stats` calculava errado o maior bloco livre quando a maior lacuna não era a final. Depois das correções, todos passaram com o AddressSanitizer e o UBSan ativos, sem relatórios.

## Limitações conhecidas

- Apenas First Fit está implementado.
- `free` e `alloc` percorrem a lista linearmente, O(n).
- Não há alinhamento de endereços além do que decorre dos tamanhos pedidos.
- A biblioteca não é segura para uso em várias threads.

## Uso de ferramentas de IA

Conforme exigido no enunciado: o código foi escrito pelos integrantes, e uma ferramenta de IA (Claude) foi usada como apoio. Ela explicou conceitos (lacunas, fragmentação, casts de ponteiro), revisou o código enviado e apontou bugs, e gerou os testes descritos na seção "Verificação". A lógica de `init`, `alloc`, `free`, `display`, `stats` e `cleanup` foi escrita pelos integrantes e corrigida a partir das revisões.

## Referências

- Enunciado do Trabalho 2 de Programação de Software Básico (PUCRS).
