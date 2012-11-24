%x comment
%x quote
%{
 /*
 To compile, save this file as fixup.l and run this:

 lex -ofixup.c fixup.l && gcc fixup.c -lfl -o fixup 

 To run (to change "foo" to "bar" in a C program):

 fixup foo bar < input.c > output.c

 */

 char * sFrom;   /* word to search for */
 char * sTo;     /* word to replace it with */
%}

%%

"/*"            ECHO; BEGIN (comment);   /* begin multi-line comment */
<comment>"*/"   ECHO; BEGIN (INITIAL);   /* end multi-line comment */

"\""            ECHO; BEGIN (quote);     /* begin quotes */
<quote>{
         ["\n]  ECHO; BEGIN (INITIAL);   /* end quotes */
         "\\\"" ECHO;                    /* escaped quote inside quotes */
       }

"//".*       ECHO;                       /* single-line comment */

    /* identifier */

[a-zA-Z0-9_]+ { if (strcmp (yytext, sFrom) == 0)
                 printf ("%s", sTo);
                else
                  ECHO; 
              }
%%

int main ( argc, argv )
int argc;
char **argv;
  {
  char * sProgram = argv [0];

  if (argc != 3)
    {
    fprintf (stderr, "Usage: %s target_word replacement_word\n", sProgram);
    return 1;
    }

  sFrom = argv [1];
  sTo = argv [2];

  yylex();
  return 0;
  }

