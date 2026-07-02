# Documentação do módulo correcao_erro

# Módulo `correcao_erro`

Responsável: Pessoa 3.

Implementa a correção de erros de bit (até 3 bits) sobre o bloco `payload + CRC`,
usando **Código de Hamming(8,4) com SECDED** (Single Error Correction, Double Error Detection).

## Funções (conforme `Contratos.md`)

```cpp
void codificarCorrecao(const uint8_t* payload_in, uint8_t tam_in, uint8_t* buffer_out, uint8_t* tam_out);
int  decodificarCorrecao(uint8_t* buffer_in_out, uint8_t tam_in, uint8_t* tam_out);
```

- `codificarCorrecao`: recebe o payload (já com CRC anexado, vindo da Pessoa 4) e gera
  em `buffer_out` uma versão com redundância. Cada byte de entrada vira **2 bytes** de saída
  (um bloco Hamming pra cada nibble). `tam_out = 2 * tam_in`.
- `decodificarCorrecao`: recebe o buffer com redundância, corrige os erros que conseguir
  diretamente no próprio buffer, e:
  - retorna `0` a `3` → quantidade de bits corrigidos com sucesso;
  - retorna `-1` → detectou erro grave demais (2+ bits no mesmo bloco de 7 bits), payload
    não é confiável e deve ser descartado pelo `.ino` principal.
  - `tam_out` é atualizado com o tamanho do payload já sem a redundância (`tam_in / 2`).

## Como o algoritmo funciona (resumo)

Cada nibble (4 bits) do payload vira 1 byte de saída:
- 4 bits de dados
- 3 bits de paridade de Hamming (corrigem 1 erro e apontam a posição exata via síndrome)
- 1 bit de paridade geral sobre o bloco inteiro (permite diferenciar "1 erro" de "2 erros")

No receptor: se só 1 bit mudou dentro do bloco, o algoritmo acha a posição exata pela
síndrome e inverte o bit sozinho. Se 2 bits mudaram no mesmo bloco, o algoritmo percebe
que não pode confiar na correção e retorna erro (`-1`) em vez de "corrigir" errado.

Como cada byte do payload gera 2 blocos independentes, o sistema consegue corrigir vários
bits errados espalhados pela mensagem — desde que não caiam 2 no mesmo bloco de 8 bits.

## Como testar

### Opção 1 — Direto no PC, sem Arduino (mais rápido pra debugar)
```bash
g++ -Wall -Wextra main_teste.cpp correcao_erro.cpp -o teste
./teste
```
(o `main_teste.cpp` fica fora da entrega final, é só ferramenta de desenvolvimento)

### Opção 2 — No Arduino IDE / Tinkercad
Use a pasta `correcao_erro_teste/`, que contém um sketch `.ino` isolado com os mesmos
testes, imprimindo os resultados no Monitor Serial (9600 baud). Esse sketch não usa
nenhum pino (LED/LDR) porque este módulo não depende de hardware — é só manipulação
de bytes em memória.

## Casos de teste cobertos

| Cenário | Esperado |
|---|---|
| Payload sem nenhum erro | 0 bits corrigidos, dado recuperado igual |
| 1 bit invertido em 1 bloco | 1 bit corrigido, dado recuperado igual |
| 3 bits invertidos, cada um em bloco diferente | 3 bits corrigidos, dado recuperado igual |
| 2 bits invertidos no mesmo bloco | retorna -1 (falha detectada, não corrigível) |
| Payload máximo (66 bytes = 64 da msg + 2 do CRC) | codifica pra 132 bytes (cabe nos 256 do buffer) sem erro |

Todos os casos acima foram testados e passaram (ver `relatorio.tex` para prints/evidências).

## Limitações conhecidas

- Se 2 erros caírem no mesmo bloco de 8 bits (mesmo byte codificado), o algoritmo **detecta**
  mas não corrige — retorna `-1`, e cabe ao `.ino` descartar o pacote.
- Não foi testado ainda em hardware físico (só simulação em software / Tinkercad), pendente
  de validação na integração final com os outros módulos.
