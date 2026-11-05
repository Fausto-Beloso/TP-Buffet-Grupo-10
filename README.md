TP Integrador — Buffet Albert Einstein

Grupo 10

Integrantes:

Fausto Beloso — @Fausto-Beloso
Lautaro Nicolás Bonafina — @lautabona
Francisco Trotta — @frantrotta06

Cómo compilar y correr

Copiar los archivos que provee la cátedra a la carpeta de trabajo (los de datos/ no se tocan):

bash
cp datos/comandas_historicas.dat datos/inventario.dat .

Compilar los 4 programas (cada uno tiene su propio main):

bash
g++ -O2 -o normalizacion normalizacion.cpp
g++ -O2 -o ventas        ventas.cpp
g++ -O2 -o cierre        cierre.cpp
g++ -O2 -o resumen       resumen.cpp

En Windows (CMD/PowerShell) la compilación es la misma y se ejecuta con .\programa en lugar de ./programa.

Ejecutar en este orden (cada uno depende de lo que deja el anterior):

./normalizacion   # historicas + inventario -> mozos.dat, comandas_dd-mm-aaaa.dat (uno por día), inventario actualizado
./ventas          # carga interactiva de ventas de UN día -> agrega/crea la planilla de ese día, ordenada por mozo
./cierre          # junta los días que se le pidan -> comandas_semana_sX-mm.dat
./resumen         # pide número de semana y mes -> imprime el resumen por mozo y el total
ventas se corre una vez por cada día que se quiera cargar.
cierre se corre una vez al final de la semana y pide las fechas una por una (formato DD-MM-AAAA).
normalizacion modifica inventario.dat en el lugar: para repetir la prueba hay que volver a copiar los originales de datos/.

Con los datos de la cátedra, el cierre de la semana da 30 comandas, 49 productos vendidos y $8770 de comisión.

Diseño de datos

Los structs están en comun.h (junto con las funciones compartidas); ComandaHistorica está en normalizacion.cpp, porque solo ese programa la usa.

cpp
struct Producto { int codigo; char descripcion[50]; float precio; int stockActual; };
struct Mozo     { int idMozo; char nombre[50]; char password[20]; float totalComision; };
struct Comanda  { char fecha[11]; int idMozo; int codigoProducto; int cantidad; float comision; };

Todos los campos de texto son char[N], de tamaño fijo, para que los registros se puedan escribir y leer en binario.

Sobre fecha en Comanda: no estaba en la struct sugerida por la cátedra y la agregamos porque normalizacion.cpp necesita ordenar todas las ventas cronológicamente y cortar por día antes de generar un archivo por cada fecha. Reconocemos que en disco es redundante, porque el nombre de cada archivo ya incluye la fecha. cierre.cpp no la usa: calcula la semana a partir de la fecha que se le tipea.

Decisiones técnicas

IDs de mozo y acceso directo. Al detectar un mozo nuevo en el histórico se le asigna idMozo = lenMozos + 1. Como mozos.dat queda en ese orden, el mozo con id N está en la posición N - 1: se accede por índice (mozos[id - 1]), sin recorrer nada. El mismo cálculo ((id - 1) * sizeof(Mozo)) se usa para editar su registro en el archivo. En el histórico, en cambio, el único dato que identifica al mozo es el nombre y no hay orden, así que ahí se usa búsqueda lineal por nombre.

Búsqueda de productos. El inventario viene ordenado por código (con huecos), así que se busca por búsqueda binaria.

Ordenamiento. Se usa selección en los dos lugares donde hace falta:

normalizacion.cpp: ordena en RAM por fecha y, en caso de empate, por idMozo. La fecha se convierte internamente de DD-MM-AAAA a AAAAMMDD (fechaAOrden), porque strcmp sobre DD-MM-AAAA no da orden cronológico.
ventas.cpp: al terminar la carga ordena la planilla del día por idMozo directamente sobre el archivo (modo rb+, fseek, fread, fwrite), con solo tres registros en memoria. Así la cantidad de ventas del día no tiene un máximo.

Corte de control. Se usa dos veces:

