/*
** arg.h header by Christopher Lohmann, licensed under MIT/X11
** modifications by Lukáš Hozda -> macros for flags and longer
** options, parameters, option function and other tweaks
*/

#ifndef ARG_H__
#define ARG_H__

static const char *argv0;

/* use main(int argc, char *argv[]) */
#define ARGBEGIN    for (argv0 = *argv, argv++, argc--;\
                    argv[0] && option(opts, argv, &argc, argv0);\
                    argc--, argv++) {\
                if(argv[0][1] == '-') continue;\
                if(!(argv[0][0] == '-' && argv[0][1])) continue; \
                char argc_;\
                char **argv_;\
                int brk_;\
                if (argv[0][1] == '-' && argv[0][2] == '\0') {\
                    argv++;\
                    argc--;\
                    break;\
                }\
                for (brk_ = 0, argv[0]++, argv_ = argv;\
                        argv[0][0] && !brk_;\
                        argv[0]++) {\
                    if (argv_ != argv)\
                        break;\
                    argc_ = argv[0][0];\
                    switch (argc_)

#define ARGEND            }\
            }

#define ARGC()        argc_

#define EARGF(x)    ((argv[0][1] == '\0' && argv[1] == NULL)?\
                ((x), abort(), (char *)0) :\
                (brk_ = 1, (argv[0][1] != '\0')?\
                    (&argv[0][1]) :\
                    (argc--, argv++, argv[0])))

#define ARGF()        ((argv[0][1] == '\0' && argv[1] == NULL)?\
                (char *)0 :\
                (brk_ = 1, (argv[0][1] != '\0')?\
                    (&argv[0][1]) :\
                    (argc--, argv++, argv[0])))
#define FLAG(c, action) case c: action; break;
#define ARGFLAG(c, action) case c: if(argv[1]) {action;} else fprintf(stderr, "error: -%c needs an argument\n", c);
#define START_OPTION(name, action) if(strcmp(argv[0], name) == 0) { action; }
#define OPTION(name, action) else if(strcmp(argv[0], name) == 0) { action; }
#define PARAMETER(name, action) else if(strcmp(argv[0], name) == 0 && argv[1]) { action; }

#endif
