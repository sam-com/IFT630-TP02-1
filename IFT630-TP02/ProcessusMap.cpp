#include "ProcessusMap.h"

// struct Rat
ProcessusMap::Rat::Rat(int processus, Case c) : processus{ processus }, c{ c }, etat{ Etat::EN_COURS }
{ }

// struct ProcessusMap

ProcessusMap::ProcessusMap(MpiContext& mpi, Map map) : Processus{ mpi,map }, log{ mpi.obtenirNbProcs() }
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

    const std::string mapString = map.toString();
    std::vector<Case> caseRat = map.trouver(Map::CASE_RAT);
    std::vector<Case> caseChasseur = map.trouver(Map::CASE_CHASSEUR_RAT);

    int decalage = 1;
    for (int i = 0; i < caseRat.size(); ++i)
    {
        mpi.envoyer(std::to_string(caseRat.at(i).x) + SEPARATEUR_CASE + std::to_string(caseRat.at(i).y) + SEPARATEUR + mapString, decalage + i, TAG_COMMENCER);
    }
    decalage += caseRat.size();
    for (int i = 0; i < caseChasseur.size(); ++i)
    {
        mpi.envoyer(std::to_string(caseChasseur.at(i).x) + SEPARATEUR_CASE + std::to_string(caseChasseur.at(i).y) + SEPARATEUR + mapString, decalage + i, TAG_COMMENCER);
    }
}

void ProcessusMap::exec()
{
    log.commencer(map);
    while (!map.trouver(Map::CASE_FROMAGE).empty() && !map.trouver(Map::CASE_RAT).empty())
    {
        std::string s = mpi.recevoirToutEmetteur(TAG_REQUETE);
        std::vector<std::string> demande = split(s, SEPARATEUR);

        if (demande[1] == Processus::BOUGER_RAT) {
            Case c = lireCase(demande[2]);
            Case nC = lireCase(demande[3]);
            ResultatBouger res = map.bougerRat(c, nC);
            traiterResultatBouger(s, demande, res);
            for (Rat r : rats)
            {
                if (c.x == r.c.x && c.y == r.c.y && r.etat != ARRETER)
                {
                    r.c.x = nC.x; r.c.y = nC.y;
                    break;
                }
            }
        }
        else if (demande[1] == Processus::BOUGER_CHASSEUR)
        {
            ResultatBouger res = map.bougerChasseur(lireCase(demande[2]), lireCase(demande[3]));
            traiterResultatBouger(s, demande, res);
        }
        else if (demande[1] == Processus::MIAULEMENT)
        {
            for (Rat r : rats)
            {
                if (r.etat != ARRETER)
                {
                    mpi.envoyer(demande[0] + SEPARATEUR + Processus::MIAULEMENT + SEPARATEUR + demande[2], r.processus, TAG_REQUETE);
                }
            }
            log.ajouterMiaulement(stoi(demande[0]), lireCase(demande[2]));
        }
        log.prendrePhotoMap(map);
    }
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.envoyer(Processus::ARRETER, i, TAG_ARRETER);
    }
    for (int i = 1; i < mpi.obtenirNbProcs(); ++i)
    {
        mpi.recevoir(i, TAG_ARRETER);
    }
    log.finir(map);
    std::cout << log.toString() << std::endl;
    log.ecrireDansFichier();
}

void ProcessusMap::traiterResultatBouger(std::string s, std::vector<std::string> demande, ResultatBouger res) {
    log.ajouterRequeteMouvement(stoi(demande[0]));
    switch (res)
    {
    case ResultatBouger::RIEN:
        mpi.envoyer(demande[0] + SEPARATEUR + Processus::BOUGER + SEPARATEUR + demande[2] + SEPARATEUR + map.toString(), stoi(demande[0]), TAG_REQUETE);
        log.ajouterRequeteMouvementRejeter(stoi(demande[0]));
        break;
    case ResultatBouger::BOUGER_RAT:
    {
        int dest{};
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
        if (dest != 0) {
            mpi.envoyer(Processus::ARRETER, dest, TAG_ARRETER);
        }
        mpi.envoyer(demande[0] + SEPARATEUR + Processus::BOUGER + SEPARATEUR + demande[3] + SEPARATEUR + map.toString(), stoi(demande[0]), TAG_REQUETE);
        log.ajouterRatAttraper(stoi(demande[0]), dest, nC);
    }
    break;
    case ResultatBouger::BOUGER_SORTIE:
        mpi.envoyer(Processus::ARRETER, stoi(demande[0]), TAG_ARRETER);
        log.ajouterRatSorti(stoi(demande[0]), lireCase(demande[3]));
        break;
    case ResultatBouger::BOUGER_FROMAGE:
        mpi.envoyer(demande[0] + SEPARATEUR + Processus::BOUGER + SEPARATEUR + demande[3] + SEPARATEUR + map.toString(), stoi(demande[0]), TAG_REQUETE);
        log.ajouterMangerFromage(stoi(demande[0]), lireCase(demande[3]));
        break;
    case ResultatBouger::BOUGER_CASE_VIDE:
        mpi.envoyer(demande[0] + SEPARATEUR + Processus::BOUGER + SEPARATEUR + demande[3] + SEPARATEUR + map.toString(), stoi(demande[0]), TAG_REQUETE);
        break;
    }
}
