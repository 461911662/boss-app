#include <nuttx/compiler.h>
#include <nuttx/symtab.h>


#if defined(CONFIG_EXECFUNCS_HAVE_SYMTAB)
const struct symtab_s CONFIG_EXECFUNCS_SYMTAB_ARRAY[] = 
#elif defined(CONFIG_NSH_SYMTAB)
const struct symtab_s CONFIG_NSH_SYMTAB_ARRAYNAME[] = 
#else
const struct symtab_s dummy_symtab[] = 
#endif
{
};

#if defined(CONFIG_EXECFUNCS_HAVE_SYMTAB)
const int CONFIG_EXECFUNCS_NSYMBOLS_VAR = sizeof(CONFIG_EXECFUNCS_SYMTAB_ARRAY) / sizeof(struct symtab_s);
#elif defined(CONFIG_NSH_SYMTAB)
const int CONFIG_NSH_SYMTAB_COUNTNAME = sizeof(CONFIG_NSH_SYMTAB_ARRAYNAME) / sizeof(struct symtab_s);
#else
const int dummy_nsymtabs = sizeof(dummy_symtab) / sizeof(struct symtab_s);
#endif
