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
    int jobId;
    char *jobFileName;
    JOB_STATUS jobStatus;
    FILE_TYPE *jobFileType;
    int eligiblePrinters;
} JOB;

/* Array that stores Printers */
PRINTER printer_array[MAX_PRINTERS];

/* Stores the number of printers in printer_array*/
extern int numPrinters;

/* Array that stores Jobs */
JOB job_array[MAX_JOBS];

/* Stores the number of jobs in jobs_array*/
extern int numJobs;
