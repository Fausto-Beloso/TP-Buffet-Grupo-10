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

//d. abre comandas_<fecha>.dat
//si no existe todavia, no es un error (es el primer alta del dia) arranca con lenVentas=0 sin mensaje de alarma
//si existe carga lo que ya habia para no pisarlo
void cargarPlanillaDia (Comanda vent[], int &lenVentas, char fecha[]);

//e. busca el producto (binaria), chequea stock, calcula comision, descuenta stock, suma comision al mozo y agrega el registro a ventas[]
bool registrarVenta(Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad, Comanda ventas[], int &lenVentas);

//f. selección, pero aca compara solo por idMozo
void ordenarPlanilla (Comanda ventas[], int lenVentas);

//g. vuelca el array ordenado al archivo del día en modo "wb"
void guardarPlanillaDia (Comanda ventas[], int lenVentas, char fecha[]);

//h. actualiza inventario.dat y mozos.dat
void guardarDatosFinales (Producto inv[], int lenInv, Mozo mozos[], int lenMozos);

//----- Funciones propias de ventas desarrolladas -----

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

// solo busca al mozo retornando la posicion
int buscarMozoPorId(Mozo mozos[], int lenMozos, int idBuscado) {
    if (idBuscado < 1 || idBuscado > lenMozos) {
        return -1; // fuera de rango, no existe ese mozo
    }
    return idBuscado - 1; // acceso directo, sin recorrer nada // decision tomada en normalizacion al asignar el id al mozo = lenMozos + 1 (para que quede primer id 1 y no 0
}

// con la posicion de mozo encontrado, compara las contrasenias
bool validarClave(Mozo mozos[], int pos, char claveIngresada[]) {
    char copia[20];
    strcpy(copia, claveIngresada); // trabajamos sobre una copia para no encriptar la clave de una, reservamos exactamente lo que tipeo el mozo
    encriptarClave(copia, K);      // ahora si encriptamos la clave tipeada por el mozo
    return strcmp(copia, mozos[pos].password) == 0;
}

void cargarPlanillaDia(Comanda ventas[], int &lenVentas, char fecha[]) {
    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_%s.dat", fecha); // sprintf = printf = cout. sprintf escribe el texto dentro de un char[] en vez de mostrarlo por pantalla
                                                     // %s indica que va un string
    FILE* f = fopen(nombreArchivo, "rb");
    if (f == NULL) {
        return; // primera venta del dia: todavia no existe la planilla, no es error
    }
    while (fread(&ventas[lenVentas], sizeof(Comanda), 1, f) == 1) {
        lenVentas++;
    }
    fclose(f);
}

// para el problema de que se quiera vender y no haya stock, decidimos rechazar la venta
bool registrarVenta(Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad, Comanda ventas[], int &lenVentas){
	 int posProd = buscarProducto(inv, lenInv, codigoProducto);
    if (posProd == -1) {
        cout << "No existe el producto " << codigoProducto << endl;
        return false;
    }
    if (inv[posProd].stockActual < cantidad) {
        cout << "Stock insuficiente. Disponible: " << inv[posProd].stockActual << endl;
        return false; // la venta se rechaza, no se vende lo que no hay
    }

    inv[posProd].stockActual -= cantidad;
    float comision = inv[posProd].precio * cantidad * TASA_COMISION;

    strcpy(ventas[lenVentas].fecha, fecha);
    ventas[lenVentas].idMozo = mozos[posMozo].idMozo;
    ventas[lenVentas].codigoProducto = codigoProducto;
    ventas[lenVentas].cantidad = cantidad;
    ventas[lenVentas].comision = comision;
    lenVentas++;

    mozos[posMozo].totalComision += comision;
    return true;
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

void guardarPlanillaDia(Comanda ventas[], int lenVentas, char fecha[]) {
    char nombreArchivo[30];
    sprintf(nombreArchivo, "comandas_%s.dat", fecha);

    FILE* f = fopen(nombreArchivo, "wb");
    if (f == NULL) {
        cout << "No se pudo guardar " << nombreArchivo << endl;
        return;
    }
    fwrite(ventas, sizeof(Comanda), lenVentas, f);
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
        cout << "No se pudo guardar mozos.dat" << endl;
        return;
    }
    fwrite(mozos, sizeof(Mozo), lenMozos, fMoz);
    fclose(fMoz);
}

int main() {
	// declaración de los 3 "contenedores" en RAM, cada uno con su contador
    const int MAX_PROD = 100;
    const int MAX_MOZOS = 100;
    const int MAX_VENTAS_DIA = 200;

    Producto inv[MAX_PROD];
    int lenInv = 0;
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;
    Comanda ventas[MAX_VENTAS_DIA];
    int lenVentas = 0;
	
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

    cargarPlanillaDia(ventas, lenVentas, fecha);
	
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

            registrarVenta(inv, lenInv, mozos, posMozo, fecha,
                            codigoProducto, cantidad, ventas, lenVentas);
        }

        cout << "Cargar otra venta? (s/n): ";
        cin >> seguir;
    }

	// cuando ya terminaron de cargar todas las ventas del dia
    ordenarPlanilla(ventas, lenVentas);
    guardarPlanillaDia(ventas, lenVentas, fecha);
    guardarDatosFinales(inv, lenInv, mozos, lenMozos);

    cout << "Listo. Ventas cargadas hoy: " << lenVentas << endl;
    return 0;
}