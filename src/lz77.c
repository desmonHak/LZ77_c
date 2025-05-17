/* PROG1.C                                                    */
/* Simple Hashing LZ77 Sliding Dictionary Compression Program */
/* By Rich Geldreich, Jr. October, 1993                       */
/* https://gist.github.com/fogus/5401265.                     */
/* Readaptado por DesmonHak(https://github.com/desmonHak)     */

#ifndef LZ77_C
#define LZ77_C
#include "lz77.h"


DerivedParams calculate_derived_params(const LZ77Params *params) {
    DerivedParams derived;
    derived.max_coincidencia = (1 << params->bits_coincidencia) + params->umbral - 1;
    derived.tam_diccionario = (1 << params->bits_diccionario);
    derived.tam_hash = (1 << params->bits_hash);
    derived.bits_desplazamiento = (params->bits_hash + params->umbral) / (params->umbral + 1);
    derived.tam_sector = (1 << params->bit_sector);
    derived.mascara_sector = ((0xFFFF << params->bit_sector) & 0xFFFF);
    return derived;
}

/* Estructuras globales */
unsigned char *diccionario;
unsigned int *hash, *siguiente_enlace;
unsigned int longitud_coincidencia, posicion_coincidencia, buffer_bits, bits_en,
        mascaras[17] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535};

/* Variables para buffers en memoria */
const unsigned char *in_ptr;
unsigned int in_size, in_pos;
unsigned char *out_ptr;
unsigned int out_capacity, out_pos;

/* Escribe múltiples bits al buffer de salida */
void EnviarBits(unsigned int bits, unsigned int num_bits) {
    buffer_bits |= (bits << bits_en);
    bits_en += num_bits;

    while (bits_en >= 8) {
        if (out_pos >= out_capacity) {
            fprintf(stderr, "\nBuffer de salida lleno (compresión)");
            exit(EXIT_FAILURE);
        }
        out_ptr[out_pos++] = buffer_bits & 0xFF;
        buffer_bits >>= 8;
        bits_en -= 8;
    }
}

/* Lee múltiples bits del buffer de entrada */
unsigned int LeerBits(unsigned int num_bits) {
    register unsigned int i = 0, shift = 0;

    while (bits_en < num_bits) {
        if (in_pos >= in_size) {
            fprintf(stderr, "\nBuffer de entrada insuficiente (descompresión)");
            exit(EXIT_FAILURE);
        }
        buffer_bits |= (in_ptr[in_pos++] << bits_en);
        bits_en += 8;
    }

    i = buffer_bits & mascaras[num_bits];
    buffer_bits >>= num_bits;
    bits_en -= num_bits;
    return i;
}

/* EnviarCoincidencia y EnviarCaracter */
void EnviarCoincidencia(const LZ77Params *params, const DerivedParams *derived, unsigned int longitud, unsigned int distancia) {
    EnviarBits(1, 1);
    EnviarBits(longitud - (params->umbral + 1), params->bits_coincidencia);
    EnviarBits(distancia, params->bits_diccionario);
}
void EnviarCaracter(const LZ77Params *params, unsigned int caracter) {
    EnviarBits(0, 1);
    EnviarBits(caracter, params->bits_caracter);
}

/* Inicialización */
void InicializarCodificacion(const DerivedParams *derived) {
    register unsigned int i;
    buffer_bits = 0;
    bits_en = 0;

    for (i = 0; i < derived->tam_hash; i++)
        hash[i] = 0xFFFF; //NULO;
    for (i = 0; i < derived->tam_diccionario; i++)
        siguiente_enlace[i] = 0xFFFF; //NULO;
}

/* Cargar datos desde el buffer de entrada */
unsigned int CargarDiccionario(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion) {
    unsigned int bytes = (in_pos + derived->tam_sector <= in_size) ? derived->tam_sector : (in_size - in_pos);
    if (bytes == 0)
        return 0;
    memcpy(&diccionario[posicion], in_ptr + in_pos, bytes);
    in_pos += bytes;
    if (posicion == 0)
        memcpy(diccionario + derived->tam_diccionario, diccionario, derived->max_coincidencia);
    return bytes;
}

