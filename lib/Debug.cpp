#include "Debug.h"

TECExcept::TECExcept(const char* func, const char* file, int line, std::string vars, std::string buf)
{
  std::string s;
  s = file;
  s += ":";
  s += func;
  s += "():";
  s += TypToStr(line);
  s += "; " + buf;
  Message = s.c_str();
  Vars = vars.c_str();
}

TECExcept::TECExcept(std::string buf)
{
	Message = buf.c_str();
	Vars = NULL;
}
