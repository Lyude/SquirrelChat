/* Provides a function to simply initialize all of the variables outside of the
 * scope of the main.c file, to simplify limiting the scope of variables that do
 * not need to be accessed out of their initial file.
 */
#include "init_vars.h"

#include "buffer_list.h"

void init_vars_outside_of_scope() {
    _init_vars_buffer_list_c();
}
