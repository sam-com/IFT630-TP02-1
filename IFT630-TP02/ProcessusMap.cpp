#include "ProcessusMap.h"

// struct Rat
ProcessusMap::Rat::Rat(int processus, Case c) : processus{ processus }, c{ c }, etat{ Etat::EN_COURS }
{ }

// struct ProcessusMap

ProcessusMap::ProcessusMap(MpiContext& mpi, Map map) : Processus{ mpi,map }
{
    std::vector<Case> caseRat = map.trouver(Map::CASE_RAT);
    for (int i = 0; i < caseRat.size(); ++i)
    {
        rats.push_back(Rat{ i + 1,caseRat[i] });
    }
}

void ProcessusMap::pret()
{
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.recevoir(i, 0);
    }
    std::cout << mpi.obtenirRang() << " : commencer" << std::endl;
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.envoyer(COMMENCER, i, 1);
    }
}

void ProcessusMap::exec()
{
    std::cout << map.toString() << std::endl;
    while (!map.trouver(Map::CASE_FROMAGE).empty() && !map.trouver(Map::CASE_RAT).empty())
    {
        std::cout << mpi.obtenirRang() << " : en attente" << std::endl;
        std::string s = mpi.recevoirToutEmetteur(TAG_REQUETE);
        std::vector<std::string> demande = split(s, SEPARATEUR);
        std::cout << mpi.obtenirRang() << " : messageRecu : " << s << std::endl;
        if (demande[1] ==  BOUGER_RAT) {
            Case c = lireCase(demande[2]);
            Case nC = lireCase(demande[3]);
            ResultatBouger res = map.bougerRat(c, nC);
            traiterResultatBouger(s, demande, res);
            for (Rat r : rats)
            {
                if (c.x == r.c.x && c.y == r.c.y && r.etat != ARRETER)
                {
                    r.c = nC;
                    break;
                }
            }
        }
        else if (demande[1] == BOUGER_CHASSEUR)
        {
            ResultatBouger res = map.bougerChasseur(lireCase(demande[2]), lireCase(demande[3]));
            traiterResultatBouger(s, demande, res);
        }
        std::cout << map.toString() << std::endl;
    }
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.envoyer(Processus::ARRETER, i, TAG_ARRETER);
    }
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.recevoir(i, TAG_ARRETER);
    }
    std::cout << mpi.obtenirRang() << " : arreter" << std::endl;
    std::cout << map.toString() << std::endl;
}

void ProcessusMap::traiterResultatBouger(std::string s, std::vector<std::string> demande, ResultatBouger res) {
    switch (res)
    {
    case ResultatBouger::RIEN:
        mpi.envoyer(demande[0] + SEPARATEUR + "impossible", stoi(demande[0]), TAG_REQUETE);
        break;
    case ResultatBouger::BOUGER_RAT:
    {
        int dest;
        Case nC = lireCase(demande[3]);
        // trouver le rat qui faut arreter
        for (Rat r : rats)
        {
            if (nC.x == r.c.x && nC.y == r.c.y && r.etat != ARRETER)
            {
                dest = r.processus;
                r.etat = Etat::ARRETER;
                break;
            }
        }
        mpi.envoyer(Processus::ARRETER, dest, TAG_ARRETER);
        envoyerATous(s);
    }
        break;
    case ResultatBouger::BOUGER_SORTIE:
        mpi.envoyer(Processus::ARRETER,stoi(demande[0]),TAG_ARRETER);
        envoyerATous(s);
        break;
    case ResultatBouger::BOUGER_FROMAGE:
        envoyerATous(s);
        break;
    case ResultatBouger::BOUGER_CASE_VIDE:
        envoyerATous(s);
        break;
    }
}

void ProcessusMap::envoyerATous(const std::string& s)
{
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.envoyer(s, i, TAG_REQUETE);
    }
}