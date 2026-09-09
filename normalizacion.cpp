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

//a. Abre comandas_historicas.dat. Por cada venta, busca/crea al mozo, busca el producto, descuenta el stock, calcula la comisi n y guarda la venta normalizada en el array ventas.
void procesarHistorico(Producto inv[], int lenInv, Mozo mozos[], int &lenMozos, Comanda ventas[], int &lenVentas);

//b. Ordena el array de ventas en RAM por fecha y luego por ID de mozo, prepar ndolo para el corte de control.
void ordenarVentas(Comanda ventas[], int lenVentas);

//c. Aplica el patr n Corte de Control agrupando por fecha. Crea los archivos comandas_dd-mm-aaaa.dat y los graba.
void generarArchivosDiarios(Comanda ventas[], int lenVentas);

//d. Abre inventario.dat y mozos.dat en modo "wb" (que crea o vac a el archivo) y guarda los arrays actualizados.
void guardarDatosFinales(Producto inv[], int lenInv, Mozo mozos[], int lenMozos);

//----- Funciones propias de normalizacion desarrolladas -----

void procesarHistorico(Producto inv[], int lenInv, Mozo mozos[], int &lenMozos, Comanda ventas[], int &lenVentas) {
    FILE* fHist = fopen("comandas_historicas.dat", "rb");
    if (fHist == NULL) {
        cout << "No se pudo abrir el archivo" << endl;
        return;
    }
    ComandaHistorica vieja;
    while (fread(&vieja, sizeof(ComandaHistorica), 1, fHist) == 1) {

        int posMozo = buscarMozo(mozos, lenMozos, vieja.nombreMozo);

        if (posMozo == -1) { // esto lo que hace con el if -1 es que si no lo encuentra crea el primer id y asi para todos los demas que entren
            // Mozo nuevo — acá van, en orden:
            posMozo = lenMozos;
                       
            mozos[lenMozos].idMozo=lenMozos+1; 
            strcpy(mozos[lenMozos].nombre, vieja.nombreMozo); // copiar nombre
            mozos[lenMozos].totalComision=0; // totalComision = 0
            sprintf(mozos[posMozo].password, "%d", mozos[posMozo].idMozo); // generar password. sprintf escribe el resultado dentro de un char[], es como un cout pero en vez de por pantalla en un char
            encriptarClave(mozos[posMozo].password, K); // generar password encriptada
            
            lenMozos++;
        }
        // Corre siempre, mozo nuevo o existente:
        mozos[posMozo].totalComision += vieja.comision; // sumar comisión, 
        
		//buscar producto y descontar stock, armar ventas[]
		int posProd = buscarProducto(inv, lenInv, vieja.codigoProducto); // es vieja.codigoProducto porque al abrir el archivo "comandas_historicas.dat" abrimos el struct Comandas
		
		if (posProd != -1){
			
			if (inv[posProd].stockActual >= vieja.cantidad){
				// hay stock suficiente: se descuenta normal
				inv[posProd].stockActual = inv[posProd].stockActual - vieja.cantidad; // descuenta la mercaderia por venta
			} else {
            // no alcanza el stock: se avisa y se deja en 0 (no se permite que quede negativo)
            cout << "Aviso: stock insuficiente para el producto " << vieja.codigoProducto
            << " (" << vieja.fecha << ", mozo " << vieja.nombreMozo << "). "
            << "Stock disponible: " << inv[posProd].stockActual
            << ", se pidieron: " << vieja.cantidad << endl;
            inv[posProd].stockActual = 0;
            }

			strcpy(ventas[lenVentas].fecha, vieja.fecha);
			ventas[lenVentas].idMozo = mozos[posMozo].idMozo; 
			ventas[lenVentas].codigoProducto = vieja.codigoProducto;
			ventas[lenVentas].cantidad = vieja.cantidad;
			ventas[lenVentas].comision  = vieja.comision;
	
			lenVentas++;
		}
		else {
			cout << "No se encontro el Producto" << endl;
		}
    }

    fclose(fHist);
}

//ordenar el array de ventas por fecha e id
void ordenarVentas(Comanda ventas[], int lenVentas){
	//selección, comparando fecha convertida a AAAAMMDD (orden cronológico real)
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
        sprintf(nombreArchivo, "comandas_%s.dat", fechaActual); // arma el nombre del archivo de ESE día

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

//guardar datos finales abriendo inventario.dat y mozos.dat en modo "wb" (que crea o vac a el archivo) y guarda los arrays actualizados
void guardarDatosFinales(Producto inv[], int lenInv, Mozo mozos[], int lenMozos){
    FILE* fInv = fopen("inventario.dat", "wb");
    if (fInv == NULL) {
        cout << "No se pudo guardar inventario.dat" << endl;
        return;
    }
    fwrite(inv, sizeof(Producto), lenInv, fInv); // un solo fwrite para TODO el array
    fclose(fInv);

    FILE* fMoz = fopen("mozos.dat", "wb");
    if (fMoz == NULL) {
        cout << "No se pudo guardar mozos.dat" << endl;
        return;
    }
    fwrite(mozos, sizeof(Mozo), lenMozos, fMoz);
    fclose(fMoz);
}


int main(){
	// declaración de los 3 "contenedores" en RAM, cada uno con su contador
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
    guardarDatosFinales(inv, lenInv, mozos, lenMozos);

    cout << "Normalizacion completada. Mozos: " << lenMozos << " | Ventas: " << lenVentas << endl;

    return 0;
}