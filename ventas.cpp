#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h"

using namespace std;


const float TASA_COMISION = 0.10f; //comision a calcular


//----- Funciones propias de ventas declaradas -----

//a. copia la clave tipeada y la compara con la original
bool validarClave (Mozo mozos[], int pos, char claveIngresada[]);

//b. busca el producto (binaria), chequea stock, calcula comision, descuenta stock, suma comision al mozo y graba la venta directo en la planilla del dia
bool registrarVenta(Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad);

//c. ordena la planilla del dia por idMozo directo sobre el archivo (seleccion con fseek), sin limite de ventas
void ordenarPlanillaDelDia (char fecha[]);

//d. actualiza inventario.dat y mozos.dat
void guardarDatosFinales (Producto inv[], int lenInv, Mozo mozos[], int lenMozos);

//----- Funciones propias de ventas desarrolladas -----


// con la posicion de mozo encontrado, compara las contrasenias
bool validarClave(Mozo mozos[], int pos, char claveIngresada[]) {
    char copia[20];
    strcpy(copia, claveIngresada); // trabajamos sobre una copia para no encriptar la clave de una, reservamos exactamente lo que tipeo el mozo
    encriptarClave(copia, K);      // ahora si encriptamos la clave tipeada por el mozo
    return strcmp(copia, mozos[pos].password) == 0;
}


// para el problema de que se quiera vender y no haya stock, decidimos rechazar la venta

bool registrarVenta(Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad){
    int posProd = buscarProducto(inv, lenInv, codigoProducto);
    if (posProd == -1) {
        cout << "No existe el producto " << codigoProducto << endl;
        return false;
    }
    if (inv[posProd].stockActual < cantidad) {
        cout << "Stock insuficiente. Disponible: " << inv[posProd].stockActual << endl;
        return false;
    }

    float comision = inv[posProd].precio * cantidad * TASA_COMISION;

    Comanda c; // una sola comanda en memoria, no un array
    strcpy(c.fecha, fecha);
    c.idMozo = mozos[posMozo].idMozo;
    c.codigoProducto = codigoProducto;
    c.cantidad = cantidad;
    c.comision = comision;

    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_%s.dat", fecha); // comandas_dd-mm-aaaa.dat
    FILE* f = fopen(nombreArchivo, "ab"); // "ab" = agrega al final, si no existe lo crea
    if (f == NULL) {
        cout << "No se pudo abrir " << nombreArchivo << endl;
        return false;
    }
    fwrite(&c, sizeof(Comanda), 1, f);
    fclose(f);

    inv[posProd].stockActual -= cantidad; // recien ahora, con la venta ya grabada
    mozos[posMozo].totalComision += comision;
    return true;
}


// ordena la planilla del dia por idMozo directamente sobre el archivo (seleccion), sin arrays ni punteros.
// la cantidad de ventas sale del tamano del archivo (ftell / sizeof), asi que no hay limite
void ordenarPlanillaDelDia(char fecha[]) {
    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_%s.dat", fecha);

    FILE* f = fopen(nombreArchivo, "rb+"); // lectura + escritura, edita in situ
    if (f == NULL) return; // ese dia no se cargo nada, no hay planilla

    fseek(f, 0, SEEK_END);
    int n = ftell(f) / sizeof(Comanda); // cantidad de ventas en el archivo

    Comanda actual, menor, aux; // solo 3 registros en memoria, no importa cuantas ventas haya
    for (int i = 0; i < n - 1; i++) {
        fseek(f, (long)i * sizeof(Comanda), SEEK_SET);
        fread(&actual, sizeof(Comanda), 1, f); // registro de la posicion i
        menor = actual;
        int posMenor = i;

        // el puntero ya quedo en i+1, sigo leyendo en orden buscando el idMozo mas chico
        for (int j = i + 1; j < n; j++) {
            fread(&aux, sizeof(Comanda), 1, f);
            if (aux.idMozo < menor.idMozo) {
                menor = aux;
                posMenor = j;
            }
        }

        if (posMenor != i) { // intercambio: el menor va a la posicion i, el que estaba en i va a donde estaba el menor
            fseek(f, (long)i * sizeof(Comanda), SEEK_SET);
            fwrite(&menor, sizeof(Comanda), 1, f);
            fseek(f, (long)posMenor * sizeof(Comanda), SEEK_SET);
            fwrite(&actual, sizeof(Comanda), 1, f);
        }
    }
    fclose(f);
}


void guardarDatosFinales(Producto inv[], int lenInv, Mozo mozos[], int lenMozos) {
    FILE* fInv = fopen("inventario.dat", "wb");
    if (fInv == NULL) {
        cout << "No se pudo guardar inventario.dat" << endl;
        return;
    }
    fwrite(inv, sizeof(Producto), lenInv, fInv);
    fclose(fInv);

    FILE* fMoz = fopen("mozos.dat", "wb");
    if (fMoz == NULL) {
        cout << "No se pudo guardar mozos.dat" <<endl;
        return;
    }
    fwrite(mozos, sizeof(Mozo), lenMozos, fMoz);
    fclose(fMoz);
}

int main() {
	// declaraciÃ³n en RAM, cada uno con su contador
    const int MAX_PROD = 100;
    const int MAX_MOZOS = 100;

    Producto inv[MAX_PROD];
    int lenInv = 0;
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;
    int ventasHoy = 0;
    int unidadesHoy = 0; 
	
	// carga lo que ya existe antes de arrancar a vender 
    cargarInventario(inv, lenInv);
    cargarMozos(mozos, lenMozos);

	// ventas.cpp necesita leer lo que normalizacion.cpp ya dejo armado
    if (lenInv == 0 || lenMozos == 0) {
        cout << "Faltan mozos.dat / inventario.dat. Corre normalizacion.cpp primero." << endl;
        return 1;
    }
    
	// la fecha una vez fuera de cualquier loop. cumple con la consigna "pide la fecha una vez al empezar"
    char fecha[11];
    cout << "Fecha del dia (DD-MM-AAAA): ";
    cin >> fecha;

	// el while que se repite mientras se quiera seguir va registrando venta por venta
    char seguir = 's';
    while (seguir == 's' || seguir == 'S') {
        int idMozo;
        char clave[20];
        cout << "Numero de mozo: ";
        cin >> idMozo;
        cout << "Clave: ";
        cin >> clave;

        int posMozo = buscarMozoPorId(mozos, lenMozos, idMozo);
        if (posMozo == -1) {
            cout << "No existe un mozo con ese numero." << endl;
        } else if (!validarClave(mozos, posMozo, clave)) {
            cout << "Clave incorrecta." << endl;
        } else {
            int codigoProducto, cantidad;
            cout << "Codigo de producto: ";
            cin >> codigoProducto;
            cout << "Cantidad: ";
            cin >> cantidad;

            if (registrarVenta(inv, lenInv, mozos, posMozo, fecha,
                               codigoProducto, cantidad)) {
                ventasHoy++;
                unidadesHoy += cantidad; 
                guardarDatosFinales(inv, lenInv, mozos, lenMozos); 
            }
        }

        cout << "Cargar otra venta? (s/n): ";
        cin >> seguir;
    }

	// cuando ya terminaron de cargar todas las ventas del dia
    ordenarPlanillaDelDia(fecha); // al terminar la carga, la planilla queda ordenada por mozo
  
    cout << "Listo. Ventas cargadas hoy: " << ventasHoy
         << " | Unidades vendidas: " << unidadesHoy << endl;
    return 0;
}