normalizacion.cpp: recorre el array ya ordenado por fecha y cada cambio de fecha cierra el archivo del día y abre el siguiente.
resumen.cpp: recorre la planilla semanal ya ordenada por idMozo y acumula productos y comisión; imprime el subtotal cuando cambia el mozo. Como el corte se dispara también al terminar los datos, el último mozo se resuelve solo, sin caso especial.

Apareo en cierre.cpp. Cada planilla diaria ya está ordenada por idMozo, así que se une con el acumulado semanal mediante apareo: se lee un registro de cada archivo, se escribe el de menor idMozo en un archivo temporal (cierre_tmp.dat) y al final el temporal reemplaza al acumulado. No se carga nada en memoria. Si empatan, sale primero el acumulado (días anteriores). El nombre del semanal (comandas_semana_sX-mm.dat) se arma con la primera fecha válida: X = (día - 1) / 7 + 1 y mm es el mes.

Actualización en el lugar (sin rehacer archivos).

Stock: descontarStockEnArchivo abre inventario.dat en rb+, hace fseek al registro (posProd * sizeof(Producto)), lo lee con fread (el puntero queda en el registro siguiente), vuelve con fseek(..., -sizeof(Producto), SEEK_CUR) y pisa solo ese registro con fwrite. Se usa en normalizacion.cpp y en ventas.cpp. El array inv en RAM se mantiene igual al archivo y se usa solo para buscar.
Comisión del mozo en ventas.cpp: actualizarMozoEnArchivo hace rb+, fseek a (id - 1) * sizeof(Mozo) y fwrite de ese único registro.
mozos.dat en normalizacion.cpp sí se crea de cero con "wb", porque en ese momento todavía no existe.

Ventas sin límite. Cada venta se graba directo en la planilla del día con "ab" (agrega al final y crea el archivo si no existe), así que no hay un máximo de ventas por día y, si el programa se corta, las ventas ya cargadas no se pierden.

Login y clave encriptada. Cifrado por corrimiento con K = 7, implementado a mano: a cada carácter de la clave se le suma K para guardarla, y para validar el login se aplica la misma cuenta a lo que tipea el mozo y se compara con lo guardado (sobre una copia, para no pisar lo tipeado). Es reversible (restando K se recupera el original), a diferencia de una operación como dividir, que pierde información. La clave inicial de cada mozo, generada por normalizacion.cpp, es su propio número de mozo como texto, ya guardada encriptada.

Casos raros contemplados
Mozo inexistente o clave incorrecta (ventas): se avisa, no se registra nada y se vuelve a pedir en el siguiente intento.
Producto inexistente: en ventas se avisa y no se registra la venta. En el histórico (normalizacion) la venta se descarta entera y no suma comisión al mozo, para que mozos.dat y las planillas queden consistentes.
Stock insuficiente: en el histórico se deja el stock en 0 y se avisa (sin valores negativos); en ventas se rechaza la venta.
Cantidad menor o igual a 0 (ventas): se rechaza antes de tocar stock o comisión.
Planilla del día que no existe: no es un error. ventas la crea con la primera venta; cierre avisa que no existe y sigue con las demás fechas; si al terminar la carga la planilla no existe (no se aceptó ninguna venta), ventas no tiene nada que ordenar y sigue sin error.
Último mozo del resumen: lo cubre el corte de control.
Limitaciones conocidas
Las lecturas con cin >> char[] (fecha y clave) no limitan la cantidad de caracteres.
normalizacion.cpp usa arrays de tamaño fijo (por ejemplo, 1000 ventas históricas) y resumen.cpp carga la planilla semanal en un array de 1000.
El ordenamiento sobre archivo en ventas es O(n²) en lecturas; es aceptable para el volumen de un día del buffet.
Si se carga la misma fecha dos veces en cierre, esas ventas se suman dos veces.
Correr normalizacion dos veces descuenta el stock dos veces; hay que volver a copiar los originales.
La semana se calcula con la convención (día - 1) / 7 + 1; el enunciado no la define.
Reparto de trabajo

Los 3 integrantes trabajamos juntos en los 4 programas y en comun.h, sin dividir un programa por persona: discutimos y programamos en conjunto cada parte (búsquedas, ordenamiento, corte de control, apareo, encriptación de claves) para que los tres pudiéramos entender y defender cualquier sección del código. Los commits de cada integrante están en el historial del repositorio.