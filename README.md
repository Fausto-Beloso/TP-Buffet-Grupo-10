# TP Integrador - Buffet Albert Einstein

Grupo 10 - Algoritmos y Estructuras de Datos, UTN FRBA

Integrantes:
- Fausto Beloso (@Fausto-Beloso)
- Lautaro Nicolás Bonafina (@lautabona)
- Francisco Trotta (@frantrotta06)


## Cómo compilar y correr

Primero se copian los archivos de la cátedra a la carpeta de trabajo (los de datos/ no se tocan):

    cp datos/comandas_historicas.dat datos/inventario.dat .

Después se compilan los 4 programas, cada uno con su propio main:

    g++ -O2 -o normalizacion normalizacion.cpp
    g++ -O2 -o ventas ventas.cpp
    g++ -O2 -o cierre cierre.cpp
    g++ -O2 -o resumen resumen.cpp

En Windows es igual, pero se ejecuta con .\programa en vez de ./programa.

Orden de ejecución (cada uno usa lo que deja el anterior):

    ./normalizacion    historicas + inventario -> mozos.dat, un comandas_dd-mm-aaaa.dat por día, inventario actualizado
    ./ventas           carga de ventas de UN día -> agrega o crea la planilla de ese día, ordenada por mozo
    ./cierre           junta los días que se le pidan -> comandas_semana_sX-mm.dat
    ./resumen          pide semana y mes -> imprime el resumen por mozo y el total

ventas se corre una vez por cada día que se quiera cargar. cierre se corre una vez al final de la semana y pide las fechas una por una (formato DD-MM-AAAA). normalizacion modifica inventario.dat en el lugar, así que para repetir la prueba hay que volver a copiar los originales de datos/.



## Diseño de datos

Los structs Producto, Mozo y Comanda están en comun.h, junto con las funciones que usan varios programas. ComandaHistorica está en normalizacion.cpp porque solo la usa ese programa. Todos los textos son char[N] de tamaño fijo para poder grabar los registros en binario.

A Comanda le agregamos el campo fecha (no estaba en la struct sugerida). Lo necesitamos en normalizacion para ordenar todas las ventas por fecha y cortar un archivo por día. En disco es medio redundante porque el nombre del archivo ya trae la fecha, y cierre.cpp no lo usa: calcula la semana con la fecha que se tipea.


## Decisiones técnicas

IDs de mozo. Al encontrar un mozo nuevo en el histórico se le asigna idMozo = lenMozos + 1. Como mozos.dat queda en ese orden, el mozo N está en la posición N - 1, así que se accede directo por índice y no hace falta recorrer. En el histórico, en cambio, el mozo solo se identifica por el nombre y no hay orden, así que ahí la búsqueda es lineal.

Búsqueda de productos. El inventario viene ordenado por código (con huecos), por eso se usa búsqueda binaria.

Ordenamiento. Usamos selección. En normalizacion se ordena en memoria por fecha y, si empatan, por idMozo; la fecha se pasa de DD-MM-AAAA a AAAAMMDD para que strcmp compare en orden cronológico. En ventas, al terminar la carga se ordena la planilla del día por idMozo directamente sobre el archivo (rb+, fseek, fread, fwrite), con solo tres registros en memoria, así que no hay límite de ventas por día.

Corte de control. Se usa en normalizacion (cada cambio de fecha cierra el archivo del día y abre el siguiente) y en resumen (se acumula por mozo y se imprime el subtotal cuando cambia el idMozo). Como el corte también se dispara cuando se terminan los datos, el último mozo sale sin ningún caso especial.

Apareo en cierre. Cada planilla diaria ya está ordenada por idMozo, así que se une con el acumulado semanal por apareo: se lee un registro de cada archivo y se escribe el de menor idMozo en un archivo temporal (cierre_tmp.dat), que al final reemplaza al acumulado. No se carga nada en memoria. El nombre del semanal se arma con la primera fecha válida: semana = (día - 1) / 7 + 1, más el mes.

Stock y comisión en el lugar. El stock no se reescribe completo: descontarStockEnArchivo abre inventario.dat en rb+, hace fseek al registro (posProd * sizeof(Producto)), lo lee, vuelve para atrás con fseek(-sizeof, SEEK_CUR) y pisa solo ese registro. Se usa en normalizacion y en ventas. En ventas, la comisión del mozo se actualiza igual: fseek a (id - 1) * sizeof(Mozo) y fwrite de ese registro. Solo mozos.dat en normalizacion se crea de cero con "wb", porque todavía no existe.

Ventas sin límite. Cada venta se graba directo en la planilla del día con "ab" (agrega al final y crea el archivo si no existe). Así no hay máximo por día y, si el programa se corta, no se pierde lo ya cargado.

Login y clave. Cifrado por corrimiento con K = 7, hecho a mano: a cada carácter de la clave se le suma K para guardarla, y para validar el login se hace la misma cuenta con lo que tipea el mozo y se compara (sobre una copia). Es reversible (restando K se recupera la clave), a diferencia de algo como dividir, que pierde información. La clave inicial de cada mozo es su número de mozo como texto, ya guardada encriptada.


## Casos raros

- Mozo inexistente o clave incorrecta: ventas avisa, no registra nada y vuelve a pedir los datos.
- Producto inexistente: en ventas se avisa y no se registra. En el histórico la venta se descarta entera y no suma comisión al mozo, para que mozos.dat y las planillas queden consistentes.
- Stock insuficiente: en el histórico el stock queda en 0 y se avisa. En ventas se rechaza la venta.
- Cantidad menor o igual a 0 en ventas: se rechaza antes de tocar stock o comisión.
- Planilla del día que no existe: no es un error. ventas la crea con la primera venta, cierre avisa y sigue con las otras fechas, y si al terminar la carga no hay planilla, ventas no ordena nada.
- Último mozo del resumen: lo cubre el corte de control, como se explicó arriba.


## Limitaciones conocidas

- Las lecturas con cin >> char[] (fecha y clave) no limitan la cantidad de caracteres.
- normalizacion usa arrays de tamaño fijo (1000 ventas históricas) y resumen carga la semana en un array de 1000.
- Si en cierre se carga dos veces la misma fecha, esas ventas se suman dos veces.
- Correr normalizacion dos veces descuenta el stock dos veces.
- El enunciado no define cómo se numera la semana, usamos (día - 1) / 7 + 1.


## Reparto de trabajo

Los tres trabajamos juntos en los 4 programas y en comun.h, sin dividir un programa por persona. Discutimos y programamos en conjunto cada parte (búsquedas, ordenamiento, corte de control, apareo, encriptación) para que los tres pudiéramos entender y defender cualquier sección del código. Los commits de cada uno están en el historial del repo.