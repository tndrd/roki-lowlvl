#ifndef SERIAL_BAUDRATE_ENV
#error SERIAL_BAUDRATE_ENV is not defined
#endif

#ifndef SERIAL_STOPBITS_ENV
#error SERIAL_STOPBITS_ENV is not defined
#endif

#ifndef SERIAL_PARITY_ENV
#error SERIAL_PARITY_ENV is not defined
#endif

#ifndef SERIAL_TIMEOUT_ENV
#error SERIAL_TIMOUT_ENV is not defined
#endif

#ifndef PYTHON_MODULE_NAME
#error PYTHON_MODULE_NAME is not defined
#endif

#ifndef DIAG_SCRIPTS_DIR
#error DIAG_SCRIPTS_DIR is not defined
#endif

#ifndef DIAG_SUITES_DIR
#error DIAG_SCRIPTS_DIR is not defined
#endif

#define str(a) #a
#define xstr(a) str(a)

#pragma message "SERIAL_BAUDRATE_ENV = " xstr(SERIAL_BAUDRATE_ENV)
#pragma message "SERIAL_STOPBITS_ENV = " xstr(SERIAL_STOPBITS_ENV)
#pragma message "SERIAL_PARITY_ENV = " xstr(SERIAL_PARITY_ENV)
#pragma message "SERIAL_TIMEOUT_ENV = " xstr(SERIAL_TIMEOUT_ENV)
#pragma message "PYTHON_MODULE_NAME = " xstr(PYTHON_MODULE_NAME)
#pragma message "DIAG_SCRIPTS_DIR = " xstr(DIAG_SCRIPTS_DIR)