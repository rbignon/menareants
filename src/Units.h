/* src/Units.h - Header of Units.cpp
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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
 *
 * $Id$
 */

#ifndef EC_UNITS_H
#define EC_UNITS_H

#include "i18n.h"
#include "src/Map.h"
#include "lib/Units.h"
#include "Resources.h"
#include <map>

/********************************************************************************************
 *                                ECUnit                                                    *
 ********************************************************************************************/

class ECUnit : public ECEntity
{
/* Constructeur/Destructeur */
public:

	ECUnit() : visual_step(0) {}

	ECUnit(uint vs, bool m = false) : move_anim(m), visual_step(vs) {}

/* Methodes */
public:

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

/* Mathodes protégées */
protected:
	bool MoveEffect(const std::vector<ECEntity*>&);


/* Variables protégées */
private:
	bool move_anim;
	uint visual_step;
};

/********************************************************************************************
 *                                ECMissiLauncher                                           *
 ********************************************************************************************/
#define MISSILAUNCHER_VISUAL_STEP  1
#define MISSILAUNCHER_MISSILE_STEP 10
class ECMissiLauncher : public ECUnit, public ECBMissiLauncher
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMissiLauncher)
		: ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(this, MISSILAUNCHER_MISSILE_STEP)
	{}

	ENTITY_CONSTRUCTOR(ECMissiLauncher), ECUnit(MISSILAUNCHER_VISUAL_STEP), missile(this, MISSILAUNCHER_MISSILE_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::MissiLauncher_Dos());
		PutImage(I_Down, Resources::MissiLauncher_Face());
		PutImage(I_Right, Resources::MissiLauncher_Right());
		PutImage(I_Left, Resources::MissiLauncher_Left());
		PutImage(I_Attaq, Resources::MissiLauncher_Right());
		PutImage(I_Deployed, Resources::MissiLauncher_Deployed());
		PutImage(I_Reployed, Resources::MissiLauncher_Reployed());
		missile.SetMissileUp(Resources::MissiLauncher_Missile_Up());
		missile.SetMissileDown(Resources::MissiLauncher_Missile_Down());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Missile launcher"); }
	virtual const char* Qual() const { return _("missile launcher"); }
	virtual const char* Infos() const
		{ return _("Missile launcher vehicle whith 8 square range once deployed."); }
	virtual const char* Description() const
	{
		return _("The missile launcher unit has very few defences but is very efficient with ranged fire.\n"
		       "It is very efficient against buildings, average against vehicles and few agains infantry.");
	}
	virtual ECImage* Icon() const { return Resources::MissiLauncher_Icon(); }
	virtual bool WantAttaq(uint, uint, bool) { return Deployed(); }

	//virtual bool CanBeCreated(ECBPlayer* pl) const { return false; }

/* Methodes */
public:
	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

/* Variables priv�s */
private:
	ECMissile missile;
};

/********************************************************************************************
 *                                ECBoat                                                    *
 ********************************************************************************************/
#define BOAT_VISUAL_STEP  3
class ECBoat : public ECUnit, public ECBBoat
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBoat) {}

	ENTITY_CONSTRUCTOR(ECBoat), ECUnit(BOAT_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Boat_Dos());
		PutImage(I_Down, Resources::Boat_Face());
		PutImage(I_Right, Resources::Boat_Right());
		PutImage(I_Left, Resources::Boat_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Ship"); }
	virtual const char* Qual() const { return _("ship"); }
	virtual const char* Infos() const { return _("Carrier boat"); }
	virtual const char* Description() const
	{
		return _("This carrier ship can transport infantry.");
	}
	virtual ECImage* Icon() const { return Resources::Boat_Icon(); }

	virtual std::string SpecialInfo();
};

/********************************************************************************************
 *                                ECPlane                                                   *
 ********************************************************************************************/
