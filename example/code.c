#include <stdio.h>
#include <stdlib.h>
#include "lz77.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <archivo_entrada>\n", argv[0]);
        return 1;
    }

    FILE *archivo_entrada = fopen(argv[1], "rb");
    if (!archivo_entrada) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    // Obtener el tamaño del archivo
    fseek(archivo_entrada, 0, SEEK_END);
    unsigned int tam_datos = ftell(archivo_entrada);
    fseek(archivo_entrada, 0, SEEK_SET);

    // Leer el archivo en un buffer
    unsigned char *datos = (unsigned char *)malloc(tam_datos);
    if (!datos) {
        fprintf(stderr, "Error al asignar memoria para el archivo de entrada\n");
        fclose(archivo_entrada);
        return 1;
    }
    if (fread(datos, 1, tam_datos, archivo_entrada) != tam_datos) {
        fprintf(stderr, "Error al leer el archivo de entrada\n");
        fclose(archivo_entrada);
        free(datos);
        return 1;
    }
    fclose(archivo_entrada);

    // Preparar buffers para comprimir y descomprimir
    unsigned char *comprimido = (unsigned char *)malloc(tam_datos * 2); // Asignar más espacio para el buffer comprimido
    unsigned char *descomprimido = (unsigned char *)malloc(tam_datos);

    if (!comprimido || !descomprimido) {
        fprintf(stderr, "Error al asignar memoria para los buffers de compresión/descompresión\n");
        free(datos);
        if (comprimido) free(comprimido);
        if (descomprimido) free(descomprimido);
        return 1;
    }

    /* Inicializar la estructura de parámetros */
    LZ77Params params = default_params;
    /* Modificar los parámetros si es necesario */
    /* params.bits_diccionario = 12; */

    /* Calcular los parámetros derivados */
    DerivedParams derived = calculate_derived_params(&params);

    /* Asignar memoria para las tablas globales */
    diccionario = (unsigned char *)malloc(derived.tam_diccionario + derived.max_coincidencia);
    hash = (unsigned int *)malloc(derived.tam_hash * sizeof(unsigned int));
    siguiente_enlace = (unsigned int *)malloc(derived.tam_diccionario * sizeof(unsigned int));

    if (!diccionario || !hash || !siguiente_enlace) {
        fprintf(stderr, "Error al asignar memoria para las tablas de compresión\n");
        free(datos);
        free(comprimido);
        free(descomprimido);
        if (diccionario) free(diccionario);
        if (hash) free(hash);
        if (siguiente_enlace) free(siguiente_enlace);
        return 1;
    }

    // Comprimir los datos
    int tam_comprimido = CodificarBuffer(&params, &derived, datos, tam_datos, comprimido, tam_datos * 2);
    if (tam_comprimido < 0) {
        printf("Error al comprimir\n");
    } else {
        printf("Size original: %u, Size comprimido: %d\n", tam_datos, tam_comprimido);
        // Descomprimir los datos
        int tam_descomprimido = DecodificarBuffer(&params, &derived, comprimido, tam_comprimido, descomprimido, tam_datos);
        if (tam_descomprimido < 0) {
            printf("Error al descomprimir\n");
        } else {
            printf("Size descomprimido: %d\n", tam_descomprimido);
            // Verificar si la descompresión fue exitosa (opcional)
            if (tam_descomprimido == tam_datos && memcmp(datos, descomprimido, tam_datos) == 0) {
                printf("Compresion y descompresion exitosas\n");
            } else {
                printf("Error: Los datos descomprimidos no coinciden con los originales\n");
            }
        }
    }

    // Liberar la memoria
    free(datos);
    free(comprimido);
    free(descomprimido);
    free(diccionario);
    free(hash);
    free(siguiente_enlace);

    return 0;
}