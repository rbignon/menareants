#include <iostream>
#include <vector>
#include <cmath>

#define ALGO_THOMAS  1
#define ALGO_PROGS   2
#define ALGO_PHH     3

#define ALGO         ALGO_PHH

struct Army
{
	Army(int _nb)
		: nb(_nb), init_nb(_nb), shooted(0), victories(0)
	{}

	uint nb;
	uint init_nb;
	uint shooted;
	
	uint victories;
	
	std::vector<uint> rests;
	
	void Init() { nb = init_nb; shooted = 0; }
	bool Attaq(Army*);
	void Release() { nb = (shooted > nb) ? 0 : (nb - shooted); }
	void Shoot(Army* a, uint nb) { a->shooted = nb; }
};

#if ALGO == ALGO_PHH
static int get_killed(int max)
{
	uint wanted;
	if(max>500)
		wanted=max/20;
	else if (max> 200)
		wanted=max/5;
	else
		wanted=max;
	float rdm=(((float)rand())/(float)RAND_MAX); //I want to get a random number between 0 and 1
	int sign=((rand()/(RAND_MAX/2))>=1) ? -1 : 1; //Randomly chose sign
	rdm=0.3*(sign*pow(10,rdm)/10.0)+1.0; //A number between 0.7 and 1.3, which is more often near 
	return rdm*wanted;
}
#endif

bool Army::Attaq(Army* enemie)
{
	uint enemies = 1;
#if (ALGO == ALGO_PROGS) || (ALGO == ALGO_PHH)
	uint attaq_nb = this->nb;
#elif ALGO == ALGO_THOMAS
	uint attaq_nb = this->nb > 1000 ? 1000 : this->nb;
#endif

#if ALGO == ALGO_PHH
	uint killed = get_killed(attaq_nb/(2+enemies));
#else
	uint killed = rand() % (attaq_nb/(2+enemies)+1); // +1 pour pas risquer d'avoir un modulo 0 (possible si nb < 2+enemies).
#endif
	if(killed < attaq_nb/(4+enemies)) killed = attaq_nb/(4+enemies);
	if(!killed)
		return false;
	Shoot(enemie, killed);
	
	return true;
}

struct Combat
{
	Army army1, army2;
	
	Combat(int nb1, int nb2)
		: army1(nb1), army2(nb2)
	{}
	
	void Exec();

} combats[] =
{
	Combat( 100,  500),
	Combat(1000,  500),
	Combat(1000,  600),
	Combat(1000,  700), 
	Combat(1000,  800),
	Combat(1000,  900),
	Combat(1000, 1000),
	Combat(1000, 1100),
	Combat(1000, 1200),
	Combat(1000, 1300),
	Combat(1000, 1400),
	Combat(1000, 1500),
	Combat(1000, 1600),
	Combat(1000, 2000),
	Combat(1000, 2500),
	Combat(1000, 3000),
	Combat(1000, 4000),
	Combat(1000, 5000),
	Combat(2000, 5000),
	Combat(3000, 5000),
	Combat(4000, 5000),
	Combat(4500, 5000),
	Combat(9000, 10000),
	Combat(15000, 10000)
};


void Combat::Exec()
{
	enum
	{
		S_ATTAQ,
		S_REMOVE,
		S_END
	} state = S_ATTAQ;
	
	army1.Init();
	army2.Init();
	//std::cout << army1.nb << " vs " << army2.nb << std::endl;
	while(1)
	{
		if(state == S_ATTAQ)
		{
			army1.Attaq(&army2);
			army2.Attaq(&army1);
			state = S_REMOVE;
			continue;
		}
		if(state == S_REMOVE)
		{
			army1.Release();
			army2.Release();
			if(!army1.nb || !army2.nb)
				state = S_END;
			else
				state = S_ATTAQ;
				
			continue;
		}
		if(state == S_END)
		{
			Army* winer = army1.nb ? &army1 : &army2;
			
			winer->victories++;
			winer->rests.push_back(winer->nb);
			//std::cout << "Winer is: " << winer->init_nb << " (rest: " << winer->nb << ")" << std::endl;
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int iterations = 1;

	if(argc > 1)
		iterations = atoi(argv[1]);
	else
	{
		std::cerr << "Usage: " << argv[0] << " nb_iterations" << std::endl;
		exit(0);
	}
	
	srand(time(NULL));
	
	for(int i = 0; i < iterations; ++i)
		for(uint j = 0; j < (sizeof combats / sizeof* combats); ++j)
			combats[j].Exec();
	
	printf("Army1 vs Army2   | Moy.1 (%%vict.) | Moy.2 (%%vict.)\n");
	for(uint j = 0; j < (sizeof combats / sizeof* combats); ++j)
	{
		Combat* combat = &combats[j];
		uint moy1 = 0, moy2 = 0;
		if(combat->army1.rests.empty() == false)
		{
			for(std::vector<uint>::iterator it = combat->army1.rests.begin(); it != combat->army1.rests.end(); ++it)
				moy1 += *it;
			moy1 = moy1/(combat->army1.rests.size());
		}
		if(combat->army2.rests.empty() == false)
		{
			for(std::vector<uint>::iterator it = combat->army2.rests.begin(); it != combat->army2.rests.end(); ++it)
				moy2 += *it;
			moy2 = moy2/(combat->army2.rests.size());
		}
		
		printf("%5d vs %5d = | %5d (%5.1f%%) | %5d (%5.1f%%)\n", combat->army1.init_nb, combat->army2.init_nb,
		                                                     moy1, float(combat->army1.rests.size())/float(iterations)*100,
		                                                     moy2, float(combat->army2.rests.size())/float(iterations)*100);
	}

	return 0;
}
