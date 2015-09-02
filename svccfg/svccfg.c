#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <getopt.h>
#include "s16db.h"
#include "svccfg.h"

CLIENT * clnt;

typedef enum
{
    IMPORT,
} svccfg_mode;

void eerror (const char * str)
{
    fprintf (stderr, "svccfg: error: %s", str);
    exit (1);
}

int main (int argc, char * argv[])
{
    int c;
    extern char * optarg;
    extern int optind;
    svccfg_mode mode;
    if (argc < 2)
    {
        fprintf (stderr, "svccfg: expected option\n");
        exit (1);
    }

    if (!strcasecmp (argv[1], "import"))
        mode = IMPORT;
    else
        goto mode_unknown;

    clnt = s16db_context_create ();
    optind++;

    switch (mode)
    {
    case IMPORT:
    {
        int is_systemd = 0;
        const char * manifest = 0;
        svc_t * newSvc;

        while ((c = getopt (argc, argv, "m:s")) != -1)
        {
            switch (c)
            {
            case 'm':
                manifest = optarg;
                break;
            case 's':
                is_systemd = 1;
                break;

            case '?':
                exit (1);
                break;
            }
        }

        if (!manifest)
            eerror ("import manifest mode, but no manifest specified\n");

        newSvc = parse_unit (is_systemd, manifest);
        if (!newSvc)
            exit (1);

        int ret = s16db_svc_install (clnt, newSvc);
        printf ("installed manifest for service <%s> %d\n", newSvc->name, ret);

        // return 0;

        break;
    }
    }

    svc_list box = s16db_svc_retrieve_all (clnt);
    for (svc_list_iterator it = svc_list_begin (box); it != NULL;
         svc_list_iterator_next (&it))
    {
        printf ("ID: %d Service: %s\n", it->val->id, it->val->name);
    }

    s16db_context_destroy (clnt);
    return 0;

mode_unknown:
    fprintf (stderr, "svccfg: unknown operation: %s\n", argv[1]);
}
