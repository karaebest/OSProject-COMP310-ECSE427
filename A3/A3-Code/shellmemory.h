void mem_reset_variable();
void mem_init();
int mem_frame_load_next(FILE *p, int o, int n);
char *mem_frame_get_value(char *var, int ind);
char *mem_variable_get_value(char *var, int ind);
int mem_frame_set_value(char *var, char *value, int ind);
int mem_variable_set_value(char *var, char *value, int ind);
void mem_frame_delete(int index);
char* mem_frame_get_line(int index);
//int mem_check_for_script(char* var);
