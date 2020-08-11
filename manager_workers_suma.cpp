// Showcases how to add custom POD message types.

// Manual refs: 24-27, 30-34, 75-78, 81-84 (ConfiguringActorApplications)
//              23-33 (TypeInspection)
#include <cassert>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <vector>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/numeric.hpp>

#include "caf/all.hpp"

// --(rst-type-id-block-begin)--
struct foo;

CAF_BEGIN_TYPE_ID_BLOCK(custom_types_1, first_custom_type_id)

  CAF_ADD_ATOM(custom_types_1, get_id_atom)
  CAF_ADD_ATOM(custom_types_1, get_filas_atom)
  CAF_ADD_ATOM(custom_types_1, envia_suma_atom)
  CAF_ADD_TYPE_ID(custom_types_1, (foo))

CAF_END_TYPE_ID_BLOCK(custom_types_1)
// --(rst-type-id-block-end)--

using namespace std;
using namespace caf;

// --(rst-foo-begin)--
struct foo {
  std::vector<int> a;
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, foo& x) {
  return f(meta::type_name("foo"), x.a);
}
// --(rst-foo-end)--

namespace{
  using manager_actor = typed_actor<replies_to<get_id_atom>::with<foo>, reacts_to<envia_suma_atom, int>>; //manager_actor
  using worker_actor = typed_actor<reacts_to<get_filas_atom>>; //worker_actor

  struct manager_state{
    int id;
    int suma_total;
    int contador_suma;

    void aumentaID() {
      id+=1;
    }

    void acumulaSumaTotal(int sumaParcial){
      suma_total+=sumaParcial;
    }

    void aumentaContadorSuma() {
      contador_suma+=1;
    }

    vector<int> calculaFilas(int n, int p, int id){
      int Nrow  = n / p;

      int filaInicio = id * Nrow;
      int filaFinal = 0;


      if (id < (p - 1)) {
        filaFinal = filaFinal + ((id + 1) * Nrow );
      } else {
        filaFinal = filaFinal + (n);
      }


      std::vector<int> vector;
      boost::push_back(vector, boost::irange(filaInicio, filaFinal));

      return vector;
    }
  };//manager_state


    struct worker_state{
      int id;
      vector<int> filas;
    };// worker state

  manager_actor::behavior_type type_checked_manager (manager_actor::stateful_pointer<manager_state> self) {
   return {
     [=]( get_id_atom ) {
       int id_actual = self ->state.id;
       self->state.aumentaID();
       vector<int> v=self->state.calculaFilas(10000,5,id_actual);
       return foo{v};
     },
     [=](envia_suma_atom, int suma){
       aout(self)<< "\n Soy el manager y recibí la suma parcial "<<suma<<"\n";
       self->state.acumulaSumaTotal(suma);
       self->state.aumentaContadorSuma();
       if(self->state.contador_suma==5){
         aout(self)<< "\n La suma total es "<<self->state.suma_total<<"\n";
       }
     }
   };
}// type_checked_manager

  worker_actor::behavior_type type_checked_worker (worker_actor::stateful_pointer<worker_state> self,int id, manager_actor ma){
    self->state.id=id;
    self->request(ma,10s,get_id_atom_v).then(
      [=](foo v){
        self->state.filas=v.a;
        //std::cout << "Soy el worker " << self->state.id << " y me asignaron las filas"<< '\n';
        //for (int x : self->state.filas)
        //  aout(self) << x << " ";

        int sum = boost::accumulate(self->state.filas, 0);
        self->send(ma, envia_suma_atom_v,sum);
        self->quit();
      }
    );
    return {
      [=]( get_filas_atom ) {
        for (int x : self->state.filas)
          aout(self) << x << " ";
      }
    };
  }// type_checked_worker

}


void caf_main(actor_system& system) {
  scoped_actor self{system};

  auto manager=self->spawn(type_checked_manager);

    for (size_t i = 0; i < 5; i++) {
      self->spawn(type_checked_worker,i,manager);
    }
  //auto worker_uno=self->spawn(type_checked_worker,1,manager);


}

CAF_MAIN(id_block::custom_types_1)
