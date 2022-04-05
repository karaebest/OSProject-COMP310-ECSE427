void mem_reset_variable();
void mem_init();
int mem_frame_load_next(FILE *p, int o, int n, int counter);
int mem_frame_find_lru();
char *mem_frame_get_value(char *var, int ind, int counter);
char *mem_variable_get_value(char *var, int ind);
int mem_frame_set_value(char *var, char *value, int ind);
int mem_variable_set_value(char *var, char *value, int ind);
//int mem_check_for_script(char* var);
