    #include <iostream>
    #include <cstring>
    #include <cstdio>
    using namespace std;

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


void cargarMozo (Mozo mozos[], int &lenMozos){
    FILE* Mozitos = fopen("mozos.dat", "rb");
    if (mozos == NULL) {
         cout << "No se pudo abrir el archivo." << endl;
         return;
    }

    while (fread(&mozos[lenMozos], sizeof(Mozo), 1, Mozitos) == 1)
    {
        lenMozos++;
    }
    fclose(Mozitos);
}


void cargarInventario(Producto inv[], int & lenMozos){
    FILE * inventario = fopen("inventario.dat ", "rb ");
    if(inventario == NULL){
        cout<<"no se pudo abrir inventario.dat";
        return ;

    }
    lenMozos = 0; 
    while ((fread(&inv[lenMozos], sizeof(Producto), 1, inventario) == 1))
    {
        lenMozos++;
    }

    fclose(inventario);
    

}



int buscarMozo(Mozo mozos[], int lenMozos, int idIngresado, char password[])
{
    for(int i = 0; i < lenMozos; i++)
    {
        if(mozos[i].idMozo == idIngresado &&  strcmp(mozos[i].password, password) == 0)
        {
            return i;
        }
    }

    return -1;
}

void encriptarClave(char clave[], int k){
	int i=0;
	while (clave[i] != '\0') // mientras no llegue al final del array. mientras la clave todavia exista
	{
		clave[i] = clave[i] + k;
		i++;
	}
}

bool validarMozo(Mozo mozos[], int lenMozos, int idMozo, char clave[], int k)
{
    int pos = -1;

    for(int i = 0; i < lenMozos; i++)
    {
        if(mozos[i].idMozo == idMozo)
        {
            pos = i;
            break;
        }
    }

    if(pos == -1)
    {
        return false;
    }

    encriptarClave(clave, k);

    if(strcmp(clave, mozos[pos].password) == 0)
    {
        return true;
    }

    return false;
}


//ordena las ventas por idMozo.


    void ordenarPlanilla(Comanda ventas[], int lenVentas) {
    for (int i = 0; i < lenVentas - 1; i++) {
        for (int j = 0; j < lenVentas - i - 1; j++) {
            // Comparamos el atributo idMozo del elemento actual con el siguiente
            if (ventas[j].idMozo > ventas[j + 1].idMozo) {
                // Intercambiamos la Comanda completa
                Comanda temp = ventas[j];
                ventas[j] = ventas[j + 1];
                ventas[j + 1] = temp;
            }
        }
    }
}
//Por que usamos burbuja?Mantiene la misma lógica que en un arreglo de enteros. El bucle externo i controla la cantidad de pasadas necesarias, 
// y el bucle interno j va comparando elementos adyacentes limitando el recorrido con - i (ya que los valores más grandes "flotan" al final en cada pasada).

const float TASA_COMISION = 0.10;
void registrarVenta(Producto inv[], int cantProductos,
                     int idMozo, int codigoBuscado, int cantidad,
                     Comanda comandas[], int & len){
    //1) AHORA DEBERIAMOS BUSCAR EL PRODUCTO 
    int StockExiste = buscarProducto (inv, len, codigoBuscado);
    if (StockExiste == -1 )
    {
        cout<<"El producto no existe ";

    }

    //2)Validar si hay stock del producto;
    if (inv[StockExiste].stockActual < cantidad)
    {
        
    }
    

                   
                   
 }





int buscarProducto(Producto inv[], int len, int codigoBuscado){
    int izquierda = 0;
    int derecha = len - 1; 
    while(izquierda <= derecha){
        int medio = (izquierda + derecha) / 2;
        if (inv[medio].codigo == codigoBuscado)
        {
            return medio;
        } else if (inv[medio].codigo < codigoBuscado)
        {
          izquierda = medio + 1;
        }
        else{
            derecha = medio - 1 ;
        }
        
        
    }
    return -1; 
}




int main(){
 
}   