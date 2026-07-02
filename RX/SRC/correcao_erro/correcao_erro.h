// Contrato para o módulo correcao_erro
// Insira aqui as assinaturas das funções conforme Contratos.md

#ifndef CORRECAO_ERRO_H
#define CORRECAO_ERRO_H

#include <stdint.h>

void codificarCorrecao(const uint8_t* payload_in, uint8_t tam_in, uint8_t* buffer_out, uint8_t* tam_out);
int decodificarCorrecao(uint8_t* buffer_in_out, uint8_t tam_in, uint8_t* tam_out);

#endif
