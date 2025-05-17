// lz77.h
#ifndef LZ77_H
#define LZ77_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Estructura que define los parámetros de configuración para el algoritmo de compresión LZ77.
 */
typedef struct LZ77Params {
    /**
     * @brief Define si el algoritmo debe ser "codicioso" o no.
     *
     * Un codificador "ávido" (codicia = 1) es más rápido pero generalmente logra menor compresión.
     * Un codificador no ávido (codicia = 0) intenta encontrar la mejor coincidencia, lo que lleva a una mejor compresión a costa de mayor tiempo de procesamiento.
     *
     * @range 0 o 1
     */
    int codicia;

    /**
     * @brief Número máximo de comparaciones que se realizan al buscar una coincidencia en el diccionario.
     *
     * Un valor más alto puede mejorar la compresión al encontrar coincidencias más largas, pero aumenta el tiempo de compresión.
     * Un valor más bajo acelera la compresión pero puede resultar en una menor tasa de compresión.
     *
     * @range 1 a ... (Depende de la memoria disponible y el tiempo de compresión deseado)
     */
    int max_comparaciones;

    /**
     * @brief Número de bits utilizados para representar cada carácter en el texto de entrada.
     *
     * Generalmente se establece en 8 para compresión de datos de propósito general (ASCII, UTF-8).
     * Si se conoce el rango de los símbolos de entrada, se puede reducir para mejorar la compresión.
     *
     * @range 1 a 8 (o más, dependiendo del conjunto de caracteres)
     */
    int bits_caracter;

    /**
     * @brief Longitud mínima que debe tener una coincidencia para ser considerada válida.
     *
     * Coincidencias más cortas que este umbral se codifican como caracteres literales.
     * Aumentar este valor puede mejorar la compresión si hay muchas coincidencias cortas, pero también puede reducir la compresión si las coincidencias cortas son importantes.
     *
     * @range 1 a ... (Debe ser menor que el tamaño máximo de coincidencia)
     */
    int umbral;

    /**
     * @brief Número de bits utilizados para representar la longitud de una coincidencia.
     *
     * Determina la longitud máxima de una coincidencia que puede ser codificada.
     * Un valor más alto permite codificar coincidencias más largas, pero requiere más bits para representar la longitud.
     * La longitud máxima de coincidencia es (1 << bits_coincidencia) + umbral - 1.
     *
     * @range 1 a ... (Depende del tamaño del diccionario y el tiempo de compresión deseado)
     */
    int bits_coincidencia;

    /**
     * @brief Número de bits utilizados para representar el desplazamiento (distancia) de una coincidencia en el diccionario.
     *
     * Determina el tamaño del diccionario (ventana deslizante).
     * Un valor más alto permite un diccionario más grande, lo que puede mejorar la compresión si hay coincidencias lejanas, pero requiere más memoria y puede aumentar el tiempo de búsqueda.
     * El tamaño del diccionario es 2^bits_diccionario.
     *
     * @range 1 a ... (Depende de la memoria disponible y el tiempo de compresión deseado)
     */
    int bits_diccionario;

    /**
     * @brief Número de bits utilizados para la tabla hash.
     *
     * Determina el tamaño de la tabla hash utilizada para buscar coincidencias en el diccionario.
     * Un valor más alto reduce las colisiones en la tabla hash, lo que puede mejorar el tiempo de búsqueda, pero requiere más memoria.
     * El tamaño de la tabla hash es 2^bits_hash.
     *
     * @range 1 a ... (Depende de la memoria disponible y el tiempo de compresión deseado)
     */
    int bits_hash;

    /**
     * @brief Número de bits utilizados para el tamaño del sector.
     *
     * Define el tamaño de los sectores en los que se divide el diccionario.
     *
     * @range 1 a ... (Depende de la memoria disponible y el tiempo de compresión deseado)
     */
    int bit_sector;
} LZ77Params;

/* Valores por defecto para los parámetros */
static LZ77Params default_params = {
    .codicia = 0,
    .max_comparaciones = 75,
    .bits_caracter = 8,
    .umbral = 2,
    .bits_coincidencia = 4,
    .bits_diccionario = 13,
    .bits_hash = 10,
    .bit_sector = 10
};

/**
 * @brief Estructura que define los parámetros derivados calculados a partir de los parámetros de configuración de LZ77Params.
 */
typedef struct DerivedParams {
    unsigned int max_coincidencia;
    unsigned int tam_diccionario;
    unsigned int tam_hash;
    unsigned int bits_desplazamiento;
    unsigned int tam_sector;
    unsigned int mascara_sector;
} DerivedParams;

/* Valores por defecto para los parámetros */
extern LZ77Params default_params;

/* Estructuras globales */
extern unsigned char *diccionario;
extern unsigned int *hash, *siguiente_enlace;
extern unsigned int longitud_coincidencia, posicion_coincidencia, buffer_bits, bits_en;

/* Variables para buffers en memoria */
extern const unsigned char *in_ptr;
extern unsigned int in_size, in_pos;
extern unsigned char *out_ptr;
extern unsigned int out_capacity, out_pos;

/* Prototipos de funciones */
DerivedParams calculate_derived_params(const LZ77Params *params);
void EnviarBits(unsigned int bits, unsigned int num_bits);
unsigned int LeerBits(unsigned int num_bits);
void EnviarCoincidencia(const LZ77Params *params, const DerivedParams *derived, unsigned int longitud, unsigned int distancia);
void EnviarCaracter(const LZ77Params *params, unsigned int caracter);
void InicializarCodificacion(const DerivedParams *derived);
unsigned int CargarDiccionario(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion);
void EliminarDatos(const DerivedParams *derived, unsigned int posicion);
void HashearDatos(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int bytes_a_hashear);
void EncontrarCoincidencia(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int longitud_inicial);
void BuscarEnDiccionario(const LZ77Params *params, const DerivedParams *derived, unsigned int posicion, unsigned int bytes_a_comprimir);
int CodificarBuffer(const LZ77Params *params, const DerivedParams *derived, const unsigned char *input, unsigned int input_size, unsigned char *output, unsigned int output_capacity);
int DecodificarBuffer(const LZ77Params *params, const DerivedParams *derived, const unsigned char *input, unsigned int input_size, unsigned char *output, unsigned int output_capacity);

#endif
