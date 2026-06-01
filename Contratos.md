# Contratos de Integração e Padrões do Projeto

Este documento define os contratos (interfaces, assinaturas de funções e formatos de dados) que cada integrante do grupo deve respeitar. O objetivo é que cada desenvolvedor possa trabalhar de forma independente. Se todos seguirem este contrato estritamente, o código de cada um irá se encaixar perfeitamente no arquivo principal (`.ino`) sem causar conflitos ou quebrar a lógica dos amiguinhos.

## 1. Arquitetura Modular
O projeto é dividido em módulos. Cada módulo tem sua própria pasta dentro de `TX/src/` e `RX/src/`.
**NUNCA** modifique o código, variáveis globais ou lógicas de outro módulo. A comunicação entre os módulos será feita **estritamente** através da passagem de parâmetros nas funções definidas abaixo.

## 2. Formato do Frame de Dados
Para enviar os dados (máx 64 caracteres), passaremos por um "túnel" de funções. O array de dados vai aumentando conforme as redundâncias são inseridas.
Ordem de empacotamento no TX:
1. `CRC_calcula` gera o checksum (geralmente 2 bytes) e o anexa ao final do payload puro.
2. `CorrecaoErro_encode` processa o bloco todo (Payload + CRC) e adiciona os bits/bytes de correção de erro. Dessa forma, o próprio CRC viaja protegido contra ruídos na luz.
3. A comunicação inicia chamando o `Sincronismo_enviar()`.
4. A função de codificação física (`enviarFrame_NRZL()`) envia o frame completo. O primeiro byte emitido DEVE ser o tamanho do frame, para que o RX não fique travado num loop infinito lendo ruído.

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
  // Sugestão: Adicione um timeout máximo (ex: 5 segundos) e retorne 0 se falhar.
  unsigned long receberSincronismo(int pin_sensor);
  ```

### 3.2. Detecção de Erros (CRC) - Pessoa 4
**Objetivo:** Calcular o CRC para validar a integridade.
- **TX (`TX/src/crc/ModuloCRC.h`):**
  ```cpp
  // Calcula e retorna o valor de CRC sobre o buffer de dados.
  // Recomendação: Use o CRC-16-IBM (0x8005, polinômio x^16 + x^15 + x^2 + 1), valor inicial 0xFFFF.
  uint16_t calcularCRC(const uint8_t* payload, uint8_t tam);
  ```
- **RX (`RX/src/crc/ModuloCRC.h`):**
  ```cpp
  // Calcula o CRC do payload recebido e compara com o crc_recebido no final do frame.
  // Retorna 'true' se estiver válido, 'false' se estiver corrompido.
  bool verificarCRC(const uint8_t* payload, uint8_t tam, uint16_t crc_recebido);
  ```

### 3.3. Correção de Erros (até 3 bits) - Pessoa 3
**Objetivo:** Inserir e interpretar algoritmos matemáticos pesados para corrigir até 3 erros.
- **TX (`TX/src/correcao_erro/CorrecaoErro.h`):**
  ```cpp
  // Pega o 'payload_in' original e escreve em 'buffer_out' os dados + bits de paridade.
  // Atualiza 'tam_out' com o novo tamanho total (payload + redundância).
  void codificarCorrecao(const uint8_t* payload_in, uint8_t tam_in, uint8_t* buffer_out, uint8_t* tam_out);
  ```
- **RX (`RX/src/correcao_erro/CorrecaoErro.h`):**
  ```cpp
  // Avalia o buffer, tenta corrigir erros de bit diretamente nele.
  // Retorna a quantidade de bits corrigidos (0 a 3). Retorna -1 se houver erros acima de 3 (falha).
  // Atualiza 'tam_out' descartando os bits de paridade e deixando apenas o tamanho do payload real.
  int decodificarCorrecao(uint8_t* buffer_in_out, uint8_t tam_in, uint8_t* tam_out);
  ```

### 3.4. Codificação de Linha - Pessoas 1, 2 e Eu
**Objetivo:** Modular bits em luz. Cada codificação deve ter um par enviar/receber.

**Exemplo para NRZ-L (Pessoa 1) em `src/nrz_l/NRZL.h`:**
- **TX:** 
  ```cpp
  // OBRIGATÓRIO: A função deve inserir fisicamente um byte de cabeçalho contendo o valor de 'tam' ANTES de transmitir os dados do array 'frame'.
  void enviarFrame_NRZL(const uint8_t* frame, uint8_t tam, int pin_led, unsigned long tempo_bit);
  ```
- **RX:** 
  ```cpp
  // Lê o 1º byte (tamanho N), aloca e lê o restante. Salva no frame_out e atualiza o tam_lido.
  // OBRIGATÓRIO: O timeout de leitura deve ser DINÂMICO e proporcional. Ex: 10 * tempo_bit sem luz = aborta e retorna false.
  bool receberFrame_NRZL(uint8_t* frame_out, uint8_t* tam_lido, int pin_sensor, unsigned long tempo_bit);
  ```

*A mesma estrutura de funções deve ser reproduzida para `NRZI`, `Manchester` e `ManchesterDiferencial`.*

## 4. Integração no `.ino` Principal
Não tera lógicas complexas no `.ino`. O `.ino` será basicamente para orquestrar as chamadas.

### Transmissor
```cpp
String msg = "Ola Marte"; // Futuramente substituído por leitura da Serial
uint8_t payload[64];
msg.getBytes(payload, 64); // Extrai os bytes físicos com tipagem correta
uint8_t tam_payload = msg.length();

