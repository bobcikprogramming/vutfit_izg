/******************************************************************************
 * Laborator 01 - Zaklady pocitacove grafiky - IZG
 * ihulik@fit.vutbr.cz
 *
 * $Id: xbobci03$
 * 
 * Popis: Hlavicky funkci pro funkce studentu
 *
 * Opravy a modifikace:
 * - ibobak@fit.vutbr.cz, orderedDithering
 */

#include "student.h"
#include "globals.h"

#include <time.h>

const int M[] = {
    0, 204, 51, 255,
    68, 136, 187, 119,
    34, 238, 17, 221,
    170, 102, 153, 85
};

const int M_SIDE = 4;

/******************************************************************************
 ******************************************************************************
 Funkce vraci pixel z pozice x, y. Je nutne hlidat frame_bufferu, pokud 
 je dana souradnice mimo hranice, funkce vraci barvu (0, 0, 0).
 Ukol za 0.25 bodu */
S_RGBA getPixel(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height) {
		return COLOR_BLACK; //vraci barvu (0, 0, 0)
		// Kontrola, �e nejsme mimo rozmez� obr�zku
	}
	return *(frame_buffer + x + width * y);
	/** 
	*	P�istupujeme na adresu pole frame_bufferu a d�le se posunujem pomoc� index� (offset�) tak,
	*	�e se posuneme o odpov�daj�c� ���ku (neboli pozici x) a pot� v��ky (y), abychom z�skali y,
	*	tak mus�me proj�t celkov� po�et x (celkovou ���ku) tolikr�t, na kter� y chceme j�t (neboli y*���ka).
	**/
}
/******************************************************************************
 ******************************************************************************
 Funkce vlozi pixel na pozici x, y. Je nutne hlidat frame_bufferu, pokud 
 je dana souradnice mimo hranice, funkce neprovadi zadnou zmenu.
 Ukol za 0.25 bodu */
void putPixel(int x, int y, S_RGBA color)
{
	if (x >= 0 && y >= 0 && x < width && y < height) {
		*(frame_buffer + x + width * y) = color;
		// Kontrola, �e jsme v rozmez� obr�zku a p�id�n� barvy na danou pozici (v poli), pomoc� stejn�ho offsetu.
	}
}
/******************************************************************************
 ******************************************************************************
 Funkce prevadi obrazek na odstiny sedi. Vyuziva funkce GetPixel a PutPixel.
 Ukol za 0.5 bodu */
void grayScale()
{	
	unsigned char red, green, blue, gray;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			red = getPixel(x, y).red * 0.299;
			green = getPixel(x, y).green * 0.587;
			blue = getPixel(x, y).blue * 0.114;
			gray = ROUND(red + green + blue);
			putPixel(x, y, makeColor(gray, gray, gray));
		}
	}
    // todo
}

/******************************************************************************
 ******************************************************************************
 Funkce prevadi obrazek na cernobily pomoci algoritmu maticoveho rozptyleni.
 Ukol za 1 bod */

void orderedDithering()
{
	grayScale();
	int xm, ym, pixValue;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			xm = x % M_SIDE;
			ym = (y % M_SIDE)*4;
			// Pomoc� operace modulo z�sk�me pozici v matici. Pozici Y je t�eba n�sobit �ty�ma pro posun mezi ��dky.
			pixValue = getPixel(x, y).red;
			// Z�sk�me hodnoty pixelu a dle podm�nky nahrad�me �ernou/b�lou.
			if (pixValue < M[xm + ym]) {
				putPixel(x, y, COLOR_BLACK);
			} else {
				putPixel(x, y, COLOR_WHITE);
			}
		}
	}
}

/******************************************************************************
 ******************************************************************************
 Funkce prevadi obrazek na cernobily pomoci algoritmu distribuce chyby.
 Ukol za 1 bod */
void errorDistribution()
{   
	grayScale();
	// Staticky zvolen� prom�nn� hodnoty 127
	static int treshHold = 127;
	int error, tmpColor, color;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			tmpColor = getPixel(x, y).red;
			if (tmpColor > treshHold) {
				// White
				error = tmpColor - 255;
				putPixel(x, y, COLOR_WHITE);
			} else {
				// Black
				error = tmpColor - 0;
				putPixel(x, y, COLOR_BLACK);
			}

			/**
			*	Zaokrouhl�me barvu pixelu s p�i�tenou chybou a zkontrolujeme, �e jsme v r�mci rozsahu 0-255.
			*	Pokud nejsme, tak color nastav�me na 0 �i 255 (white �i black), dle toho, kam jsme p�etekli.
			*	Pot� pixel vlo��me pomoc� funkce putPixel.
			*	Opakujeme pro zbyl�, jen m�n�me sou�adnice.
			**/
			color = ROUND(getPixel(x + 1, y).red + error * 3 / 8);
			if (color < 0) {
				color = 0;
			} else if(color > 255){
				color = 255;
			}
			putPixel(x + 1, y, makeColor(color, color, color));

			color = ROUND(getPixel(x, y + 1).red + error * 3 / 8);
			if (color < 0) {
				color = 0;
			}
			else if (color > 255) {
				color = 255;
			}
			putPixel(x, y + 1, makeColor(color, color, color));


			color = ROUND(getPixel(x + 1, y + 1).red + error * 2 / 8);
			if (color < 0) {
				color = 0;
			}
			else if (color > 255) {
				color = 255;
			}
			putPixel(x + 1, y + 1, makeColor(color, color, color));


		}
	}
}

/******************************************************************************
 ******************************************************************************
 Funkce prevadi obrazek na cernobily pomoci metody prahovani.
 Demonstracni funkce */
void thresholding(int Threshold)
{
	/* Prevedeme obrazek na grayscale */
	grayScale();

	/* Projdeme vsechny pixely obrazku */
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			/* Nacteme soucasnou barvu */
			S_RGBA color = getPixel(x, y);

			/* Porovname hodnotu cervene barevne slozky s prahem.
			   Muzeme vyuzit jakoukoli slozku (R, G, B), protoze
			   obrazek je sedotonovy, takze R=G=B */
			if (color.red > Threshold)
				putPixel(x, y, COLOR_WHITE);
			else
				putPixel(x, y, COLOR_BLACK);
		}
}

/******************************************************************************
 ******************************************************************************
 Funkce prevadi obrazek na cernobily pomoci nahodneho rozptyleni. 
 Vyuziva funkce GetPixel, PutPixel a GrayScale.
 Demonstracni funkce. */
void randomDithering()
{
	/* Prevedeme obrazek na grayscale */
	grayScale();

	/* Inicializace generatoru pseudonahodnych cisel */
	srand((unsigned int)time(NULL));

	/* Projdeme vsechny pixely obrazku */
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x)
		{
			/* Nacteme soucasnou barvu */
			S_RGBA color = getPixel(x, y);
			
			/* Porovname hodnotu cervene barevne slozky s nahodnym prahem.
			   Muzeme vyuzit jakoukoli slozku (R, G, B), protoze
			   obrazek je sedotonovy, takze R=G=B */
			if (color.red > rand()%255)
			{
				putPixel(x, y, COLOR_WHITE);
			}
			else
				putPixel(x, y, COLOR_BLACK);
		}
}
/*****************************************************************************/
/*****************************************************************************/