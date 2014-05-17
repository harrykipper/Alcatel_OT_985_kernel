
#include <et/com_err.h>

#define H3TEST_TEST1                             (43787520L)
#define H3TEST_TEST2                             (43787521L)
extern const struct error_table et_h3test_error_table;
extern void initialize_h3test_error_table(void);

/* For compatibility with Heimdal */
extern void initialize_h3test_error_table_r(struct et_list **list);

#define ERROR_TABLE_BASE_h3test (43787520L)

/* for compatibility with older versions... */
#define init_h3test_err_tbl initialize_h3test_error_table
#define h3test_err_base ERROR_TABLE_BASE_h3test
