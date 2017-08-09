#include "Map.h"

const char Map::CASE_VIDE = ' ';
const char Map::CASE_MUR = '#';
const char Map::CASE_RAT = 'R';
const char Map::CASE_CHASSEUR_RAT = 'C';
const char Map::CASE_FROMAGE = 'F';

Map::Map(std::vector<char> cases, int maxX, int maxY) :
    cases{ cases }, maxX{ maxX }, maxY{ maxY }
{ }

Map  Map::creerMapViaFichier(std::string && nomFichier)
{
    std::string s{ };
    std::copy(std::istreambuf_iterator<char>(std::ifstream{ nomFichier }),
        std::istreambuf_iterator<char>{},
        std::back_inserter(s));

    return creerMap(std::move(s));
}

Map  Map::creerMap(std::string && stringDeCases)
{
    int pos = stringDeCases.find_first_of('\n');
    if (pos == -1) { throw; }

    std::stringstream ss{ stringDeCases };
    std::string tmp;
    int rangee{ };
    std::vector<char> cases{};
    while (std::getline(ss, tmp, '\n')) {
        ++rangee;
        for (char& c : tmp) {
            cases.push_back(c);
        }
    }
    return Map{ cases, pos - 1, rangee - 1 };
}

const int Map::obtenirMaxX()
{
    return maxX;
}

const int Map::obtenirMaxY()
{
    return maxY;
}

char& Map::operator()(int x, int y)
{
    if (x > maxX) { throw; }
    if (y > maxY) { throw; }
    return cases[y * (maxX + 1) + x];
}

char& Map::operator()(Case c)
{
    return operator()(c.x, c.y);
}

bool Map::peutBougerRat(Case c, Case nC) 
{
    int diffX = std::abs(c.x - nC.x);
    int diffY = std::abs(c.y - nC.y);
    if (operator()(c) == CASE_RAT 
        && (operator()(nC) == CASE_VIDE || operator()(nC) == CASE_FROMAGE)
        && ((diffX == 1 && diffY == 0)
        || (diffX == 0 && diffY == 1)
        || (diffX == 1 && diffY == 1)))
    {
        return true;
    }
    return false;
}

bool Map::peutbougerChasseur(Case c, Case nC)
{
    int diffX = std::abs(c.x - nC.x);
    int diffY = std::abs(c.y - nC.y);
    if (operator()(c) == CASE_CHASSEUR_RAT
        && (operator()(nC) == CASE_VIDE || operator()(nC) == CASE_RAT)
        && ((diffX == 1 && diffY == 0)
        || (diffX == 0 && diffY == 1)))
    {
        return true;
    }
    return false;
}

ResultatBouger Map::bougerRat(Case c, Case nC)
{
    if (peutBougerRat(c, nC)) 
    {
        char tmp = operator()(nC);
        operator()(c) = CASE_VIDE;
        if (nC.x == 0 || nC.y == 0 || nC.x == maxX || nC.x == maxY)
        {
            operator()(nC) = CASE_VIDE;
            return BOUGER_SORTIE;
        }
        else
        {
            operator()(nC) = CASE_RAT;
        }

        if (tmp == CASE_VIDE) 
        {
            return BOUGER_CASE_VIDE;
        } 
        if (tmp == CASE_FROMAGE)
        {
            return BOUGER_FROMAGE;
        }
    }
    return RIEN;
}

ResultatBouger Map::bougerChasseur(Case c, Case nC)
{
    if (peutbougerChasseur(c, nC))
    {
        char tmp = operator()(nC);
        operator()(c) = CASE_VIDE;
        operator()(nC) = CASE_CHASSEUR_RAT;
        
        if (tmp == CASE_VIDE)
        {
            return BOUGER_CASE_VIDE;
        }
        if (tmp == CASE_RAT)
        {
            return BOUGER_RAT;
        }
    }
    return RIEN;
}

std::vector<Case> Map::trouver(char charac)
{
    std::vector<Case> cases{ };
    Case c{ 0,0 };
    for (;c.x < maxX; ++c.x)
    {
        for (c.y = 0; c.y < maxY; ++c.y)
        {
            if (operator()(c) == charac) 
            {
                cases.push_back(c);
            }
        }
    }
    return cases;
}

std::string Map::toString()
{
    std::string s{ };
    for (unsigned int i = 0; i < cases.size(); ++i) {
        if (i % (maxX + 1) == 0 && i != 0) { s += '\n'; }
        s += cases[i];
    }
    return s;
}

// Struct Case 

Case::Case() : Case{ 0, 0 }
{ }

Case::Case(int x, int y) : x{ x }, y{ y }
{ }