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

#include "caf/init_global_meta_objects.hpp"
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
    int no_actores;
    int dim_mat;
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

  manager_actor::behavior_type type_checked_manager (manager_actor::stateful_pointer<manager_state> self, int no_actores, int dim_mat ) {
    self->state.no_actores=no_actores;
    self->state.dim_mat=dim_mat;

   return {
     [=]( get_id_atom ) {
       int id_actual = self ->state.id;
       self->state.aumentaID();
       vector<int> v=self->state.calculaFilas(self->state.dim_mat,self->state.no_actores,id_actual);
       return foo{v};
     },
     [=](envia_suma_atom, int suma){
       aout(self)<< "\n Soy el manager y recibí la suma parcial "<<suma<<"\n";
       self->state.acumulaSumaTotal(suma);
       self->state.aumentaContadorSuma();
       if(self->state.contador_suma==self->state.no_actores){
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



int main(int argc, char** argv) {

  // Initialze the global type information before anything else.
  init_global_meta_objects<id_block::custom_types_1>();
  core::init_global_meta_objects();

  actor_system_config cfg;
  //cfg.parse(argc, argv, "caf-application.ini");
  actor_system system{cfg};

  scoped_actor self{system};

  int no_actores= atoi(argv[1]);
  int dim_mat= atoi(argv[2]);

  auto manager=self->spawn(type_checked_manager,no_actores,dim_mat);

    for (size_t i = 0; i < (size_t)no_actores; i++) {
      self->spawn(type_checked_worker,i,manager);
    }

}
