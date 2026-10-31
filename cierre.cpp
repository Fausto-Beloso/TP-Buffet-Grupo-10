#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h" // reusa Comanda; NO hace falta struct nueva

using namespace std;

//----- Funciones propias de cierre declaradas -----

// a. Devuelve true si el archivo existe (se puede abrir en "rb").
bool existeArchivo(const char nombre[]);

// b. A partir de la primera fecha con datos ("DD-MM-AAAA"), calcula el numero de
//    semana dentro del mes y arma el string del mes (mm), para el nombre del archivo.
void obtenerSemanaYMes(char primeraFecha[], int &numSemana, char mes[3]);

// c. APAREO: une dos archivos de Comanda ya ordenados por idMozo en un tercero
//    tambien ordenado, sin cargar nada en memoria (un registro de cada uno a la vez).
//    Si archA no existe se lo trata como vacio.
void aparearArchivos(const char archA[], const char archB[], const char archSalida[]);

// d. Suma la planilla de un dia al semanal acumulado (apareo + reemplazo del acumulado).
void sumarDiaASemanal(const char archSemana[], const char archDia[]);

//----- Funciones propias de cierre desarrolladas -----

bool existeArchivo(const char nombre[]) {
    FILE* f = fopen(nombre, "rb");
    if (f == NULL) return false;
    fclose(f);
    return true;
}

void obtenerSemanaYMes(char primeraFecha[], int &numSemana, char mes[3]) {
    // primeraFecha tiene formato "DD-MM-AAAA"
    int dia = (primeraFecha[0] - '0') * 10 + (primeraFecha[1] - '0');

    mes[0] = primeraFecha[3];
    mes[1] = primeraFecha[4];
    mes[2] = '\0';

    numSemana = ((dia - 1) / 7) + 1; // dias 1-7 -> 1, 8-14 -> 2, etc.
}

void aparearArchivos(const char archA[], const char archB[], const char archSalida[]) {
    FILE* fA = fopen(archA, "rb");   // puede ser NULL: se trata como archivo vacio
    FILE* fB = fopen(archB, "rb");
    FILE* fS = fopen(archSalida, "wb");

    if (fB == NULL || fS == NULL) {
        cout << "Error abriendo archivos para el apareo" << endl;
        if (fA) fclose(fA);
        if (fB) fclose(fB);
        if (fS) fclose(fS);
        return;
    }

    Comanda a, b;
    bool hayA = (fA != NULL) && (fread(&a, sizeof(Comanda), 1, fA) == 1);
    bool hayB = (fread(&b, sizeof(Comanda), 1, fB) == 1);

    // Mientras queden registros en AMBOS: se escribe el de menor idMozo.
    // Con <= , si empatan sale primero A (el acumulado, dias anteriores).
    while (hayA && hayB) {
        if (a.idMozo <= b.idMozo) {
            fwrite(&a, sizeof(Comanda), 1, fS);
            hayA = (fread(&a, sizeof(Comanda), 1, fA) == 1);
        } else {
            fwrite(&b, sizeof(Comanda), 1, fS);
            hayB = (fread(&b, sizeof(Comanda), 1, fB) == 1);
        }
    }

    // Uno de los dos se termino: se vuelca lo que le quede al otro (ya viene ordenado).
    while (hayA) {
        fwrite(&a, sizeof(Comanda), 1, fS);
        hayA = (fread(&a, sizeof(Comanda), 1, fA) == 1);
    }
    while (hayB) {
        fwrite(&b, sizeof(Comanda), 1, fS);
        hayB = (fread(&b, sizeof(Comanda), 1, fB) == 1);
    }

    if (fA) fclose(fA);
    fclose(fB);
    fclose(fS);
}

void sumarDiaASemanal(const char archSemana[], const char archDia[]) {
    const char archTmp[] = "cierre_tmp.dat";

    aparearArchivos(archSemana, archDia, archTmp);

    remove(archSemana);            // se borra el acumulado viejo
    rename(archTmp, archSemana);   // el resultado del apareo pasa a ser el acumulado
}

int main() {
    char archSemana[30] = "";
    bool huboPrimeraFecha = false;
    int totalDias = 0;

    char seguir = 's';
    while (seguir == 's' || seguir == 'S') {
        char fecha[11];
        cout << "Fecha a juntar (DD-MM-AAAA): ";
        cin >> fecha;

        char archDia[30];
        sprintf(archDia, "comandas_%s.dat", fecha);

        if (!existeArchivo(archDia)) {
            // no es error: typo del usuario o dia sin ventas; se avisa y se sigue
            cout << "No existe planilla para " << fecha << " (" << archDia << ")" << endl;
        } else {
            if (!huboPrimeraFecha) {
                // con la primera fecha valida se define el nombre del semanal
                int numSemana;
                char mes[3];
                obtenerSemanaYMes(fecha, numSemana, mes);
                sprintf(archSemana, "comandas_semana_s%d-%s.dat", numSemana, mes);

                // se crea/vacia el semanal para no mezclar con una corrida anterior
                FILE* fNuevo = fopen(archSemana, "wb");
                if (fNuevo == NULL) {
                    cout << "No se pudo crear " << archSemana << endl;
                    return 1;
                }
                fclose(fNuevo);
                huboPrimeraFecha = true;
            }

            sumarDiaASemanal(archSemana, archDia);
            totalDias++;
            cout << "Dia " << fecha << " agregado." << endl;
        }

        cout << "Cargar otro dia? (s/n): ";
        cin >> seguir;
    }

    if (!huboPrimeraFecha) {
        cout << "No se cargo ninguna planilla valida. No se genera archivo semanal." << endl;
        return 1;
    }

    cout << "Cierre completado. Planilla semanal: " << archSemana
         << " (" << totalDias << " dias juntados)" << endl;

    return 0;
}