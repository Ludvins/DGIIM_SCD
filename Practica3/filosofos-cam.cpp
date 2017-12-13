// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos-plantilla.cpp
// Implementación del problema de los filósofos.
// Plantilla para completar.
//
// Historial:
// Actualizado a C++11 en Septiembre de 2017
// -----------------------------------------------------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;


const int
num_filosofos = 5,
num_procesos  = 2 * num_filosofos + 1,
etiq_sentarse = 0,
etiq_levantarse = 1,
etiq_coger = 2,
etiq_soltar = 3,
id_cam = 0;

//***********************************************************************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//-----------------------------------------------------------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
    static default_random_engine generador ( ( random_device() ) () );
    static uniform_int_distribution<int> distribucion_uniforme ( min, max ) ;
    return distribucion_uniforme ( generador );
}

// ----------------------------------------------------------------------------------------------------------------------

void funcion_filosofos ( int id )
{
    int id_ten_der = std::max ( 1, ( id + 1 ) % num_procesos ),
        id_ten_izq = id - 1,
        peticion;

    while ( true )
    {

        cout << "[Filósofo " << id << "]: solicita sentarse." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_cam, etiq_sentarse, MPI_COMM_WORLD );

        cout << "[Filósofo " << id << "]: solicita tenedor izquierdo " << id_ten_izq << "." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD );

        cout << "[Filósofo " << id << "]: solicita tenedor derecho " << id_ten_der << "." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD );

        cout << "[Filósofo " << id << "]: comienza a comer. " << endl << flush;
        sleep_for ( milliseconds ( aleatorio <10, 100>() ) );

        cout << "[Filósofo " << id << "]: suelta tenedor izquierdo " << id_ten_izq << "." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD ) ;

        cout << "[Filósofo " << id << "]: suelta tenedor derecho " << id_ten_der << "." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD ) ;

        cout << "[Filósofo " << id << "]: solicita levantarse." << endl << flush;
        MPI_Ssend ( &peticion, 1, MPI_INT, id_cam, etiq_levantarse, MPI_COMM_WORLD );

        cout << "[Filósofo " << id << "]: comienza a pensar. " << endl << flush;
        sleep_for ( milliseconds ( aleatorio <10, 100>() ) );
    }
}
// ---------------------------------------------------------------------------------------------------------------------------

void funcion_tenedores ( int id )
{
    int valor, id_filosofo ;
    MPI_Status estado ;

    while ( true )
    {
        MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado );
        id_filosofo = estado.MPI_SOURCE;
        cout << "\t[Tenedor " << id << "]: ha sido cogido por filósofo " << id_filosofo << endl;

        MPI_Recv ( &valor, 1, MPI_INT, id_filosofo, etiq_soltar, MPI_COMM_WORLD, &estado ) ;
        cout << "\t[Tenedor " << id << "]: ha sido liberado por filósofo " << id_filosofo << endl ;
    }
}
// ---------------------------------------------------------------------------------------------------------------------------

void funcion_camarero()
{
    int valor, etiq_aceptable, fil_sentados = 0;
    MPI_Status estado;

    while ( true )
    {

        if ( fil_sentados < num_filosofos - 1 ) etiq_aceptable = MPI_ANY_TAG;
        else etiq_aceptable = etiq_levantarse;

        MPI_Recv ( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado );

        if ( estado.MPI_TAG == etiq_sentarse )
        {
            fil_sentados++;
            cout << "\t\t[Camarero]: sienta filósofo " << estado.MPI_SOURCE << " (" << fil_sentados << " sentados)." << endl << flush;
        }
        else if ( estado.MPI_TAG == etiq_levantarse )
        {
            fil_sentados--;
            cout << "\t\t[Camarero]: levanta filósofo " << estado.MPI_SOURCE << " (" << fil_sentados << " sentados)." << endl << flush;
        }
        else
        {
            cout << "\t\t[Camarero] ha recibido un mensage erróneo. " << endl << flush;
            exit ( 1 );
        }
    }
}


int main ( int argc, char** argv )
{
    int id_propio, num_procesos_actual ;

    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &id_propio );
    MPI_Comm_size ( MPI_COMM_WORLD, &num_procesos_actual );


    if ( num_procesos == num_procesos_actual )
    {
        if ( id_propio == id_cam )     funcion_camarero( );
        else if ( id_propio % 2 )        funcion_tenedores ( id_propio );
        else                           funcion_filosofos ( id_propio );
    }

    else
    {
        if ( id_propio == 0 )
        {
            cout << "El número de procesos esperados es:    " << num_procesos << endl
                 << "El número de procesos en ejecución es: " << num_procesos_actual << endl
                 << "(programa abortado)" << endl ;
        }
    }

    MPI_Finalize( );
    return 0;
}

// ----------------------------------------------------------------------------------------------------------------------------
