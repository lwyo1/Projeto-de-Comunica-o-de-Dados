# Contratos de Integração e Padrões do Projeto

Este documento define os contratos (interfaces, assinaturas de funções e formatos de dados) que cada integrante do grupo deve respeitar. O objetivo é que cada desenvolvedor possa trabalhar de forma independente. Se todos seguirem este contrato estritamente, o código de cada um irá se encaixar perfeitamente no arquivo principal (`.ino`) sem causar conflitos ou quebrar a lógica dos amiguinhos.

## 1. Arquitetura Modular
O projeto é dividido em módulos. Cada módulo tem sua própria pasta dentro de `TX/src/` e `RX/src/`.
**NUNCA** modifique o código, variáveis globais ou lógicas de outro módulo. A comunicação entre os módulos será feita **estritamente** através da passagem de parâmetros nas funções definidas abaixo.

## 2. Formato do Frame de Dados
Para enviar os dados (máx 64 caracteres), passaremos por um "túnel" de funções. O array de dados vai aumentando conforme as redundâncias são inseridas.
Ordem de empacotamento no TX:
1. `CorrecaoErro_encode` adiciona os bits/bytes de correção no payload.
2. `CRC_calcula` gera o checksum que é anexado ao final.
3. O frame completo é enviado usando o `Codificacao_enviar_XXX()`, precedido pelo `Sincronismo_enviar()`.

## 3. Contratos de Funções por Módulo

Abaixo estão os protótipos de funções que **devem** ser implementados. Não modifique as assinaturas (parâmetros e retornos).

### 3.1. Sincronismo Inicial (Auto-Baud) - Pessoa 2
**Objetivo:** Estabelecer a taxa de transmissão entre emissor e receptor.
- **TX (`TX/src/auto_baud/AutoBaud.h`):** 
  ```cpp
  // Pisca o LED em um padrão de calibração (tipo: 10101010) indicando o início de transmissão.
  void enviarSincronismo(int pin_led, unsigned long tempo_bit);
  ```
- **RX (`RX/src/auto_baud/AutoBaud.h`):**
  ```cpp
  // Bloqueia a execução até ler o padrão de calibração. Retorna o tempo de bit descoberto em microssegundos.
  unsigned long receberSincronismo(int pin_sensor);
  ```

### 3.2. Detecção de Erros (CRC) - Pessoa 4
**Objetivo:** Calcular o CRC para validar a integridade.
- **TX (`TX/src/crc/ModuloCRC.h`):**
  ```cpp
  // Calcula e retorna o valor de CRC sobre o buffer de dados.
  uint16_t calcularCRC(const uint8_t* payload, uint8_t tamanho);
  ```
- **RX (`RX/src/crc/ModuloCRC.h`):**
  ```cpp
  // Calcula o CRC do payload recebido e compara com o crc_recebido no final do frame.
  // Retorna 'true' se estiver válido, 'false' se estiver corrompido.
  bool verificarCRC(const uint8_t* payload, uint8_t tamanho, uint16_t crc_recebido);
  ```

### 3.3. Correção de Erros (até 3 bits) - Pessoa 3
**Objetivo:** Inserir e interpretar algoritmos matemáticos pesados para corrigir até 3 erros.
- **TX (`TX/src/correcao_erro/CorrecaoErro.h`):**
  ```cpp
  // Pega o 'payload_in' original e escreve em 'buffer_out' os dados + bits de paridade.
  // Atualiza 'tamanho_out' com o novo tamanho total (payload + redundância).
  void codificarCorrecao(const uint8_t* payload_in, uint8_t tamanho_in, uint8_t* buffer_out, uint8_t* tamanho_out);
  ```
- **RX (`RX/src/correcao_erro/CorrecaoErro.h`):**
  ```cpp
  // Avalia o buffer, tenta corrigir erros de bit diretamente nele.
  // Retorna a quantidade de bits corrigidos (0 a 3). Retorna -1 se houver erros acima de 3 (falha).
  // Atualiza 'tamanho_out' descartando os bits de paridade e deixando apenas o tamanho do payload real.
  int decodificarCorrecao(uint8_t* buffer_in_out, uint8_t tamanho_in, uint8_t* tamanho_out);
  ```

### 3.4. Codificação de Linha - Pessoas 1, 2 e Eu
**Objetivo:** Modular bits em luz. Cada codificação deve ter um par enviar/receber.

**Exemplo para NRZ-L (Pessoa 1) em `src/nrz_l/NRZL.h`:**
- **TX:** 
  ```cpp
  void enviarFrame_NRZL(const uint8_t* frame, uint8_t tamanho, int pin_led, unsigned long tempo_bit);
  ```
- **RX:** 
  ```cpp
  bool receberFrame_NRZL(uint8_t* frame_out, uint8_t tamanho_esperado, int pin_sensor, unsigned long tempo_bit);
  ```

*A mesma estrutura de funções deve ser reproduzida para `NRZI`, `Manchester` e `ManchesterDiferencial`.*

## 4. Integração no `.ino` Principal
Nós não vamos programar lógicas complexas no `.ino`. O `.ino` do TX será basicamente:
```cpp
// Pseudocódigo TX.ino
String msg = "Ola Marte"; //Ola Mundo diferente (Marte estaria incluso em Mundo ?)
uint8_t buffer_protegido[128];
uint8_t tamanho_protegido = 0;

// Correção de Erro (Adiciona redundância)
codificarCorrecao((uint8_t*)msg.c_str(), msg.length(), buffer_protegido, &tamanho_protegido);

// CRC (Calcula Checksum)
uint16_t crc = calcularCRC(buffer_protegido, tamanho_protegido);
// (anexa o crc ao final do buffer_protegido)

// Sincronismo Inicial
enviarSincronismo(pin_led, TEMPO_BIT);

// Codificação e Transmissão Física 
enviarFrame_NRZL(buffer_protegido, tamanho_protegido + 2, pin_led, TEMPO_BIT);
```
Dessa forma o trabalho de todos se conecta de forma fluida.
