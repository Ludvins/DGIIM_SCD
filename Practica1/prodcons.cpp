#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include <vector>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 40,    // número de items
          tam_vec   = 10 ;   // tamaño del buffer
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
                                 cont_cons[num_items] = {0}; // contadores de verificación: consumidos
vector <int> vec;
Semaphore libres ( tam_vec );
Semaphore ocupadas ( 0 );



//**********************************************************************
// plantilla de función para generar un entero aleatorio unifohttps://askubuntu.com/questions/712564/why-cant-i-see-169-aspect-ratiosrmemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
    static default_random_engine generador ( ( random_device() ) () );
    static uniform_int_distribution<int> distribucion_uniforme ( min, max ) ;
    return distribucion_uniforme ( generador );
}

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato ( int index )
{
    static int contador = 0 ;
    this_thread::sleep_for ( chrono::milliseconds ( aleatorio<20, 100>() ) );

    cout << index <<  " producido: " << contador << endl << flush ;

    cont_prod[contador] ++ ;
    return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato ( unsigned dato, int index )
{
    assert ( dato < num_items );
    cont_cons[dato] ++ ;
    this_thread::sleep_for ( chrono::milliseconds ( aleatorio<20, 100>() ) );

    cout << "                 " << index <<  " consumido: " << dato << endl ;

}


//----------------------------------------------------------------------

void test_contadores()
{
    bool ok = true ;
    cout << "comprobando contadores ...." ;

    for ( unsigned i = 0 ; i < num_items ; i++ )
    {
        if ( cont_prod[i] != 1 )
        {
            cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
            ok = false ;
        }

        if ( cont_cons[i] != 1 )
        {
            cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
            ok = false ;
        }
    }

    if ( ok )
        cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora ( int index, int n )
{
    for ( unsigned i = 0 ; i < num_items / 2 ; i++ )
    {
        int dato = producir_dato ( index ) ;
        sem_wait ( libres );

        vec.push_back ( dato );

        sem_signal ( ocupadas );
    }
}

//----------------------------------------------------------------------


int popLifo()
{
    int ret = vec.back();
    vec.pop_back();
    return ret;
}

int popFifo()
{
    int ret = vec.front();
    vec.erase ( vec.begin() );
    return ret;
}

void funcion_hebra_consumidora ( int index, int n )
{
    for ( unsigned i = 0 ; i < num_items / n ; i++ )
    {
        sem_wait ( ocupadas );

        //int dato = popFifo();
        int dato = popLifo();

        sem_signal ( libres );
        consumir_dato ( dato, index ) ;
    }
}



//----------------------------------------------------------------------

int main ( int argc, char *argv[] )
{

    cout << "-----------------------------------------" << endl
         << "Problema de los productores-consumidores." << endl
         << "-----------------------------------------" << endl
         << flush ;

    int n_productores = 2, n_consumidores = 2;
    thread productores [n_productores];
    thread consumidores [n_consumidores];


    for ( int i = 0 ; i < n_productores; i++ ) productores[i] = thread ( funcion_hebra_productora, i, n_productores );

    for ( int i = 0; i < n_consumidores; i++ ) consumidores[i] = thread ( funcion_hebra_consumidora, i, n_consumidores );

    for ( int i = 0; i < n_productores; i++ ) productores[i].join();

    for ( int i = 0; i < n_consumidores; i++ ) consumidores[i].join();


    test_contadores();
}
