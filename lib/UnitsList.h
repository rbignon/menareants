#ifdef VECTOR
#define U_BEGIN
#define U_DECL(x) entities.push_back(new x);
#define U_END
#else
#define U_BEGIN { 0 },
#define U_DECL(x) { CreateEntity<x> },
#define U_END { 0 }
#endif

U_BEGIN
U_DECL(ECArmy)
U_DECL(ECaserne)
U_DECL(ECharFact)
U_DECL(EChar)
U_END
