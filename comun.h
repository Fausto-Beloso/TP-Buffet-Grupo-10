#ifndef COMUN_H
#define COMUN_H

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

//a. Abre inventario.dat en modo "rb". Lee registro por registro y los guarda en el array inv, incrementando lenInv.
void cargarInventario(Producto inv[], int& lenInv);

//b. Recorre el string de la clave sumando la constante K a cada car cter para cumplir con el requerimiento de seguridad.
void encriptarClave(char clave[], int k);

//c. Hace una B squeda Secuencial en el array de mozos para ver si el nombre ya existe. Retorna la posici n o -1.
int buscarMozo(Mozo mozos[], int len, char nombreBuscado[]);

//d. Hace una busqueda Binaria en el array del inventario (aprovechando que ya viene ordenado por c digo)para encontrar el producto al instante.
int buscarProducto(Producto inv[], int len, int codigoBuscado);

//e. Convierte "DD-MM-AAAA" a "AAAAMMDD" para poder comparar fechas cronológicamente
void fechaAOrden(char fecha[], char resultado[]);

#endif