#define PLANE_VISUAL_STEP 4
class ECPlane : public ECUnit, public ECBPlane
{
/* Constructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECPlane) {}

	ENTITY_CONSTRUCTOR(ECPlane), ECUnit(PLANE_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Plane_Dos());
		PutImage(I_Down, Resources::Plane_Face());
		PutImage(I_Right, Resources::Plane_Right());
		PutImage(I_Left, Resources::Plane_Left());
		PutImage(I_Attaq, Resources::Brouillard());
		PutImage(I_Deployed, Resources::Plane_Deployed());
		PutImage(I_Reployed, Resources::Plane_Reployed());
		SetImage(GetSprite(I_Deployed));
		Image()->SetFrame(Image()->NbFrames()-1);
	}

	void Init()
	{
		ECEntity::Init();
		ECBPlane::Init();
	}

public:
	virtual const char *Name() const { return _("Freighter"); }
	virtual const char* Qual() const { return _("freighter"); }
	virtual const char *Infos() const { return _("Carrier plane (cargo plane)"); }
	virtual const char *Description() const { return _("This plane is able to carrier everything."); }
	virtual ECImage *Icon() const { return Resources::Plane_Icon(); }
	virtual std::string SpecialInfo();
	virtual std::string DeployButton() { return Deployed() ? _("Taking off") : _("Landing"); }
};

/********************************************************************************************
 *                                ECBoeing                                                  *
 ********************************************************************************************/
#define BOEING_VISUAL_STEP 3
class ECBoeing : public ECUnit, public ECBBoeing
{
/* Constructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECBoeing) : anim_state(ECBoeing::AN_NONE) {}

	ENTITY_CONSTRUCTOR(ECBoeing), ECUnit(BOEING_VISUAL_STEP), anim_state(ECBoeing::AN_NONE) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Boeing_Dos(),            GRAY2COLOR);
		PutImage(I_Down, Resources::Boeing_Face(),         GRAY2COLOR);
		PutImage(I_Right, Resources::Boeing_Right(),       GRAY2COLOR);
		PutImage(I_Left, Resources::Boeing_Left(),         GRAY2COLOR);
		PutImage(I_Attaq, Resources::Brouillard(),         GRAY2COLOR);
		PutImage(I_Deployed, Resources::Boeing_Deployed(), GRAY2COLOR);
		PutImage(I_Reployed, Resources::Boeing_Reployed(), GRAY2COLOR);
		SetImage(GetSprite(I_Deployed));
		Image()->SetFrame(Image()->NbFrames()-1);
	}

	void Init()
	{
		ECEntity::Init();
		ECBBoeing::Init();
	}

public:
	virtual const char *Name() const { return _("Boeing 767"); }
	virtual const char* Qual() const { return _("boeing 767"); }
	virtual const char *Infos() const { return _("Boeing 767 (suicide plane)"); }
	virtual const char *Description() const { return _("Throw this suicide-plane on enemies to explode."); }
	virtual ECImage *Icon() const { return Resources::Boeing_Icon(); }
	virtual std::string SpecialInfo();
	virtual std::string DeployButton() { return Deployed() ? _("Taking off") : _("Landing"); }
	bool BeforeEvent(const std::vector<ECEntity*>& entities, ECase* c, EC_Client* me);

private:
	enum
	{
		AN_NONE,
		AN_MOVING,
		AN_DOWN
	} anim_state;
};

/********************************************************************************************
 *                                EChar                                                     *
 ********************************************************************************************/
#define CHAR_VISUAL_STEP  3
class EChar : public ECUnit, public ECBChar
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(EChar) {}

	ENTITY_CONSTRUCTOR(EChar), ECUnit(CHAR_VISUAL_STEP, true) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Char_Dos());
		PutImage(I_Down, Resources::Char_Face());
		PutImage(I_Right, Resources::Char_Right());
		PutImage(I_Left, Resources::Char_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Tank"); }
	virtual const char* Qual() const { return _("tank"); }
	virtual const char* Infos() const { return _("A classical tank"); }
	virtual const char* Description() const
	{
		return _("Tank is a basic vehicle which has the particularity to be faster and cheapper than infantry "
		       "for the same fire power. However it is unable to cross over bridges.");
	}
	virtual ECImage* Icon() const { return Resources::Char_Icon(); }
};

/********************************************************************************************
 *                                ECJouano                                                  *
 ********************************************************************************************/
#define JOUANO_VISUAL_STEP 2
class ECJouano : public ECUnit, public ECBJouano
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECJouano) {}

	ENTITY_CONSTRUCTOR(ECJouano), ECUnit(JOUANO_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Jouano_Face());
		PutImage(I_Down, Resources::Jouano_Face());
		PutImage(I_Right, Resources::Jouano_Face());
		PutImage(I_Left, Resources::Jouano_Face());
	}

	struct ECAnim
	{
		ECSpriteBase* anim;

		ECAnim() : anim(0) {}

		void Init();

		~ECAnim()
		{
			delete anim;
		}
	};

	static ECAnim Anim;

/* Infos */
public:

	virtual const char* Name() const { return _("Jouano"); }
	virtual const char* Qual() const { return _("Jouano"); }
	virtual const char* Infos() const { return _("Bad dressed with a beer belly, he's able to destroy an McPuke installed on your barrack and to fart. It can be created only one time."); }
	virtual const char* Description() const
	{
		return _("Send your fat Jouano on one of your McPuke occupied barrack, he'll eat everything and ruin it.\n"
		         "\n"
		         "You can fart on all enemy infantry...");
	}
	virtual ECImage* Icon() const { return Resources::Jouano_Icon(); }
	virtual std::string DeployButton() { return Deployed() ? _("Fart") : _("Fart"); }
	virtual std::string SpecialInfo();

	virtual bool BeforeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool MakeEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);

	virtual bool AfterEvent(const std::vector<ECEntity*>&, ECase* c, EC_Client*);
};

/********************************************************************************************
 *                                ECMcDo                                                    *
 ********************************************************************************************/
