#include <iostream>
#include <cstring>
#include <cstdio>
#include "comun.h"

using namespace std;
int main() {
    const int MAX_SEMANA = 1000; // suficiente para toda la semana junta
    Comanda semana[MAX_SEMANA];
    int lenSemana = 0;

    // Array con los nombres de los archivos de cada día de la semana
    const char* diasSemana[] = {
        "comandas_02-06-2025.dat",
        "comandas_03-06-2025.dat",
        "comandas_04-06-2025.dat",
        "comandas_05-06-2025.dat",
        "comandas_06-06-2025.dat",
        "comandas_07-06-2025.dat"
    };
    int cantDias = 6;

    // 1) Leer cada archivo del día y volcar sus registros al array semanal
    for (int i = 0; i < cantDias; i++) {
        FILE* f = fopen(diasSemana[i], "rb");
        if (f == NULL) {
            cout << "No se encontro " << diasSemana[i] << ", se omite." << endl;
            continue; // caso raro: un dia sin ventas, no es un error
        }
        while (fread(&semana[lenSemana], sizeof(Comanda), 1, f) == 1) {
            lenSemana++;
        }
        fclose(f);
    }

    // 2) Ordenar por mozo (reusando la logica que ya tienen)
    ordenarPlanilla(semana, lenSemana);

    // 3) Guardar la planilla semanal
    FILE* fSemana = fopen("comandas_semana_s1-06.dat", "wb");
    if (fSemana == NULL) {
        cout << "No se pudo crear la planilla semanal." << endl;
        return 1;
    }
    fwrite(semana, sizeof(Comanda), lenSemana, fSemana);
    fclose(fSemana);

    cout << "Cierre completado. Ventas totales de la semana: " << lenSemana << endl;
    return 0;
}