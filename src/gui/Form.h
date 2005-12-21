/* src/gui/Form.h - Header of Form.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * $Id$
 */

#ifndef EC_FORM_H
#define EC_FORM_H

#include "Component.h"
#include "tools/Images.h"
#include <vector>

/*****************
 * DOCUMENTATION *
 *****************
 * Il faut déclarer une classe dérivée de TForm. Dedans, mettre les composants commes
 * POINTEURS PUBLIQUES. En outre, il est nécessaire, dans le constructeur, d'initialiser
 * les variables de la façon suivante :
 *   VarComp = AddComponent(new TComponent());
 * La fonction AddComponent ajoute le composant dans la liste et utilise sa fonction Init().
 * Et il faut également détruire les composants dans le destructeur avec delete.
 *
 * Par la suite, lors de l'utilisation de la Form créée, il faut créer l'objet.
 * Il est nécessaire d'utiliser une boucle sans fin dans laquelle on utilise à chaques fois Update().
 * Il est possible pour chaques composants de définir les fonctions à executer en cas d'actions.
 *
 * Voici un exemple de structure d'une classe dérivée de TForm :
 *
 * class TForm1 : public TForm
 * {
 * // Constructeur/Deconstructeur
 * public:
 *
 *    // A noter qu'il est toutefois préférable d'implémenter les deux fonctions
 *    // suivantes hors de la classe.
 *    TForm1()
 *    {
 *      // Ajout des composants
 *      Button1 =   AddComponent(new TButton(500,350,100,49));
 *      MyListBox = AddComponent(new TListBox(300,200,200,300));
 *      Blah =      AddComponent(new TMemo(75,325,300,200,30));
 *      // Définition des evenements.
 *      Button1->OnClick =    Button1OnClick;
 *      MyListBox->OnSelect = MyListBoxOnSelect;
 *    }
 *    ~TForm1()
 *    {
 *      delete Button1;
 *      delete MyListBox;
 *      delete Blah;
 *    }
 *
 * // Composants
 * public:
 *
 *    TButton      *Button1;
 *    TListBox     *MyListBox;
 *    TMemo        *Blah;
 *
 * // Evenements
 * public:
 *    void Button1OnClick();
 *    void MyListBoxOnSelect(uint selected);
 * };
 *
 *****************/

class TForm
{
/* Constructeur/Destructeur */
public:

	TForm();
	virtual ~TForm() {}

/* Methodes */
public:

	/* Dessine chaques composants */
	void Update();
	void Update(bool flip);
	void Update(int x, int y);
	void Update(int x, int y, bool flip);

/* Attributs */
public:

	/* Définit l'arrière plan */
	void SetBackground(ECImage *image);

/* Variables protégées */
protected:

	ECImage *background;

	/* Ajoute un composant */
	template<typename T>
	T* AddComponent(T* comp)
	{
		composants.push_back(comp);
		comp->Init();
		return comp;
	}

/* Variables privées */
private:
	std::vector<TComponent*> composants;
};

#endif /* EC_FORM_H */
