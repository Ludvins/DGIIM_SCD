// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que recibe mensajes síncronos de forma alterna.
// (versión con un único productor y un único consumidor)
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------


#include <iostream>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>  // includes de MPI
#include <atomic>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

// ---------------------------------------------------------------------
// constantes que determinan la asignación de identificadores a roles:
const int
num_productores       = 4,
num_consumidores      = 5,
id_productor          = num_productores - 1, // identificador del proceso productor
id_buffer             = num_productores,  // identificador del proceso buffer
id_consumidor         = id_buffer + num_consumidores, // identificador del proceso consumidor
num_procesos_esperado = 10,  // número total de procesos esperado
num_items             = 20,  // numero de items producidos o consumidos
tam_vector            = 10;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
    static default_random_engine generador ( ( random_device() ) () );
    static uniform_int_distribution<int> distribucion_uniforme ( min, max ) ;
    return distribucion_uniforme ( generador );
}

bool between ( int x, int a, int b )
{
    return ( min ( a, x ) == a && max ( b, x ) == b );
}

bool esProductor ( int x )
{
    return between ( x, 0, id_productor );
}

bool esBuffer ( int x )
{
    return x == id_buffer;
}

bool esConsumidor ( int x )
{
    return between ( x, id_buffer + 1, id_consumidor );
}

// ---------------------------------------------------------------------
// produce los numeros en secuencia (1,2,3,....)

int producir ( int id )
{
    static int contador = ( num_items / num_productores ) * id;
    sleep_for ( milliseconds ( aleatorio <10, 200> () ) );
    cout << "Productor " << id << " ha producido valor " << contador << endl << flush;
    return contador++ ;
}
// ---------------------------------------------------------------------

void funcion_productor ( int id )
{
    for ( unsigned int i = 0 ; i < num_items / num_productores ; i++ )
    {
        int valor_prod = producir ( id ); //Producir valor
        cout << "Productor " << id << " va a enviar valor " << valor_prod << endl << flush;
        MPI_Ssend ( &valor_prod, 1, MPI_INT, id_buffer, 1, MPI_COMM_WORLD ); //Enviar valor
    }
}
// ---------------------------------------------------------------------

void consumir ( int valor_cons, int id )
{
    sleep_for ( milliseconds ( aleatorio <10, 200>() ) ); // Espera bloqueada
    cout << "\tConsumidor " << id << " ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------

void funcion_consumidor ( int id )
{
    int         peticion,
                valor_rec = 1 ;
    MPI_Status  estado ;

    for ( unsigned int i = 0 ; i < num_items / num_consumidores; i++ )
    {
        MPI_Ssend ( &peticion,  1, MPI_INT, id_buffer, 2, MPI_COMM_WORLD );
        MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, 0, MPI_COMM_WORLD, &estado );
        cout << "\tConsumidor " << id << " ha recibido valor " << valor_rec << endl << flush ;
        consumir ( valor_rec, id );
    }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
    int         buffer[tam_vector],
                valor,
                primera_libre = 0,
                primera_ocupada = 0,
                num_celdas_ocupadas = 0,
                tag_emisor_aceptable = 0;

    MPI_Status estado ;

    for ( unsigned int i = 0 ; i < num_items * 2 ; i++ )
    {

        if ( num_celdas_ocupadas == 0 )                tag_emisor_aceptable = 1;
        else if ( num_celdas_ocupadas == tam_vector )  tag_emisor_aceptable = 2;
        else                                         tag_emisor_aceptable = MPI_ANY_TAG;


        MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE,  tag_emisor_aceptable, MPI_COMM_WORLD, &estado );

        if ( esProductor ( estado.MPI_SOURCE ) )
        {

            buffer[primera_libre] = valor;
            primera_libre = ( primera_libre + 1 ) % tam_vector;
            num_celdas_ocupadas++;
            cout << "\t\tBuffer ha recibido el valor " << valor << endl;
        }

        else if ( esConsumidor ( estado.MPI_SOURCE ) )
        {

            valor = buffer[primera_ocupada];
            primera_ocupada = ( primera_ocupada + 1 ) % tam_vector;
            num_celdas_ocupadas--;
            cout << "\t\tBuffer envia valor " << valor << endl;
            MPI_Ssend ( &valor, 1, MPI_INT, estado.MPI_SOURCE, 0, MPI_COMM_WORLD );
        }

    }
}

// ---------------------------------------------------------------------

int main ( int argc, char *argv[] )
{
    int id_propio, num_procesos_actual; // ident. propio, núm. de procesos

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size ( MPI_COMM_WORLD, &num_procesos_actual );

    if ( num_procesos_esperado == num_procesos_actual )
    {
        if ( between ( id_propio, 0, id_productor ) )     funcion_productor ( id_propio );
        else if ( id_propio == id_buffer )              funcion_buffer();
        else                                          funcion_consumidor ( id_propio );
    }
    else if ( id_propio == 0 )  // si hay error, el proceso 0 informa
        cerr << "error: número de procesos distinto del esperado." << endl ;

    MPI_Finalize( );
    return 0;
}
// ---------------------------------------------------------------------
