// Autor:     Antonio Carrillo Ledesma.
// R.F.C.:    CALA-691229-TV5
// Direcci�n: Amsterdam 312 col. Hipodromo Condesa
// Tel�fono:  5-74-43-53

// Propiedad intelectual, todos los derechos reservados conforme a la ley, registro en tr�mite
// Revisi�n  1.1-A


void s_solo_car_punt(const char *cad, const unsigned int t_p, char *xcad);
  // Rutina que extrae los caracteres de puntuaci�n de la cadena CAD
  // si T_P es (1) Extrae todos los caracteres de la cadena
  //           (2) Extrae s�lo los primeros caracteres de la cadena
  //           (3) Extrae s�lo los ultimos caracteres de la cadena
  //           (4) Extrae todos aquellos caracteres que no sean de puntuaci�n
  //           (5) Extrae s�lo los primeros caracteres que no sean de puntuaci�n
  //           (6) Extrae s�lo los ultimos caracteres que no sean de puntuaci�n
  // Acepta los siguientes caracteres como de puntuaci�n �"#$%&`()*+'-./:;<>?@=\[]^_{}|~�
  // Nombre del archivo: S_SOLPTC.CPP



#include "cad_car.hpp"
extern "C" {
   #include <string.h>
}

void s_solo_car_punt(const char *cad, const unsigned int t_p, char *xcad)
{
   int i = 0, xi = 0, CTE_TRB = 1;
   if(t_p > 3) CTE_TRB = 0;
   // Extrae todos los caracteres de la cadena
   if(t_p == 1 || t_p == 4) {
      while(cad[i] != 0) {
	 if(s_es_car_punt(cad[i]) == CTE_TRB) {
	    xcad[xi] = cad[i];
	    xi ++;
	 }
	 i++;
      }
      xcad[xi] = 0;
   }

   // Extrae solo los carcateres al principio de la cadena
   if(t_p == 2 || t_p ==  5) {
      while(cad[i] != 0 && s_es_car_punt(cad[i]) == CTE_TRB) {
	 xcad[xi] = cad[i];
	 xi ++;
	 i++;
      }
      xcad[xi] = 0;
   }

   // Retorna solo los carcateres al final de la cadena
   if(t_p == 3 || t_p == 6) {
      i = strlen(cad) - 1;
      while(i >= 0 && s_es_car_punt(cad[i]) == CTE_TRB) {
	 xcad[xi] = cad[i];
	 xi ++;
	 i--;
      }
      xcad[xi] = 0;
      strrev(xcad);
   }

   xcad[xi] = 0;
}



