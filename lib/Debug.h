#include <string>
#include "Outils.h"

#define VName(vr) #vr "=" + vr + "; "
#define VSName(vr) #vr "=" + StringF("\"%s\"", vr) + "; "
#define VIName(vr) #vr "=" + StringF("%d", vr) + "; "
#define VBName(vr) #vr "=" + StringF("%s", (vr) ? "true" : "false") + "; "
#define VPName(vr) #vr "=" + StringF("%p", vr) + "; "

class TECExcept
{
public:
	const char* Message;
	const char* Vars;
#define ECExcept(vars, msg)                             \
                TECExcept(__func__, __FILE__, __LINE__, (vars), (msg))
	TECExcept(const char* func, const char* file, int line, std::string vars, std::string msg);
	TECExcept(std::string msg);
};
