/*
  Teste isolado do módulo correcao_erro (Pessoa 3)

  Este sketch NÃO usa nenhum pino (LED/LDR), pois o módulo de correção
  de erros trabalha só em cima de arrays de bytes em memória — não
  depende de sincronismo, CRC ou codificação de linha de nenhum colega.
*/

#include "correcao_erro.h"

int falhas = 0;

void checa(bool cond, const char* msg) {
  Serial.print(cond ? "[OK] " : "[FALHOU] ");
  Serial.println(msg);
  if (!cond) falhas++;
}

bool bytesIguais(const uint8_t* a, const uint8_t* b, uint8_t tam) {
  for (uint8_t i = 0; i < tam; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* aguarda porta serial (necessário em algumas placas) */ }

  Serial.println("=== Teste do modulo correcao_erro ===");

  // Teste 1: sem erro nenhum
  {
    uint8_t payload[] = {0x41, 0x42, 0x43};
    uint8_t cod[256];
    uint8_t tam_cod;

    codificarCorrecao(payload, 3, cod, &tam_cod);
    checa(tam_cod == 6, "Teste1: tamanho codificado = 2x entrada");

    uint8_t tam_final;
    int erros = decodificarCorrecao(cod, tam_cod, &tam_final);
    checa(erros == 0, "Teste1: 0 erros corrigidos");
    checa(tam_final == 3 && bytesIguais(cod, payload, 3), "Teste1: payload recuperado igual ao original");
  }

  // Teste 2: 1 bit invertido em 1 bloco -> deve corrigir
  {
    uint8_t payload[] = {0x41, 0x42, 0x43};
    uint8_t cod[256];
    uint8_t tam_cod;

    codificarCorrecao(payload, 3, cod, &tam_cod);
    cod[0] ^= 0x04; // simula ruído: inverte 1 bit

    uint8_t tam_final;
    int erros = decodificarCorrecao(cod, tam_cod, &tam_final);
    checa(erros == 1, "Teste2: 1 erro corrigido");
    checa(bytesIguais(cod, payload, 3), "Teste2: payload recuperado igual ao original apos 1 bit invertido");
  }

  // Teste 3: 3 bits invertidos em 3 blocos DIFERENTES -> deve corrigir todos
  {
    uint8_t payload[] = {0x41, 0x42, 0x43};
    uint8_t cod[256];
    uint8_t tam_cod;

    codificarCorrecao(payload, 3, cod, &tam_cod);
    cod[0] ^= 0x02; // bloco 0
    cod[2] ^= 0x08; // bloco 2
    cod[4] ^= 0x40; // bloco 4

    uint8_t tam_final;
    int erros = decodificarCorrecao(cod, tam_cod, &tam_final);
    checa(erros == 3, "Teste3: 3 erros corrigidos (blocos diferentes)");
    checa(bytesIguais(cod, payload, 3), "Teste3: payload recuperado igual ao original apos 3 bits invertidos");
  }

  // Teste 4: 2 bits invertidos no MESMO bloco -> nao corrigivel, retorna -1
  {
    uint8_t payload[] = {0x41, 0x42, 0x43};
    uint8_t cod[256];
    uint8_t tam_cod;

    codificarCorrecao(payload, 3, cod, &tam_cod);
    cod[0] ^= 0x03; // 2 bits no mesmo byte codificado

    uint8_t tam_final;
    int erros = decodificarCorrecao(cod, tam_cod, &tam_final);
    checa(erros == -1, "Teste4: 2 erros no mesmo bloco -> retorna -1");
  }

  // Teste 5: payload maximo (66 bytes = 64 de msg + 2 do CRC, conforme fluxo do .ino oficial)
  {
    uint8_t payload[66];
    for (int i = 0; i < 66; i++) payload[i] = (uint8_t)(i * 7 + 3);
    uint8_t cod[256];
    uint8_t tam_cod;

    codificarCorrecao(payload, 66, cod, &tam_cod);
    checa(tam_cod == 132, "Teste5: 66 bytes de entrada -> 132 bytes codificados (cabe em 256)");

    uint8_t tam_final;
    int erros = decodificarCorrecao(cod, tam_cod, &tam_final);
    checa(erros == 0 && tam_final == 66 && bytesIguais(cod, payload, 66), "Teste5: payload de 66 bytes recuperado sem erro");
  }

  Serial.println();
  if (falhas == 0) {
    Serial.println("TODOS OS TESTES PASSARAM");
  } else {
    Serial.print("TEM TESTE QUEBRADO, falhas: ");
    Serial.println(falhas);
  }
}

void loop() {
  // nada aqui, os testes rodam uma vez no setup()
}
