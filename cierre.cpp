#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h" // reusa Comanda; NO hace falta struct nueva
 
using namespace std;
 
//----- Funciones propias de cierre declaradas -----
 
// a. Intenta abrir comandas_<fecha>.dat. Si existe, vuelca sus registros al array
//    acumulador (append, sin pisar lo que ya se cargó de otros días).
//    Devuelve true si encontró el archivo, false si no (no es error: puede ser
//    un typo del usuario o un día que no tuvo ventas, se avisa y se sigue).
bool cargarPlanillaDia(Comanda ventas[], int &lenVentas, char fecha[]);

// b. A partir de la primera fecha con datos ("DD-MM-AAAA"), calcula el numero de
//    semana dentro del mes y arma el string del mes (mm), para el nombre del archivo.
void obtenerSemanaYMes(char primeraFecha[], int &numSemana, char mes[3]);

// c. Arma el nombre comandas_semana_sX-mm.dat y graba el array ordenado en modo "wb".
void guardarComandasSemana(Comanda ventas[], int lenVentas, int numSemana, char mes[3]);
 
//----- Funciones propias de cierre desarrolladas -----
 
bool cargarPlanillaDia(Comanda ventas[], int &lenVentas, char fecha[]) {
    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_%s.dat", fecha);
 
    FILE* f = fopen(nombreArchivo, "rb");
    if (f == NULL) {
        cout << "No existe planilla para " << fecha << " (" << nombreArchivo << ")" << endl;
        return false; // no corta el programa, solo avisa
    }
 
    while (fread(&ventas[lenVentas], sizeof(Comanda), 1, f) == 1) {
        lenVentas++;
    }
    fclose(f);
    return true;
}
 
void obtenerSemanaYMes(char primeraFecha[], int &numSemana, char mes[3]) {
    // primeraFecha tiene formato "DD-MM-AAAA"
    int dia = (primeraFecha[0] - '0') * 10 + (primeraFecha[1] - '0');

    mes[0] = primeraFecha[3];
    mes[1] = primeraFecha[4];
    mes[2] = '\0';

    numSemana = ((dia - 1) / 7) + 1; // semana dentro del mes: dias 1-7 -> 1, 8-14 -> 2, etc.
}

void guardarComandasSemana(Comanda ventas[], int lenVentas, int numSemana, char mes[3]) {
    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_semana_s%d-%s.dat", numSemana, mes);

    FILE* f = fopen(nombreArchivo, "wb");
    if (f == NULL) {
        cout << "No se pudo crear " << nombreArchivo << endl;
        return;
    }
    fwrite(ventas, sizeof(Comanda), lenVentas, f);
    fclose(f);

    cout << "Planilla semanal guardada en " << nombreArchivo << endl;
}

int main() {
    const int MAX_VENTAS_SEMANA = 1500; // 7 dias * MAX_VENTAS_DIA de ventas.cpp (200), con margen
 
    Comanda ventas[MAX_VENTAS_SEMANA];
    int lenVentas = 0;
 
    char primeraFecha[11] = ""; // se completa con la primera fecha que SI tenga planilla
    bool huboPrimeraFecha = false;
 
    char seguir = 's';
    while (seguir == 's' || seguir == 'S') {
        char fecha[11];
        cout << "Fecha a juntar (DD-MM-AAAA): ";
        cin >> fecha;
 
        bool encontrada = cargarPlanillaDia(ventas, lenVentas, fecha);
 
        if (encontrada && !huboPrimeraFecha) {
            strcpy(primeraFecha, fecha);
            huboPrimeraFecha = true;
        }
 
        cout << "Cargar otro dia? (s/n): ";
        cin >> seguir;
    }
 
    if (!huboPrimeraFecha) {
        cout << "No se cargo ninguna planilla valida. No se genera archivo semanal." << endl;
        return 1;
    }
 
    ordenarPlanilla(ventas, lenVentas); // definida en comun.h, ordena por idMozo

    int numSemana;
    char mes[3];
    obtenerSemanaYMes(primeraFecha, numSemana, mes);

    guardarComandasSemana(ventas, lenVentas, numSemana, mes);

    cout << "Cierre completado. Ventas totales de la semana: " << lenVentas << endl;

    return 0;
}