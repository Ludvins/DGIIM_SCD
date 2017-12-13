
#include "HoareMonitor.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <chrono>
#include <cassert>
#include <mutex>
#include <string>

const int n_barberos = 1; // Solo funciona con 1
const int n_sillas = 1; // Solo funciona con 1
const int n_clientes = 10;
const int n_sillas_espera = 5;
std::mutex mt; // Utilizado para entrada y salida.

// --------------------------------------------------------------------
// ------------------- Funciones auxiliares ---------------------------

template <int min, int max> int aleatorio()
{
    static std::default_random_engine generador ( ( std::random_device() ) () );
    static std::uniform_int_distribution<int> distribucion_uniforme ( min, max );
    return distribucion_uniforme ( generador );
}

void barberprint ( const char* c, int a )
{

    mt.lock();
    std::cout << "\tBarbero " << a << c << std::endl;
    mt.unlock();
}

void clientprint ( const char* c, int a )
{

    mt.lock();
    std::cout << "Cliente " << a << c << std::endl;
    mt.unlock();
}

// ---------------------------------------------------------------------
// --------------------------- Monitor ---------------------------------
// ---------------------------------------------------------------------

class BarberiaHM : public HM::HoareMonitor
{

private:

    HM::CondVar sillas_espera = newCondVar(),
                barberos = newCondVar(),
                sillas_barberia = newCondVar();


public:

    BarberiaHM() { }

    void cortarPelo ( int index )
    {

        if ( !sillas_barberia.empty() )
        {

            clientprint ( ": No hay sitio, esperare en la sala de espera.", index );

            if ( sillas_espera.get_nwt() < n_sillas_espera )  sillas_espera.wait();

            else
            {
                clientprint ( ": El aforo esta completo, ESTOY MUY ENFADADO!!!.", index );
                return;
            }

        }

        else if ( !barberos.empty() )
        {

            clientprint ( ": Despertando a barbero durmiendo.", index );
            barberos.signal();

        }

        clientprint ( ": Comienzo a cortarme el pelo.", index );
        sillas_barberia.wait();
        clientprint ( ": He terminado de cortarme el pelo.", index );

    }

    void siguienteCliente ( int index )
    {

        if ( sillas_espera.empty() )
        {

            barberprint ( ": No hay clientes. (Zzz...)", index );

            barberos.wait();

        }
        else
            sillas_espera.signal();

        barberprint ( ": Atendiendo a nuevo cliente.", index );

    }

    void finCliente ( int index )
    {

        barberprint ( ": Terminado de antender al cliente actual.", index );

        sillas_barberia.signal();

    }


};


// --------------------------------------------------------------------
// --------------------------------------------------------------------


//---------------------------------------------------------------------
//------------ Función que ejecuta la hebra del barbero ---------------

void funcion_hebra_barbero ( HM::MRef <BarberiaHM> m, int index )
{
    while ( true )
    {

        barberprint ( ": Llamando a siguiente cliente. ", index );

        m->siguienteCliente ( index );

        std::this_thread::sleep_for ( std::chrono::milliseconds ( aleatorio<20, 200>() ) ); //Cortar Pelo a Cliente

        m->finCliente ( index );

    }

}


// --------------------------------------------------------------------
// ------------ Función que ejecuta la hebra del cliente --------------

void funcion_hebra_cliente ( int index, HM::MRef <BarberiaHM> m )
{

    while ( true )
    {

        clientprint ( ": Entra en la barberia. ", index );

        m->cortarPelo ( index );

        clientprint ( ": Saliendo de la barberia. ", index );

        std::this_thread::sleep_for ( std::chrono::milliseconds ( aleatorio <20, 5000>() ) ); //EsperarFueraBarberia

    }

}

// -------------------------------------------------------------------
// -------------------------------------------------------------------

int main ()
{

    std::thread clientes[n_clientes], barbero[n_barberos];

    auto barberia = HM::Create <BarberiaHM> () ;

    for ( int i = 0; i < n_barberos; i++ ) barbero[i] = std::thread ( funcion_hebra_barbero, barberia, i );

    for ( int i = 0; i < n_clientes; i++ ) clientes[i] = std::thread ( funcion_hebra_cliente, i, barberia );

    for ( int i = 0; i < n_clientes; i++ ) clientes[i].join();

    for ( int i = 0; i < n_barberos; i++ ) barbero[i].join();


    return 0;

}
