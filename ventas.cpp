#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h"

using namespace std;


const float TASA_COMISION = 0.10f; //comision a calcular


//----- Funciones propias de ventas declaradas -----

//a. copia la clave tipeada y la compara con la original
bool validarClave (Mozo mozos[], int pos, char claveIngresada[]);

//b. Edita en el lugar el registro del producto en inventario.dat: fseek, fread, fseek hacia atras, fwrite (la misma de normalizacion).
bool descontarStockEnArchivo(FILE* fInv, int posProd, int cantidad, int &stockPrevio);

//c. Edita en el lugar el registro del mozo en mozos.dat ("rb+", fseek, fwrite), sin rehacer el archivo.
bool actualizarMozoEnArchivo(Mozo &m, int posMozo);

//d. busca el producto, chequea stock, calcula comision, graba la venta directo en la planilla del dia ("ab"),
//   descuenta el stock en inventario.dat EN EL LUGAR y actualiza al mozo en mozos.dat EN EL LUGAR
bool registrarVenta(FILE* fInv, Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad);

//e. ordena la planilla del dia por idMozo directo sobre el archivo (seleccion con fseek), sin limite de ventas
void ordenarPlanillaDelDia (char fecha[]);

//----- Funciones propias de ventas desarrolladas -----


// con la posicion de mozo encontrado, compara las contrasenias
bool validarClave(Mozo mozos[], int pos, char claveIngresada[]) {
    char copia[20];
    strcpy(copia, claveIngresada); // trabajamos sobre una copia para no encriptar la clave de una, reservamos exactamente lo que tipeo el mozo
    encriptarClave(copia, K);      // ahora si encriptamos la clave tipeada por el mozo
    return strcmp(copia, mozos[pos].password) == 0;
}

bool descontarStockEnArchivo(FILE* fInv, int posProd, int cantidad, int &stockPrevio) {
    Producto p;

    fseek(fInv, (long)posProd * (long)sizeof(Producto), SEEK_SET); 
    fread(&p, sizeof(Producto), 1, fInv);                          

    stockPrevio = p.stockActual;
    bool alcanzo = (p.stockActual >= cantidad);
    if (alcanzo) {
        p.stockActual = p.stockActual - cantidad;
    } else {
        p.stockActual = 0;
    }

    fseek(fInv, -(long)sizeof(Producto), SEEK_CUR);               
    fwrite(&p, sizeof(Producto), 1, fInv);                  
    return alcanzo;
}


// reescribe SOLO el registro del mozo, sin rehacer el archivo
bool actualizarMozoEnArchivo(Mozo &m, int posMozo) {
    FILE* f = fopen("mozos.dat", "rb+"); // lee/escribe sin borrar el contenido
    if (f == NULL) {
        cout << "No se pudo abrir mozos.dat" << endl;
        return false;
    }
    fseek(f, (long)posMozo * (long)sizeof(Mozo), SEEK_SET); 
    fwrite(&m, sizeof(Mozo), 1, f);                          
    fclose(f);
    return true;
}


// para el problema de que se quiera vender y no haya stock, decidimos rechazar la venta
bool registrarVenta(FILE* fInv, Producto inv[], int lenInv, Mozo mozos[], int posMozo, char fecha[], int codigoProducto, int cantidad){
    if (cantidad <= 0) {
        cout << "La cantidad tiene que ser mayor a 0." << endl;
        return false;
    }
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
    

    // recien ahora, con la venta ya grabada, se actualizan el stock y la comision

    int stockPrevio;
    descontarStockEnArchivo(fInv, posProd, cantidad, stockPrevio);
    fclose(fInv); 
    inv[posProd].stockActual = stockPrevio - cantidad; 
    // comision del mozo: se edita SOLO ese mozo en mozos.dat
    mozos[posMozo].totalComision += comision;
    actualizarMozoEnArchivo(mozos[posMozo], posMozo);

    return true;
}
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

int main() {
    // declaracion en RAM, cada uno con su contador
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

    // inventario.dat se abre UNA sola vez en "rb+" (lee y escribe sin borrar) y se usa para editar en el lugar
    FILE* fInv = fopen("inventario.dat", "rb+");
    if (fInv == NULL) {
        cout << "No se pudo abrir inventario.dat" << endl;
        return 1;
    }
    char fecha[11];
    cout << "Fecha del dia (DD-MM-AAAA): ";
    cin >> fecha;


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

            if (registrarVenta(fInv, inv, lenInv, mozos, posMozo, fecha,
                               codigoProducto, cantidad)) {
                ventasHoy++;
                unidadesHoy += cantidad;
            }
        }

        cout << "Cargar otra venta? (s/n): ";
        cin >> seguir;
    }

    fclose(fInv); // se cierra al terminar la carga

    // cuando ya terminaron de cargar todas las ventas del dia
    ordenarPlanillaDelDia(fecha); // al terminar la carga, la planilla queda ordenada por mozo

    cout << "Listo. Ventas cargadas hoy: " << ventasHoy
         << " | Unidades vendidas: " << unidadesHoy << endl;
    return 0;
}