/* EliminarDatos, HashearDatos, EncontrarCoincidencia */
void EliminarDatos(const DerivedParams *derived, unsigned int posicion) {
    register unsigned int i;
    for (i = 0; i < derived->tam_diccionario; i++)
        if ((siguiente_enlace[i] & derived->mascara_sector) == posicion)
            siguiente_enlace[i] = 0xFFFF; //NULO;
    for (i = 0; i < derived->tam_hash; i++)
        if ((hash[i] & derived->mascara_sector) == posicion)
            hash[i] = 0xFFFF; //NULO;
}
void HashearDatos(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int bytes_a_hashear) {
    register unsigned int i, j, k;
    if (bytes_a_hashear <= params->umbral) {
        for (i = 0; i < bytes_a_hashear; i++)
            siguiente_enlace[posicion + i] = 0xFFFF; //NULO;
    } else {
        for (i = bytes_a_hashear - params->umbral; i < bytes_a_hashear; i++)
            siguiente_enlace[posicion + i] = 0xFFFF; //NULO;
        j = (((unsigned int)diccionario[posicion]) << derived->bits_desplazamiento) ^ diccionario[posicion + 1];
        k = posicion + bytes_a_hashear - params->umbral;
        for (i = posicion; i < k; i++) {
            siguiente_enlace[i] = hash[j = (((j << derived->bits_desplazamiento) & (derived->tam_hash - 1)) ^ diccionario[i + params->umbral])];
            hash[j] = i;
        }
    }
}
void EncontrarCoincidencia(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int longitud_inicial) {
    register unsigned int i, j, k;
    unsigned char l;
    i = posicion;
    longitud_coincidencia = longitud_inicial;
    k = params->max_comparaciones;
    l = diccionario[posicion + longitud_coincidencia];
    do {
        if ((i = siguiente_enlace[i]) == 0xFFFF) //NULO)
            return;
        if (diccionario[i + longitud_coincidencia] == l) {
            for (j = 0; j < derived->max_coincidencia; j++)
                if (diccionario[posicion + j] != diccionario[i + j])
                    break;
            if (j > longitud_coincidencia) {
                longitud_coincidencia = j;
                posicion_coincidencia = i;
                if (longitud_coincidencia == derived->max_coincidencia)
                    return;
                l = diccionario[posicion + longitud_coincidencia];
            }
        }
    } while (--k);
}

/* BuscarEnDiccionario */
void BuscarEnDiccionario(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int bytes_a_comprimir) {
    register unsigned int i, j;
    
    if (params->codicia == 0){
        unsigned int longitud1, posicion1;
        i = posicion;
        j = bytes_a_comprimir;
        while (j) {
            EncontrarCoincidencia(params, derived, i, params->umbral);
            if (longitud_coincidencia > params->umbral) {
                longitud1 = longitud_coincidencia;
                posicion1 = posicion_coincidencia;
                for (;;) {
                    EncontrarCoincidencia(params, derived, i + 1, longitud1);
                    if (longitud_coincidencia > longitud1) {
                        longitud1 = longitud_coincidencia;
                        posicion1 = posicion_coincidencia;
                        EnviarCaracter(params, diccionario[i++]);
                        j--;
                    } else {
                        if (longitud1 > j) {
                            longitud1 = j;
                            if (longitud1 <= params->umbral) {
                                EnviarCaracter(params, diccionario[i++]);
                                j--;
                                break;
                            }
                        }
                        EnviarCoincidencia(params, derived, longitud1, (i - posicion1) & (derived->tam_diccionario - 1));
                        i += longitud1;
                        j -= longitud1;
                        break;
                    }
                }
            } else {
                EnviarCaracter(params, diccionario[i++]);
                j--;
            }
        }
    }else {
        i = posicion;
        j = bytes_a_comprimir;
        while (j) {
            EncontrarCoincidencia(params, derived, i, params->umbral);
            if (longitud_coincidencia > j)
                longitud_coincidencia = j;
            if (longitud_coincidencia > params->umbral) {
                EnviarCoincidencia(params, derived, longitud_coincidencia, (i - posicion_coincidencia) & (derived->tam_diccionario - 1));
                i += longitud_coincidencia;
                j -= longitud_coincidencia;
            } else {
                EnviarCaracter(params, diccionario[i++]);
                j--;
            }
        }
    }
}

