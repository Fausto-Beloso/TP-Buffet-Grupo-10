#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h"

using namespace std;


const float TASA_COMISION = 0.10f; //comision a calcular


//----- Funciones propias de ventas declaradas -----

//a. abre mozos.dat creado en normalizacion.cpp
void cargarMozos (Mozo mozos[], int &lenMozos);

//b. ya no se busca mozo por nombre sino que por id
int buscarMozoPorId (Mozo mozos[], int lenMozos, int idBuscado);

//c. copia la clave tipeada y la compara con la original
bool validarClave (Mozo mozos[], int pos, char claveIngresada[]);

//d. busca el producto (binaria), chequea stock, calcula comision, descuenta stock, suma comision al mozo y agrega el registro a ventas[]
bool registrarVenta(Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad);

//e. selección, pero aca compara solo por idMozo
void ordenarPlanilla (Comanda ventas[], int lenVentas);

//f. actualiza inventario.dat y mozos.dat
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
    sprintf(nombreArchivo, "comandas%s.dat", fecha);
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
	// declaración en RAM, cada uno con su contador
    const int MAX_PROD = 100;
    const int MAX_MOZOS = 100;
    const int MAX_VENTAS_DIA = 200;

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
  
    cout << "Listo. Ventas cargadas hoy: " << ventasHoy
         << " | Unidades vendidas: " << unidadesHoy << endl;
    return 0;
}