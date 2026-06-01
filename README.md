# Projeto de Comunicação de Dados

Projeto para realizar a comunicação entre dois dispositivos Arduino (Emissor e Receptor) por meio de sinais luminosos, como requisito da disciplina de Comunicação de Dados.

## Divisão da Equipe e Responsabilidades

*   **Pessoa 1:** NRZ-L (Obrigatório) e Manchester Diferencial (Opcional/Bônus).
*   **Pessoa 2:** NRZ-I (Obrigatório) e Sincronismo Inicial / Auto-Baud (Bônus).
*   **Pessoa 3:** Correção de Erros (até 3 bits) (Bônus).
*   **Pessoa 4:** Detecção de Erros via CRC (Obrigatório).
*   **(Eu):** Manchester, estruturação do repositório, definição dos contratos de integração, unificação do código no arquivo `.ino` (menu e chamadas principais) e modelo de testes práticos.

## Arquitetura Modular e Regras de Desenvolvimento

Este projeto deve utiliza o padrão de **projeto modular**. Isso significa que cada membro trabalhará de forma isolada e o código será conectado no final através de interfaces bem definidas.

**Regras:**
1. **Não altere o código do colega.** Toda a comunicação entre módulos se dá pelas assinaturas de funções definidas em `Contratos.md`.
2. **O `.ino` deve ser extremamente enxuto.** O arquivo principal do Arduino só servirá para orquestrar as chamadas. Toda a lógica (matemática do CRC, lógica de codificação, etc.) deve estar nos arquivos `.cpp` da sua respectiva pasta.
3. **Leia o `Contratos.md`:** Ele especifica os parâmetros que sua função deve receber e o que ela deve retornar para que o sistema funcione como um todo.

## Estrutura de Pastas

Dentro das pastas `TX` (Emissor) e `RX` (Receptor), existe a pasta `src/`. Cada desenvolvedor terá uma subpasta específica para o seu módulo.
Exemplo:
```
TX/
└── src/
    ├── auto_baud/      (Pessoa 2)
    ├── crc/            (Pessoa 4)
    ├── correcao_erro/  (Pessoa 3)
    ├── nrz_l/          (Pessoa 1)
    ├── nrz_i/          (Pessoa 2)
    ├── manchester/     (EU)
    └── manchester_diff/(Pessoa 1)
```

Dentro da sua pasta correspondente em `TX` ou `RX`, já deixei criados os templates de:
*   Seu arquivo `.h` (com as assinaturas definidas no Contrato)
*   Seu arquivo `.cpp` (onde você vai programar a lógica)
*   Um `README.md` documentando o seu módulo (escreva como testar e como usar sua parte)
*   Um arquivo `.tex` para o seu relatório final.

## Relatórios e Documentação (LaTeX)

Como parte dos requisitos, e preciso entregar um **Relatório Completo**. Para facilitar a compilação, cada integrante deverá escrever sua parte do relatório no arquivo `.tex` que deixei na sua pasta. Eu, como gerente, usarei a tag `\input{sua_pasta/relatorio.tex}` para unificar tudo no documento final do grupo.

**O seu mini-relatório (.tex) DEVE conter:**
1. **Modelagem realizada:** Explicação da teoria por trás do seu algoritmo (Tipo: como o polinômio do CRC foi escolhido, como a matemática da correção de erro foi aplicada).
2. **Tomadas de decisão e Lógica desenvolvida:** Como o código foi estruturado e a razão de ser feito assim.
3. **Dificuldades encontradas:** Problemas que ocorreram durante a programação e como foram superados.
4. **Pontos positivos e negativos:** Avaliação crítica da sua própria implementação.
5. **Casos de Teste (MUITO IMPORTANTE):** Inserir figuras (prints) mostrando as execuções isoladas do seu módulo, testes de mesa, ou links de vídeos provando que o seu código funciona.

## Simulador e Testes Práticos

Para facilitar os testes de integração do código, montei um ambiente de simulação no Tinkercad contendo o Arduino Emissor (TX) e o Arduino Receptor (RX) já interligados.

**Link do Simulador:** [Tinkercad - Projeto TX/RX Arduino](https://www.tinkercad.com/things/f21a4JdJJI2-txrx-arduino)

Podem duplicar o projeto no Tinkercad e colar o código de vocês para validar a lógica em um ambiente controlado antes de colocar no hardware físico.