/* Compresión en memoria */
int CodificarBuffer(const LZ77Params *params, const DerivedParams *derived, const unsigned char *input, unsigned int input_size, unsigned char *output, unsigned int output_capacity) {
    in_ptr = input;
    in_size = input_size;
    in_pos = 0;
    out_ptr = output;
    out_capacity = output_capacity;
    out_pos = 0;

    unsigned int posicion_diccionario = 0, marcar_para_eliminar = 0, longitud_sector;
    InicializarCodificacion(derived);

    while (1) {
        if (marcar_para_eliminar)
            EliminarDatos(derived, posicion_diccionario);
        if ((longitud_sector = CargarDiccionario(params, derived, posicion_diccionario)) == 0)
            break;
        HashearDatos(params, derived, posicion_diccionario, longitud_sector);
        BuscarEnDiccionario(params, derived, posicion_diccionario, longitud_sector);
        posicion_diccionario += derived->tam_sector;
        if (posicion_diccionario == derived->tam_diccionario) {
            posicion_diccionario = 0;
            marcar_para_eliminar = 1;
        }
    }
    EnviarCoincidencia(params, derived, derived->max_coincidencia + 1, 0);
    if (bits_en)
        EnviarBits(0, 8 - bits_en);
    return out_pos; // Tamaño de los datos comprimidos
}

/* Descompresión en memoria */
int DecodificarBuffer(const LZ77Params *params, const DerivedParams *derived, const unsigned char *input, unsigned int input_size, unsigned char *output, unsigned int output_capacity) {
    in_ptr = input;
    in_size = input_size;
    in_pos = 0;
    out_ptr = output;
    out_capacity = output_capacity;
    out_pos = 0;
    bits_en = 0;
    buffer_bits = 0;
    unsigned int i = 0;
    for (;;) {
        if (LeerBits(1) == 0) {
            if (i >= derived->tam_diccionario + derived->max_coincidencia) {
                fprintf(stderr, "Desbordamiento de diccionario\n");
                return -1;
            }
            diccionario[i++] = LeerBits(params->bits_caracter);
            if (i == derived->tam_diccionario) {
                if (out_pos + derived->tam_diccionario > out_capacity) {
                    fprintf(stderr, "Buffer de salida lleno (descompresión)\n");
                    return -1;
                }
                memcpy(out_ptr + out_pos, diccionario, derived->tam_diccionario);
                out_pos += derived->tam_diccionario;
                i = 0;
            }
        } else {
            unsigned int k = (params->umbral + 1) + LeerBits(params->bits_coincidencia);
            if (k == (derived->max_coincidencia + 1)) {
                if (out_pos + i > out_capacity) {
                    fprintf(stderr, "Buffer de salida lleno (descompresión)\n");
                    return -1;
                }
                memcpy(out_ptr + out_pos, diccionario, i);
                out_pos += i;
                return out_pos;
            }
            unsigned int j = ((i - LeerBits(params->bits_diccionario)) & (derived->tam_diccionario - 1));
            if ((i + k) >= derived->tam_diccionario) {
                do {
                    diccionario[i++] = diccionario[j++];
                    j &= (derived->tam_diccionario - 1);
                    if (i == derived->tam_diccionario) {
                        if (out_pos + derived->tam_diccionario > out_capacity) {
                            fprintf(stderr, "Buffer de salida lleno (descompresión)\n");
                            return -1;
                        }
                        memcpy(out_ptr + out_pos, diccionario, derived->tam_diccionario);
                        out_pos += derived->tam_diccionario;
                        i = 0;
                    }
                } while (--k);
            } else {
                if ((j + k) >= derived->tam_diccionario) {
                    do {
                        diccionario[i++] = diccionario[j++];
                        j &= (derived->tam_diccionario - 1);
                    } while (--k);
                } else {
                    do {
                        diccionario[i++] = diccionario[j++];
                    } while (--k);
                }
            }
        }
    }
}



#endif