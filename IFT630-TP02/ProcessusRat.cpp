#include "ProcessusRat.h"

ProcessusRat::ProcessusRat(MpiContext& mpi, Map map, Case c) : ProcessusJoueur{ mpi,map,c }
{ }

void ProcessusRat::exec()
{
    do
    {
        mpi.envoyer(requeteBouger(BOUGER_RAT, c, Case{ c.x + 1,c.y + 1 }), 0, TAG_REQUETE);
    } while (lireMessage());
    mpi.envoyer("arrete", 0, TAG_ARRETER);
    std::cout << mpi.obtenirRang() << " : arreter" << std::endl;
}