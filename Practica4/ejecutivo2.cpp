// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
//
// Historial:
// Creado en Diciembre de 2017
// -----------------------------------------------------------------------------


#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide


// tipo para duraciones en segundos y milisegundos, en coma flotante:
typedef std::chrono::duration <float, std::ratio <1,1> >    seconds_f ;
typedef std::chrono::duration <float, std::ratio <1,1000> > milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string& nombre, std::chrono::milliseconds tcomputo )
{
  std::cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
  std::this_thread::sleep_for( tcomputo );
  std::cout << "fin." << std::endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", std::chrono::milliseconds(100) );  }
void TareaB() { Tarea( "B", std::chrono::milliseconds(150) );  }
void TareaC() { Tarea( "C", std::chrono::milliseconds(200) );  }
void TareaD() { Tarea( "D", std::chrono::milliseconds(240) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario
  const std::chrono::milliseconds Ts( 500 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
  std::chrono::time_point <std::chrono::steady_clock> ini_sec = std::chrono::steady_clock::now();

   while( true ) // ciclo principal
   {
     std::cout << std::endl
               << "---------------------------------------" << std::endl
               << "Comienza iteración del ciclo principal." << std::endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
        std::cout << std::endl << "Comienza iteración " << i << " del ciclo secundario." << std::endl ;

         switch( i )
         {
           case 1 : TareaA(); TareaB(); TareaC();           break ;
           case 2 : TareaA(); TareaB(); TareaD();           break ;
           case 3 : TareaA(); TareaB(); TareaC();           break ;
           case 4 : TareaA(); TareaB();                     break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         std::this_thread::sleep_until( ini_sec );

         std::chrono::steady_clock::duration d = std::chrono::steady_clock::now() - ini_sec ;

         std::cout << "Hay un retardo de: " << milliseconds_f(d).count() << " milliseconds. " << std::endl;
         if (milliseconds_f(d) > milliseconds_f(20) ){
           exit(1);
         }

      }
   }
}
