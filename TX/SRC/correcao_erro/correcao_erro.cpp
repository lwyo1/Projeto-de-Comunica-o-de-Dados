// Implementação para o módulo correcao_erro
#include "correcao_erro.h"

static uint8_t encode_nibble(uint8_t nibble) {
    uint8_t d1 = (nibble >> 3) & 0x01;
    uint8_t d2 = (nibble >> 2) & 0x01;
    uint8_t d3 = (nibble >> 1) & 0x01;
    uint8_t d4 = nibble & 0x01;

    uint8_t p1 = d1 ^ d2 ^ d4;
    uint8_t p2 = d1 ^ d3 ^ d4;
    uint8_t p4 = d2 ^ d3 ^ d4;

    uint8_t bits1a7 = (p1 << 0) | (p2 << 1) | (d1 << 2) | (p4 << 3) | (d2 << 4) | (d3 << 5) | (d4 << 6);

    uint8_t p8 = 0;
    for (int i = 0; i < 7; i++) p8 ^= (bits1a7 >> i) & 0x01;

    return bits1a7 | (p8 << 7);
}

static uint8_t decode_nibble(uint8_t byte_in, bool* erro_nao_corrigivel, bool* bit_corrigido) {
    *erro_nao_corrigivel = false;
    *bit_corrigido = false;

    uint8_t bits1a7 = byte_in & 0x7F;
    uint8_t p8_recebido = (byte_in >> 7) & 0x01;

    uint8_t p1 = (bits1a7 >> 0) & 0x01;
    uint8_t p2 = (bits1a7 >> 1) & 0x01;
    uint8_t d1 = (bits1a7 >> 2) & 0x01;
    uint8_t p4 = (bits1a7 >> 3) & 0x01;
    uint8_t d2 = (bits1a7 >> 4) & 0x01;
    uint8_t d3 = (bits1a7 >> 5) & 0x01;
    uint8_t d4 = (bits1a7 >> 6) & 0x01;

    uint8_t c1 = p1 ^ d1 ^ d2 ^ d4;
    uint8_t c2 = p2 ^ d1 ^ d3 ^ d4;
    uint8_t c4 = p4 ^ d2 ^ d3 ^ d4;
    uint8_t sindrome = c1 | (c2 << 1) | (c4 << 2);

    uint8_t paridade_calc = 0;
    for (int i = 0; i < 7; i++) paridade_calc ^= (bits1a7 >> i) & 0x01;
    uint8_t p_check = paridade_calc ^ p8_recebido;

    if (sindrome == 0 && p_check == 0) {
        // sem erro
    } else if (sindrome == 0 && p_check == 1) {
        *bit_corrigido = true;
    } else if (sindrome != 0 && p_check == 1) {
        bits1a7 ^= (1 << (sindrome - 1));
        d1 = (bits1a7 >> 2) & 0x01;
        d2 = (bits1a7 >> 4) & 0x01;
        d3 = (bits1a7 >> 5) & 0x01;
        d4 = (bits1a7 >> 6) & 0x01;
        *bit_corrigido = true;
    } else {
        *erro_nao_corrigivel = true;
    }

    return (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
}

void codificarCorrecao(const uint8_t* payload_in, uint8_t tam_in, uint8_t* buffer_out, uint8_t* tam_out) {
    uint8_t tam = 0;
    for (uint8_t i = 0; i < tam_in; i++) {
        if (tam + 2 > 256) break;

        uint8_t nibble_alto = (payload_in[i] >> 4) & 0x0F;
        uint8_t nibble_baixo = payload_in[i] & 0x0F;

        buffer_out[tam++] = encode_nibble(nibble_alto);
        buffer_out[tam++] = encode_nibble(nibble_baixo);
    }
    *tam_out = tam;
}

int decodificarCorrecao(uint8_t* buffer_in_out, uint8_t tam_in, uint8_t* tam_out) {
    if (tam_in % 2 != 0) {
        *tam_out = 0;
        return -1;
    }

    uint8_t tam_payload = tam_in / 2;
    int total_corrigidos = 0;

    for (uint8_t i = 0; i < tam_payload; i++) {
        bool erro_fatal_a, corrigido_a;
        bool erro_fatal_b, corrigido_b;

        uint8_t nibble_alto  = decode_nibble(buffer_in_out[i * 2],     &erro_fatal_a, &corrigido_a);
        uint8_t nibble_baixo = decode_nibble(buffer_in_out[i * 2 + 1], &erro_fatal_b, &corrigido_b);

        if (erro_fatal_a || erro_fatal_b) {
            *tam_out = 0;
            return -1;
        }

        if (corrigido_a) total_corrigidos++;
        if (corrigido_b) total_corrigidos++;

        buffer_in_out[i] = (nibble_alto << 4) | nibble_baixo;
    }

    if (total_corrigidos > 3) {
        *tam_out = 0;
        return -1;
    }

    *tam_out = tam_payload;
    return total_corrigidos;
}

// Desenvolva sua lógica aqui

