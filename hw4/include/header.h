/*
 * Structure that describes a printer.
 */
typedef struct printer
{
    int printerId;
    char *printerName;
    PRINTER_STATUS printerStatus;
    FILE_TYPE *printerFileType;
} PRINTER;

/*
 * Structure that describes a job.
 */
typedef struct job
{
    FILE_TYPE jobFileType;
    JOB_STATUS jobStatus;
    char *jobFileName;
    char *jobCreatedAt;
    int jobEligible;
} JOB;

/* Array that stores Printers */
PRINTER printer_array[MAX_PRINTERS];

/* Stores the number of printers in printer_array*/
extern int numPrinters;