uint8_t buffer_crc[66]; // Payload + 2 bytes do CRC
uint8_t buffer_luz[256]; // A correção de erro pode gerar muita redundância. Limite: 256 bytes.
uint8_t tam_final = 0;

// Pessoa 4 atua (Calcula CRC e anexa no final do payload puro)
uint16_t crc = calcularCRC(payload, tam_payload);
memcpy(buffer_crc, payload, tam_payload);
buffer_crc[tam_payload] = (crc >> 8) & 0xFF; // Byte alto
buffer_crc[tam_payload + 1] = crc & 0xFF;    // Byte baixo

// Pessoa 3 atua (Aplica Correção de Erros em TUDO, inclusive no CRC)
codificarCorrecao(buffer_crc, tam_payload + 2, buffer_luz, &tam_final);

// Pessoa 2 atua (Calibra a linha)
enviarSincronismo(PINO_LED, TEMPO_BIT);

// O primeiro byte enviado DEVE ser o 'tam_final' para o RX não travar
enviarFrame_NRZL(buffer_luz, tam_final, PINO_LED, TEMPO_BIT);
```

### Receptor
```cpp
uint8_t buffer_recebido[256]; // Buffer grande para suportar a redundância da correção
uint8_t tam_lido = 0;
uint8_t tam_corrigido = 0;

// Sincroniza e descobre a velocidade
unsigned long tempo_bit = receberSincronismo(PINO_LDR);
if (tempo_bit == 0) return; // Timeout de sincronismo. Cancela a leitura e tenta novamente depois.

// Lê a luz com timeout de segurança dinâmico
if(receberFrame_NRZL(buffer_recebido, &tam_lido, PINO_LDR, tempo_bit)) {
    
    // Tenta corrigir a sujeira da luz
    int erros = decodificarCorrecao(buffer_recebido, tam_lido, &tam_corrigido);

    // Se a correção funcionou e o pacote tem tamanho seguro (evita underflow)
    if(erros != -1 && tam_corrigido >= 2) {
        uint8_t tam_payload = tam_corrigido - 2; // Seguro para isolar o payload
        uint16_t crc_recebido = (buffer_recebido[tam_payload] << 8) | buffer_recebido[tam_payload + 1];
        
        if(verificarCRC(buffer_recebido, tam_payload, crc_recebido)) {
            //Repassa os dados puros para a Serial
        }
    }
}
```
Dessa forma o trabalho de todos se conecta de forma fluida.

## 5. Dicas Extras para Integração (Arduino)
*   **Pinos no `.ino`:** Use `#define PINO_LED` e `#define PINO_LDR` no início do arquivo para que fique padronizado.
*   **Temporização Não-Bloqueante (PROIBIDO USAR DELAY):** Nas funções de codificação de linha, é estritamente proibido usar `delay()` ou `delayMicroseconds()`. Como o LDR é sensível, o ciclo de leitura precisa de rodar livremente. O controlo de tempo do bit deve ser feito monitorizando continuamente a função `micros()`. Exemplo de espera correta: 
    `unsigned long inicio = micros();`
    `while(micros() - inicio < tempo_bit) { /* Lê o sensor e processa */ }`
