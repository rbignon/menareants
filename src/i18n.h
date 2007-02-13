#ifndef EC_I18N_H
#define EC_I18N_H

#include <libintl.h>

#define gettext_noop(x) (x)
#define _(x) gettext(x)
 #ifdef gettext_noop
  #define N_(String) gettext_noop(String)
 #else
  #define N_(String) (String)
 #endif

#endif /* EC_I18N_H */
