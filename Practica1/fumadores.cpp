#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include <vector>

using namespace std ;
using namespace SEM ;

const int n_fumadores = 5;
const int n_estanqueros = 3;
vector<Semaphore> fum;
Semaphore estanco ( 0 );

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

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero ( int index )
{
    do
    {

        int ingrediente = aleatorio < 0, n_fumadores - 1 > ();
        cout << "Estanquero " << index << ": generado ingrediente " << ingrediente << endl;
        sem_signal ( fum[ingrediente] );
        sem_wait ( estanco );

    }
    while ( true );
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar ( int num_fumador )
{

    // calcular milisegundos aleatorios de duración de la acción de fumar)
    chrono::milliseconds duracion_fumar ( aleatorio<20, 200>() );

    // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
         << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

    // espera bloqueada un tiempo igual a 'duracion_fumar' milisegundos
    this_thread::sleep_for ( duracion_fumar );

    // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador ( int num_fumador )
{
    while ( true )
    {
        sem_wait ( fum[num_fumador] );
        sem_signal ( estanco );
        fumar ( num_fumador );
    }
}

//----------------------------------------------------------------------

int main ( int argc, char** argv )
{

    for ( int i = 0; i < n_fumadores; i++ ) fum.push_back ( Semaphore ( 0 ) );

    thread fumadores[n_fumadores];
    thread estanquero[n_estanqueros];

    for ( int i = 0; i < n_estanqueros; i++ ) estanquero[i] = thread ( funcion_hebra_estanquero, i );

    for ( int i = 0 ; i < n_fumadores; i++ ) fumadores[i] = thread ( funcion_hebra_fumador, i );

    for ( int i = 0 ; i < n_fumadores; i++ ) fumadores[i].join();

    for ( int i = 0; i < n_estanqueros; i++ ) estanquero[i].join();

    return 0;
}