#define MCDO_VISUAL_STEP 3
class ECMcDo : public ECUnit, public ECBMcDo
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECMcDo) : invested(ECEntity::E_NONE), ex_owner(0) {}

	ENTITY_CONSTRUCTOR(ECMcDo), ECUnit(MCDO_VISUAL_STEP), invested(ECEntity::E_NONE), ex_owner(0) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::McDo_Dos());
		PutImage(I_Down, Resources::McDo_Face());
		PutImage(I_Right, Resources::McDo_Right());
		PutImage(I_Left, Resources::McDo_Left());
		PutImage(I_Deployed, Resources::McDo_Caserne());
	}

	virtual ~ECMcDo();

/* Infos */
public:

	virtual const char* Name() const
	{
		return Deployed() ? _("Barrack + McPuke") : _("McPuke Donald");
	}
	virtual const char* Qual() const { return Deployed() ? _("barracks+McPuke") : _("McPuke Donald"); }
	virtual const char* Infos() const
	{
		return Deployed() ? "" : _("Send him to a barrack, he'll install a McPuke.");
	}
	virtual const char* Description() const
	{
		if(Deployed())
			return _("This barrack is populated by a McPuke !!!");
		else
			return _("The MkPuke Donald can take oven an enemy's barracks to build a McPuke. Then all units "
			       "created by this barrack will have their movements limit divided by two. Units with only one square per turn"
			       "will be definitely trapped because of their fat and profuse food.\n"
			       "Furthermore, the barracks owner will give money each turn to the MkPuke builder to pay all this food.\n"
			       "The only way to destroy a McPuke is to send a Jouano");
	}
	virtual ECImage* Icon() const { return Deployed() ? Resources::Caserne_Icon() : Resources::McDo_Icon(); }

	//virtual ECSpriteBase* DeadCase() const { return Deployed() ? Resources::CaseCitySODead() : 0; }

/* Methodes */
public:

	void RecvData(ECData data);
	std::string SpecialInfo();
	bool CanCreate(const ECBEntity* entity);

/* Variables priv�s */
public:
	ECEntity::e_type invested;
	ECPlayer* ex_owner;
};

/********************************************************************************************
 *                                ECTourist                                                 *
 ********************************************************************************************/
#define TOURIST_VISUAL_STEP 3
class ECTourist : public ECUnit, public ECBTourist
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECTourist) {}

	ENTITY_CONSTRUCTOR(ECTourist), ECUnit(TOURIST_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Tourist_Dos());
		PutImage(I_Down, Resources::Tourist_Face());
		PutImage(I_Right, Resources::Tourist_Right());
		PutImage(I_Left, Resources::Tourist_Left());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Japan tourist"); }
	virtual const char* Qual() const { return _("japan tourist"); }
	virtual const char* Infos() const { return _("He has a high range vision and ground stay visible after his transit."); }
	virtual const char* Description() const
	{
		return _("With his very special camera, this very fast tourist let the ground visible after his transit even if "
		       "any unit stay on this place.");
	}
	virtual ECImage* Icon() const { return Resources::Tourist_Icon(); }

/* Methodes */
public:

	virtual void ChangeCase(ECBCase* new_case);
};

/********************************************************************************************
 *                                ECEnginer                                                 *
 ********************************************************************************************/
#define ENGINER_VISUAL_STEP 2
class ECEnginer : public ECUnit, public ECBEnginer
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECEnginer) {}

	ENTITY_CONSTRUCTOR(ECEnginer), ECUnit(ENGINER_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Enginer_Dos());
		PutImage(I_Down, Resources::Enginer_Face());
		PutImage(I_Right, Resources::Enginer_Right());
		PutImage(I_Left, Resources::Enginer_Left());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Engineer"); }
	virtual const char* Qual() const { return _("engineer"); }
	virtual const char* Infos() const { return _("He is able to repair buildings or capture new ones."); }
	virtual const char* Description() const
	{
		return _("Send the engineer to an ennemy building to capture it, or to one of yours damaged to repair it");
	}
	virtual ECImage* Icon() const { return Resources::Enginer_Icon(); }
};

/********************************************************************************************
 *                                ECArmy                                                    *
 ********************************************************************************************/
#define ARMY_VISUAL_STEP 2
class ECArmy : public ECUnit, public ECBArmy
{
/* Constructeur/Destructeur */
public:

	ENTITY_EMPTY_CONSTRUCTOR(ECArmy) {}

	ENTITY_CONSTRUCTOR(ECArmy), ECUnit(ARMY_VISUAL_STEP) {}

	void UpdateImages()
	{
		PutImage(I_Up, Resources::Army_Dos());
		PutImage(I_Down, Resources::Army_Face());
		PutImage(I_Right, Resources::Army_Right());
		PutImage(I_Left, Resources::Army_Left());
		PutImage(I_Attaq, Resources::Brouillard());
	}

/* Infos */
public:

	virtual const char* Name() const { return _("Army"); }
	virtual const char* Qual() const { return _("army"); }
	virtual const char* Infos() const { return _("Basic army"); }
	virtual const char* Description() const
	{
		return _("This represent basic army buyd by 100 mens a time. Not very fast but very efficient in hand-to-hand fights "
		         "easily upgradable by 100 soldiers in a cheap way.");
	}
	virtual ECImage* Icon() const { return Resources::Army_Icon(); }
};

#endif /* EC_UNITS_H */
