#include <iostream>
#include <cstdio>
#include <cstring>
#include "comun.h" // contiene structs base

using namespace std;


//----- Struct -----

struct ComandaHistorica{
	char  fecha[11];        // "DD-MM-AAAA" 
	char  nombreMozo[50];   // el nombre completo, repetido en cada venta 
	int   codigoProducto;   
	int cantidad;   
	float comision;
};

//----- Funciones propias de normalizacion declaradas -----

//a. Abre comandas_historicas.dat. Por cada venta, busca/crea al mozo, busca el producto, descuenta el stock EN EL LUGAR dentro de inventario.dat (rb+), calcula la comision y guarda la venta normalizada en el array ventas.
void procesarHistorico(Producto inv[], int lenInv, Mozo mozos[], int &lenMozos, Comanda ventas[], int &lenVentas);

//b'. Edita en el lugar el registro del producto en inventario.dat: fseek, fread, fseek hacia atras, fwrite.
bool descontarStockEnArchivo(FILE* fInv, int posProd, int cantidad, int &stockPrevio);

//c. Ordena el array de ventas en RAM por fecha y luego por ID de mozo, preparandolo para el corte de control.
void ordenarVentas(Comanda ventas[], int lenVentas);

//d. Aplica el patron Corte de Control agrupando por fecha. Crea los archivos comandas_dd-mm-aaaa.dat y los graba.
void generarArchivosDiarios(Comanda ventas[], int lenVentas);

//e. Abre mozos.dat en modo "wb" (que crea o vacia el archivo) y guarda el array actualizado. El inventario ya no se reescribe: se edita en el lugar en procesarHistorico.
void guardarDatosFinales(Mozo mozos[], int lenMozos);

//----- Funciones propias de normalizacion desarrolladas -----

// Descuenta stock editando el registro directamente en inventario.dat.
// Devuelve false si el stock no alcanzaba (el stock queda en 0).
// stockPrevio devuelve el stock que habia antes de descontar (sirve para el aviso).
bool descontarStockEnArchivo(FILE* fInv, int posProd, int cantidad, int &stockPrevio) {
    Producto p;

    fseek(fInv, (long)posProd * (long)sizeof(Producto), SEEK_SET); // 1) ubicar el registro
    fread(&p, sizeof(Producto), 1, fInv);                          // 2) leerlo (el puntero queda en el registro siguiente)

    stockPrevio = p.stockActual;
    bool alcanzo = (p.stockActual >= cantidad);
    if (alcanzo) {
        p.stockActual = p.stockActual - cantidad;
    } else {
        p.stockActual = 0;
    }

    fseek(fInv, -(long)sizeof(Producto), SEEK_CUR);                // 3) volver al inicio del registro (tambien habilita pasar de leer a escribir)
    fwrite(&p, sizeof(Producto), 1, fInv);                         // 4) pisar solo ese registro
    return alcanzo;
}

void procesarHistorico(Producto inv[], int lenInv, Mozo mozos[], int &lenMozos, Comanda ventas[], int &lenVentas) {
    FILE* fHist = fopen("comandas_historicas.dat", "rb");
    if (fHist == NULL) {
        cout << "No se pudo abrir el archivo" << endl;
        return;
    }

    // "rb+" lee y escribe sin borrar el contenido. El archivo tiene que existir.
    FILE* fInv = fopen("inventario.dat", "rb+");
    if (fInv == NULL) {
        cout << "No se pudo abrir inventario.dat" << endl;
        fclose(fHist);
        return;
    }

    ComandaHistorica vieja;
    while (fread(&vieja, sizeof(ComandaHistorica), 1, fHist) == 1) {

        // 1)  se valida el producto: si no existe, se descarta la venta ENTERA
        
        int posProd = buscarProducto(inv, lenInv, vieja.codigoProducto);
        if (posProd == -1) {
            cout << "No se encontro el Producto " << vieja.codigoProducto
                 << " (" << vieja.fecha << ", mozo " << vieja.nombreMozo << "). Venta descartada." << endl;
            continue;
        }

        // 2) La venta es valida: ahora si se busca/crea al mozo
        int posMozo = buscarMozo(mozos, lenMozos, vieja.nombreMozo);

        if (posMozo == -1) {
            posMozo = lenMozos;

            mozos[lenMozos].idMozo = lenMozos + 1;
            strcpy(mozos[lenMozos].nombre, vieja.nombreMozo);
            mozos[lenMozos].totalComision = 0;
            sprintf(mozos[posMozo].password, "%d", mozos[posMozo].idMozo);
            encriptarClave(mozos[posMozo].password, K);

            lenMozos++;
        }

        // 3) Comision: solo para ventas validas
        mozos[posMozo].totalComision += vieja.comision;

        // 4) Stock: se edita en el archivo, sin reescribirlo entero
        int stockPrevio;
        bool alcanzo = descontarStockEnArchivo(fInv, posProd, vieja.cantidad, stockPrevio);

        if (!alcanzo) {
            cout << "Aviso: stock insuficiente para el producto " << vieja.codigoProducto
                 << " (" << vieja.fecha << ", mozo " << vieja.nombreMozo << "). "
                 << "Stock disponible: " << stockPrevio
                 << ", se pidieron: " << vieja.cantidad << endl;
            inv[posProd].stockActual = 0;
        } else {
            inv[posProd].stockActual = stockPrevio - vieja.cantidad;
        }
        // (el array inv se actualiza tambien para que quede igual que el archivo)

        // 5) Venta normalizada
        strcpy(ventas[lenVentas].fecha, vieja.fecha);
        ventas[lenVentas].idMozo = mozos[posMozo].idMozo;
        ventas[lenVentas].codigoProducto = vieja.codigoProducto;
        ventas[lenVentas].cantidad = vieja.cantidad;
        ventas[lenVentas].comision = vieja.comision;

        lenVentas++;
    }

    fclose(fInv);
    fclose(fHist);
}

