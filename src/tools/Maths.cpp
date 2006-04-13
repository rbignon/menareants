#include <math.h>
#include "Maths.h"

// Modèle pour borner une valeur entre min et max
template <class T>
T BorneTpl (const T &valeur, const T &min, const T &max)
{
  if (valeur < min)
    return min;
  else if (max < valeur)
    return max;
  else
    return valeur;
}

//-----------------------------------------------------------------------------

int BorneInt (const int &valeur, const int &min, const int &max)
{ return BorneTpl (valeur, min, max); }

long BorneLong (const long &valeur, const long &min, const long &max)
{ return BorneTpl (valeur, min, max); }

double BorneDouble (const double &valeur, const double &min, const double &max)
{ return BorneTpl (valeur, min, max); }

