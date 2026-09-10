#include <iostream>
#include <cstdio>
#include "comun.h"

using namespace std;

int main() {
    // 1) Cargar mozos.dat (para poder mostrar nombres, no solo numeros)
    const int MAX_MOZOS = 100;
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;
    cargarMozos(mozos, lenMozos); // si esta declarada en comun.h, la reusas de ahi

    // 2) Pedir y abrir la planilla semanal
    int numSemana, mes;
    cout << "Numero de semana: ";
    cin >> numSemana;
    cout << "Mes (numero): ";
    cin >> mes;

    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_semana_s%d-%02d.dat", numSemana, mes);

    const int MAX_SEMANA = 1000;
    Comanda semana[MAX_SEMANA];
    int lenSemana = 0;

    FILE* f = fopen(nombreArchivo, "rb");
    if (f == NULL) {
        cout << "No se encontro " << nombreArchivo << ". Corre cierre.cpp primero." << endl;
        return 1;
    }
    while (fread(&semana[lenSemana], sizeof(Comanda), 1, f) == 1) {
        lenSemana++;
    }
    fclose(f);

    if (lenSemana == 0) {
        cout << "No hay ventas registradas esta semana." << endl;
        return 0;
    }

    // 3) Corte de control: recorremos una sola vez, ya viene ordenado por idMozo
    cout << "===== Resumen de la semana " << numSemana << " =====" << endl;
    int totalBuffet = 0;
    int i = 0;
    while (i < lenSemana) {
        int mozoActual = semana[i].idMozo;
        int cantidadMozo = 0;
        float comisionMozo = 0;

        while (i < lenSemana && semana[i].idMozo == mozoActual) {
            cantidadMozo += semana[i].cantidad;
            comisionMozo += semana[i].comision;
            i++;
        }

        int posMozo = buscarMozoPorId(mozos, lenMozos, mozoActual);
        cout << "Mozo " << mozoActual << " (" << mozos[posMozo].nombre << ")"
             << " | Productos vendidos: " << cantidadMozo
             << " | Comision a pagar: $" << comisionMozo << endl;

        totalBuffet += cantidadMozo;
    }

    cout << "------------------------------------" << endl;
    cout << "Total de productos vendidos por el buffet: " << totalBuffet << endl;

    return 0;
}