//ordenar el array de ventas por fecha e id
void ordenarVentas(Comanda ventas[], int lenVentas){
	//seleccion, comparando fecha convertida a AAAAMMDD (orden cronologico real)
    for (int i = 0; i < lenVentas - 1; i++){
        int posMenor = i;
        for (int j = i + 1; j < lenVentas; j++){
            char ordenJ[9], ordenMenor[9];
            fechaAOrden(ventas[j].fecha, ordenJ);
            fechaAOrden(ventas[posMenor].fecha, ordenMenor);

            if (strcmp(ordenJ, ordenMenor) < 0 ||
               (strcmp(ordenJ, ordenMenor) == 0 && ventas[j].idMozo < ventas[posMenor].idMozo)) { //desempate por idMozo si las fechas son iguales
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

//generar archivos diarios con corte de control creando el archivo comandas_dd-mm-aaaa.dat
void generarArchivosDiarios(Comanda ventas[], int lenVentas){
	//depende de que ventas[] ya venga ordenado
    int i = 0;
    while (i < lenVentas) {
        char fechaActual[11];
        strcpy(fechaActual, ventas[i].fecha); // fija la fecha del grupo que arranca ahora

        char nombreArchivo[30];
        sprintf(nombreArchivo, "comandas_%s.dat", fechaActual); // arma el nombre del archivo de ESE dia

        FILE* fDia = fopen(nombreArchivo, "wb");
        if (fDia == NULL) {
            cout << "No se pudo crear " << nombreArchivo << endl;
            return;
        }

        while (i < lenVentas && strcmp(ventas[i].fecha, fechaActual) == 0) {
            fwrite(&ventas[i], sizeof(Comanda), 1, fDia);
            i++;
        }
        fclose(fDia);
    }
}

//guardar datos finales: solo mozos.dat en modo "wb". El inventario ya se actualizo en el lugar.
void guardarDatosFinales(Mozo mozos[], int lenMozos){
    FILE* fMoz = fopen("mozos.dat", "wb");
    if (fMoz == NULL) {
        cout << "No se pudo guardar mozos.dat" << endl;
        return;
    }
    fwrite(mozos, sizeof(Mozo), lenMozos, fMoz);
    fclose(fMoz);
}


int main(){
	// declaracion en RAM, cada uno con su contador
    const int MAX_PROD = 100;
    const int MAX_MOZOS = 100;
    const int MAX_VENTAS = 1000;

    Producto inv[MAX_PROD];
    int lenInv = 0;

    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;

    Comanda ventas[MAX_VENTAS];
    int lenVentas = 0;

    cargarInventario(inv, lenInv);
    procesarHistorico(inv, lenInv, mozos, lenMozos, ventas, lenVentas);
    ordenarVentas(ventas, lenVentas);
    generarArchivosDiarios(ventas, lenVentas);
    guardarDatosFinales(mozos, lenMozos);

    cout << "Normalizacion completada. Mozos: " << lenMozos << " | Ventas: " << lenVentas << endl;

    return 0;
}