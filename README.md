Grupo 10 — Algoritmos y Estructuras de Datos, UTN FRBA

Integrantes:
- Fausto Beloso — [@Fausto-Beloso]
- Lautaro Nicolás Bonafina — [@lautabona]
- Francisco Trotta — [@frantrotta06]

Cómo compilar y correr

1. Copiar los archivos que provee la cátedra a la carpeta de trabajo:
  
   cp datos/comandas_historicas.dat datos/inventario.dat .
  
2. Compilar los 4 programas:
  
   g++ -O2 -Wall -o normalizacion normalizacion.cpp
   g++ -O2 -Wall -o ventas        ventas.cpp
   g++ -O2 -Wall -o cierre        cierre.cpp
   g++ -O2 -Wall -o resumen       resumen.cpp

3. Ejecutar en este orden (cada uno depende de lo que deja el anterior):

   ./normalizacion   # historicas + inventario -> mozos.dat, comandas_dd-mm-aaaa.dat (uno por dia), inventario actualizado
   ./ventas          # carga interactiva de ventas de UN dia -> agrega/crea la planilla de ese dia
   ./cierre          # junta los dias que se le pidan -> comandas_semana_sX-mm.dat
   ./resumen         # pide numero de semana y mes -> imprime el resumen por mozo y el total

   'ventas.cpp' se corre una vez por cada día de la semana que se quiera
   cargar: 'cierre.cpp' se corre una sola vez al final de la semana, pidiendo
   fecha por fecha.

En Windows (CMD/PowerShell) la compilación es igual pero sin `-O2 -Wall`
si no se quiere, y la ejecución cambia de `./programa` a `.\programa`. Por
ejemplo, para compilar y correr `normalizacion.cpp` y `ventas.cpp`:
> g++ normalizacion.cpp -o normalizacion
> .\normalizacion
> g++ ventas.cpp -o ventas
> .\ventas
Lo mismo aplica para 'cierre.cpp' y 'resumen.cpp'.

- Diseño de datos

Structs definidas en 'comun.h'.

struct Producto { int codigo; char descripcion[50]; float precio; int stockActual; };
struct Mozo     { int idMozo; char nombre[50]; char password[20]; float totalComision; };
struct Comanda  { char fecha[11]; int idMozo; int codigoProducto; int cantidad; float comision; };

Se agregó 'fecha' a 'Comanda' (no estaba en la struct sugerida por la
cátedra) porque 'normalizacion.cpp' necesita poder ordenar todas las ventas
cronológicamente antes de separarlas por día, y 'cierre.cpp' necesita saber
la fecha de cada venta para calcular el número de semana del cierre.

- Decisiones clave

IDs de mozo y acceso directo. Al detectar un mozo nuevo en el histórico,
se le asigna 'idMozo = lenMozos + 1'. Como los mozos quedan en el array en
ese mismo orden, después se puede acceder a cualquiera por índice
('mozos[id - 1]') sin recorrer nada, en vez de buscar linealmente cada vez
que se necesita.

Búsqueda de productos. El inventario provisto ya viene ordenado por
código, así que se busca por búsqueda binaria.

Ordenamiento. Se usa seleccion. En 'normalizacion.cpp' el criterio
es fecha (convertida internamente de 'DD-MM-AAAA' a 'AAAAMMDD' para que
'strcmp' compare cronológicamente bien) y, en caso de empate, 'idMozo'. En
'ventas.cpp' y 'cierre.cpp' alcanza con ordenar por 'idMozo', ya que cada
archivo corresponde a un único día o ya viene agrupado.

Corte de control. Se usa dos veces: en 'normalizacion.cpp' para separar
el array ya ordenado por fecha en un archivo por día (se recorre una vez y
cada cambio de fecha cierra el archivo actual y abre el siguiente), y en
'resumen.cpp' para acumular cantidad y comisión por mozo sobre la planilla
semanal ya ordenada por 'idMozo', imprimiendo el subtotal cada vez que
cambia el mozo. El corte por cambio de valor resuelve solo el caso del
último mozo (no hace falta "mirar el siguiente registro").

Actualización de stock sin reconstruir el archivo. 'inventario.dat' no
se recrea desde cero: se carga entero a un array en RAM, se descuenta el
stock vendido en memoria, y se vuelve a grabar completo con un único
'fwrite' en modo' '"wb"'.

Login y clave encriptada. Para no guardar las claves en texto plano, se
usa un cifrado por corrimiento con una constante 'K = 7': a
cada carácter de la clave se le suma 'K' para guardarla, y para validar el
login se le aplica la misma cuenta a lo que tipea el mozo y se compara con
lo guardado. Es reversible (restando 'K' se recupera el original), a
diferencia de dividir, que pierde información. La clave inicial de cada
mozo, generada por 'normalizacion.cpp', es su propio número de mozo como
texto, ya guardada encriptada.

- Casos raros contemplados

- Mozo inexistente o clave incorrecta: 'ventas.cpp' no deja cargar la
  venta y vuelve a pedir los datos en el siguiente intento.
- Producto inexistente: se avisa y no se registra la venta.
- Stock insuficiente: en 'normalizacion.cpp' (histórico) se deja el
  stock en 0 y se avisa, sin permitir valores negativos; en 'ventas.cpp' se
  rechaza directamente la venta.
- Planilla del día que todavía no existe: tanto 'ventas.cpp' (al cargar)
  como 'cierre.cpp' (al juntar la semana) lo tratan como caso normal, no
  como error — si es el primer alta del día, se crea; si 'cierre.cpp' no
  encuentra un día, avisa y sigue con los demás.
- Último mozo del cierre: cubierto por el corte de control de
  'resumen.cpp', que imprime el subtotal en cuanto detecta el cambio de
  'idMozo' (o el fin del array), sin necesidad de un caso especial.

- Reparto de trabajo

Los 3 integrantes trabajamos juntos en los 4 programas y en 'comun.h',
sin dividir un programa por persona: discutimos y programamos en conjunto
cada parte (búsquedas, ordenamiento, corte de control, encriptación de
claves) para que los tres pudiéramos entender y defender cualquier
sección del código.
