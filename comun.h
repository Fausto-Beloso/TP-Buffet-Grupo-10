#ifndef COMUN_H
#define COMUN_H
#include <cstring>
#include <iostream>
using namespace std;

//----- Structs -----

struct Producto {
    int   codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Comanda {
    char fecha[11];
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

const int K = 7; // constante k para encriptar clave

//----- Funciones compartidas -----

int buscarMozo(Mozo mozos[], int len, char nombreBuscado[]){
    int i=0;
    while (i<len && strcmp(mozos[i].nombre, nombreBuscado) != 0){ //strcmp (string compare) compara nombres para usarlos como clave, si devuelve 0 es porque son iguales
        i++;
    }
    if (i==len){
        return -1; //no encontrado
    } else {
        return i; //encontrado en posicion i
    }
}

void cargarInventario(Producto inv[], int& lenInv){
    FILE* inventario = fopen("inventario.dat", "rb");
    if (inventario == NULL) {
         return;
    }
    while (fread(&inv[lenInv], sizeof(Producto), 1, inventario) == 1)
    {
        lenInv++;
    }
    fclose(inventario);
}

void encriptarClave(char clave[], int k){
    int i=0;
    while (clave[i] != '\0') { // mientras no llegue al final del array. mientras la clave todavia exista
        clave[i] = clave[i] + k;
        i++;
    }
}

int buscarProducto(Producto inv[], int len, int codigoBuscado){
    int izquierda = 0;
    int derecha = len - 1;
     while (izquierda <= derecha) {
        int medio = (izquierda + derecha) / 2;
        if (inv[medio].codigo == codigoBuscado) {
             return medio; // encontrado
         } else if (inv[medio].codigo < codigoBuscado) {
             izquierda = medio + 1; // buscar en la mitad derecha
         } else {
             derecha = medio - 1; // buscar en la mitad izquierda
         }
     }
 return -1;
}

// Reordena "DD-MM-AAAA" a "AAAAMMDD" en una variable aparte (resultado),
// sin modificar la fecha original. En ese orden, comparar con strcmp
// sí da el orden cronológico correcto, sin depender de que las fechas
// compartan mes o año.
void fechaAOrden(char fecha[], char resultado[]){
    resultado[0] = fecha[6]; // A
    resultado[1] = fecha[7]; // A
    resultado[2] = fecha[8]; // A
    resultado[3] = fecha[9]; // A
    resultado[4] = fecha[3]; // M
    resultado[5] = fecha[4]; // M
    resultado[6] = fecha[0]; // D
    resultado[7] = fecha[1]; // D
    resultado[8] = '\0';
}

void ordenarPlanilla(Comanda ventas[], int lenVentas) {
    for (int i = 0; i < lenVentas - 1; i++) {
        int posMenor = i;
        for (int j = i + 1; j < lenVentas; j++) {
            if (ventas[j].idMozo < ventas[posMenor].idMozo) {
                posMenor = j;
            }
        }
        if (posMenor != i) {
            Comanda aux = ventas[i];
            ventas[i] = ventas[posMenor];
            ventas[posMenor] = aux;
        }
    }
}
int buscarMozoPorId(Mozo mozos[], int lenMozos, int idBuscado) {
    if (idBuscado < 1 || idBuscado > lenMozos) {
        return -1; // fuera de rango, no existe ese mozo
    }
    return idBuscado - 1; // acceso directo, sin recorrer nada // decision tomada en normalizacion al asignar el id al mozo = lenMozos + 1 (para que quede primer id 1 y no 0
}

void cargarMozos(Mozo mozos[], int &lenMozos) {
    FILE* f = fopen("mozos.dat", "rb");
    if (f == NULL) {
        cout << "No se pudo abrir mozos.dat" << endl;
        return;
    }
    while (fread(&mozos[lenMozos], sizeof(Mozo), 1, f) == 1) {
        lenMozos++;
    }
    fclose(f);
}
#endif