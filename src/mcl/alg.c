/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "alg.h"
#include "proc.h"
#include "interpret.h"
#include "expand.h"
#include "procinit.h"
#include "shadow.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "impala/io.h"
#include "impala/stream.h"

#include "impala/version.h"
#include "impala/matrix.h"
#include "impala/ivp.h"
#include "impala/tab.h"
#include "impala/app.h"
#include "impala/compose.h"
#include "impala/iface.h"

#include "util/ting.h"
#include "util/equate.h"
#include "util/io.h"
#include "util/minmax.h"
#include "util/opt.h"
#include "util/err.h"
#include "util/types.h"
#include "util/ding.h"
#include "util/tr.h"

#define chb(a,b,c,d,e,f,g) mcxOptCheckBounds("mcl-main", a, b, c, d, e, f, g)

static const char* us = "mcl::alg";

const char* legend
=
   "\n"
   "Legend:\n"
   "  (i)   for informative options that exit after usage\n"
   "  (!)   for important options that you should be aware of\n"
   "Consult the manual page for more information\n"
;

enum
{                          ALG_OPT_OUTPUTFILE = 0
,  ALG_OPT_SHOWVERSION  =  ALG_OPT_OUTPUTFILE + 2
,  ALG_OPT_SHOWHELP
,  ALG_OPT_SHOWSETTINGS
,  ALG_OPT_SHOWCHARTER
,  ALG_OPT_SHOWRAM
,  ALG_OPT_SHOWSCHEMES
,  ALG_OPT_SHOWSKID
,  ALG_OPT_AMOIXA
                        ,  ALG_OPT_APROPOS
,  ALG_OPT_OVERLAP      =  ALG_OPT_APROPOS + 2
,  ALG_OPT_FORCE_CONNECTED
,  ALG_OPT_CHECK_CONNECTED
,  ALG_OPT_OUTPUT_LIMIT
,  ALG_OPT_APPEND_LOG
,  ALG_OPT_SHOW_LOG
,  ALG_OPT_CACHE_MX
,  ALG_OPT_ADAPTLOCAL
,  ALG_OPT_ADAPTTEST
,  ALG_OPT_ADAPTSMOOTH
,  ALG_OPT_REGULARIZED
,  ALG_OPT_SHADOW_MODE
,  ALG_OPT_SHADOW_VL
,  ALG_OPT_SHADOW_S
,  ALG_OPT_DISCARDLOOPS
,  ALG_OPT_SCALELOOPS
,  ALG_OPT_QUIET
,  ALG_OPT_ANALYZE
,  ALG_OPT_SORT
,  ALG_OPT_UNCHECKED
,  ALG_OPT_DIGITS
,  ALG_OPT_SETENV
                        ,  ALG_OPT_BINARY
,  ALG_OPT_ABC          =  ALG_OPT_BINARY + 2
,  ALG_OPT_EXPECT_ABC
,  ALG_OPT_YIELD_ABC
,  ALG_OPT_ABC_TRANSFORM
,  ALG_OPT_ABC_LOGTRANSFORM
,  ALG_OPT_ABC_NEGLOGTRANSFORM
,  ALG_OPT_TAB_EXTEND
,  ALG_OPT_TAB_RESTRICT
,  ALG_OPT_TAB_STRICT
,  ALG_OPT_TAB_USE
,  ALG_OPT_WRITE_MXIN
,  ALG_OPT_WRITE_MXTF
,  ALG_OPT_WRITE_SHADOW
,  ALG_OPT_WRITE_XP
                        ,  ALG_OPT_WRITE_TAB
,  ALG_OPT_ANNOT        =  ALG_OPT_WRITE_TAB + 2
,  ALG_OPT_AUTOAPPEND
,  ALG_OPT_OUTPUTDIR
,  ALG_OPT_AUTOBOUNCENAME
,  ALG_OPT_AUTOBOUNCEFIX
                        ,  ALG_OPT_AUTOPREFIX
,  ALG_OPT_TRANSFORM    =  ALG_OPT_AUTOPREFIX + 2
,  ALG_OPT_CEILNB
,  ALG_OPT_PREINFLATION
,  ALG_OPT_PREINFLATIONX
,  ALG_OPT_INFLATE_FIRST
}  ;


typedef struct grade
{  int   mark
;  const char* ind
;
}  grade ;

grade gradeDir[] =
{  {  10000, "implausible" }
,  { 100, "really really really good" }
,  {  98, "perfect" }
,  {  97, "marvelous" }
,  {  96, "superb" }
,  {  95, "sensational" }
,  {  93, "fabulous" }
,  {  92, "ripping" }
,  {  91, "smashing baby!" }
,  {  90, "scrumptious" }
,  {  89, "cracking" }
,  {  88, "top-notch" }
,  {  87, "excellent" }
,  {  86, "splendid" }
,  {  85, "outstanding" }
,  {  84, "really neat" }
,  {  82, "dandy" }
,  {  80, "favourable" }
,  {  79, "not bad at all" }
,  {  78, "groovy" }
,  {  77, "good" }
,  {  76, "all-right" }
,  {  74, "solid" }
,  {  72, "decent" }
,  {  70, "adequate" }
,  {  66, "fair" }
,  {  63, "fairish" }
,  {  60, "satisfactory" }
,  {  55, "acceptable" }
,  {  53, "tolerable" }
,  {  50, "mediocre" }
,  {  49, "so-so" }
,  {  48, "off colour" }
,  {  47, "shabby" }
,  {  46, "shoddy" }
,  {  44, "dodgy", }
,  {  43, "poor" }
,  {  40, "bad" }
,  {  35, "deplorable" }
,  {  34, "woeful" }
,  {  32, "rotten" }
,  {  30, "lousy" }
,  {  26, "abominable" }
,  {  24, "miserable" }
,  {  23, "dismal" }
,  {  22, "appalling" }
,  {  20, "awful" }
,  {  18, "pathetic" }
,  {  16, "terrible" }
,  {  14, "dreadful" }
,  {  12, "wretched" }
,  {  10, "horrid" }
,  {   8, "ghastly" }
,  {   6, "atrocious" }
,  {  -2, "impossible" }
}  ;


void juryCharter
(  void
)  ;

void  howMuchRam
(  long i
,  mclProcParam* mpp
)  ;

void mclWriteLog
(  FILE* fp
,  mclAlgParam* mlp
,  mclMatrix* cl
)  ;

void mclAlgPrintInfo
(  FILE* fp
,  mclAlgParam* mlp  
,  mclMatrix* cl
)  ;



/*  options with 'flags & MCX_OPT_INFO' exit after displaying info         */
/*  options with 'flags & MCX_OPT_HASARG' take an argument (who would've guessed) */


mcxOptAnchor mclAlgOptions[] =
{
   {  "-force-connected"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_FORCE_CONNECTED
   ,  "y/n"
   ,  "analyze clustering, make sure it induces cocos"
   }
,  {  "-write-limit"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_OUTPUT_LIMIT
   ,  "y/n"
   ,  "output the limit matrix"
   }
,  {  "-check-connected"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_CHECK_CONNECTED
   ,  "y/n"
   ,  "analyze clustering, see whether it induces cocos"
   }
,  {  "-analyze"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_ANALYZE
   ,  "y/n"
   ,  "append performance/characteristics measures"
   }
,  {  "-show-log"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_SHOW_LOG
   ,  "y/n"
   ,  "send log to stdout"
   }
,  {  "-cache-mx"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  ALG_OPT_CACHE_MX
   ,  "y/n"
   ,  "keep input matrix in memory"
   }
,  {  "--adapt-test"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  ALG_OPT_ADAPTTEST
   ,  NULL
   ,  "report adapt homg measure"
   }
,  {  "--adapt-smooth"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  ALG_OPT_ADAPTSMOOTH
   ,  NULL
   ,  "locally change inflation based on dispersion, smoothed"
   }
,  {  "--adapt-local"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_ADAPTLOCAL
   ,  NULL
   ,  "locally change inflation based on dispersion"
   }
,  {  "--regularized"
   ,  MCX_OPT_HIDDEN
   ,  ALG_OPT_REGULARIZED
   ,  NULL
   ,  "use 'regularized mcl' expansion step"
   }
,  {  "-discard-loops"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_DISCARDLOOPS
   ,  "y/n"
   ,  "remove loops in input graphs if any"
   }
,  {  "-c"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_SCALELOOPS
   ,  "<num>"
   ,  "increase loop-weights <num>-fold"
   }
,  {  "-shadow"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_SHADOW_MODE
   ,  "st|eh|el|vh|vl|xx"
   ,  "dilate parts of the graph"
   }
,  {  "--shadow-vl"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_SHADOW_VL
   ,  NULL
   ,  "shadow low values"
   }
,  {  "-shadow-s"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_SHADOW_S
   ,  "<power>"
   ,  "set fac = fac ** power when fac > 1"
   }
,  {  "-append-log"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_APPEND_LOG
   ,  "y/n"
   ,  "append log to clustering"
   }
,  {  "-overlap"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_OVERLAP
   ,  "<split|remove|keep>"
   ,  "what to do with overlap (default remove)"
   }
,  {  "-sort"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_SORT
   ,  "<mode>"
   ,  "order clustering by one of lex|size|revsize|none"
   }
,  {  "-q"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_QUIET
   ,  "log-spec"
   ,  "quiet level of logging"
   }
,  {  "--unchecked"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  ALG_OPT_UNCHECKED
   ,  NULL
   ,  "do not check input matrix validity (danger sign here)"
   }
,  {  "--abc"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_ABC
   ,  NULL
   ,  "both of --expect-abc and --yield-abc"
   }
,  {  "--expect-abc"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_EXPECT_ABC
   ,  NULL
   ,  "expect <label1> <label2> [value] format"
   }
,  {  "--yield-abc"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_YIELD_ABC
   ,  NULL
   ,  "write clusters as tab-separated labels"
   }
,  {  "--abc-log"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_ABC_LOGTRANSFORM
   ,  NULL
   ,  "log-transform label value"
   }
,  {  "--abc-neg-log"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_ABC_NEGLOGTRANSFORM
   ,  NULL
   ,  "minus log-transform label value"
   }
,  {  "-abc-tf"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_ABC_TRANSFORM
   ,  "tf-spec"
   ,  "transform label values"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_TRANSFORM
   ,  "tf-spec"
   ,  "transform matrix values"
   }
,  {  "-write-tab"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_WRITE_TAB
   ,  "fname"
   ,  "write tab to file fname"
   }
,  {  "-write-graph"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_WRITE_MXIN
   ,  "fname"
   ,  "write input matrix to file"
   }
,  {  "-write-graphx"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_WRITE_MXTF
   ,  "fname"
   ,  "write transformed matrix to file"
   }
,  {  "-write-expanded"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_WRITE_XP
   ,  "<fname>"
   ,  "file name to write expanded graph to"
   }
,  {  "-use-tab"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_TAB_USE
   ,  "fname"
   ,  "tab file"
   }
,  {  "-strict-tab"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_TAB_STRICT
   ,  "fname"
   ,  "tab file (universe)"
   }
,  {  "-restrict-tab"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_TAB_RESTRICT
   ,  "fname"
   ,  "tab file (world)"
   }
,  {  "-extend-tab"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_TAB_EXTEND
   ,  "fname"
   ,  "tab file (extendable)"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  ALG_OPT_BINARY
   ,  NULL
   ,  "write binary output"
   }
,  {  "-digits"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_DIGITS
   ,  "<int>"
   ,  "precision in interchange (intermediate matrices) output"
   }
,  {  "-set"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  ALG_OPT_SETENV
   ,  "key=val"
   ,  "set key to val in environment (debugging interface)"
   }
,  {  "--jury-charter"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_SHOWCHARTER
   ,  NULL
   ,  "\tMi\tDshow the meaning of the jury pruning synopsis"
   }
,  {  "-z"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_SHOWSETTINGS
   ,  NULL
   ,  "\tMi\tDshow some of the default settings (--show-settings alias)"
   }
,  {  "--version"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_SHOWVERSION
   ,  NULL
   ,  "\tMi\tDshow version"
   }
,  {  "--amoixa"
   ,  MCX_OPT_INFO | MCX_OPT_HIDDEN
   ,  ALG_OPT_AMOIXA
   ,  NULL
   ,  ">o<"
   }
,  {  "--apropos"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_APROPOS
   ,  NULL
   ,  "\tMi\tDshow summaries of all options"
   }
,  {  "-h"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_SHOWHELP
   ,  NULL
   ,  "\tMi\tDoutput description of most important options"
   }
,  {  "--show-schemes"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_SHOWSCHEMES
   ,  NULL
   ,  "\tMi\tDshow the preset -scheme options"
   }
,  {  "--show-skid"
   ,  MCX_OPT_INFO | MCX_OPT_HIDDEN
   ,  ALG_OPT_SHOWSKID
   ,  NULL
   ,  "\tMi\tDshow the preset -skid options"
   }
,  {  "-how-much-ram"
   ,  MCX_OPT_INFO | MCX_OPT_HASARG
   ,  ALG_OPT_SHOWRAM
   ,  "<int>"
   ,  "\tMi\tDshow estimated RAM usage for graphs with <int> nodes"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_OUTPUTFILE
   ,  "<fname>"
   ,  "\tM!\tDwrite output to file <fname>"
   }
,  {  "-od"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_OUTPUTDIR
   ,  "<directory>"
   ,  "\tM!\tDuse this directory for output"
   }
,  {  "-annot"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_ANNOT
   ,  "<description>"
   ,  "string describing the experiment"
   }
,  {  "-aa"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_AUTOAPPEND
   ,  "<suffix>"
   ,  "append <suffix> to mcl-created output file name"
   }
,  {  "-az"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_AUTOBOUNCENAME
   ,  NULL
   ,  "\tMi\tDshow output file name mcl would construct"
   }
,  {  "-ax"
   ,  MCX_OPT_INFO
   ,  ALG_OPT_AUTOBOUNCEFIX
   ,  NULL
   ,  "\tMi\tDshow the suffix mcl constructs from parameters"
   }
,  {  "-ap"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_AUTOPREFIX
   ,  "<prefix>"
   ,  "prepend <prefix> to mcl-created output file name"
   }
,  {  "-ceil-nb"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_CEILNB
   ,  "<int>"
   ,  "shrink all neighbourhoods to at most <int>, saving heigh weight edges"
   }
,  {  "-pi"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_PREINFLATION
   ,  "<num>"
   ,  "preprocess by applying inflation with parameter <num>"
   }
,  {  "-ph"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_PREINFLATIONX
   ,  "<num>"
   ,  "as -pi, applied before shadowing"
   }
,  {  "-if"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_INFLATE_FIRST
   ,  "<num>"
   ,  "assume expanded input, inflate with parameter <num>"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


void showSettings
(  mclAlgParam* mlp
)  ;


const char* mclHelp[]
=
{  "________ mcl verbosity modes"
,  "--show ....... print MCL iterands (small graphs only)"
,  "-v all ....... turn on all -v options"
,  ""
,  "________ on multi-processor systems"
,  "-te <i> ...... number of threads to use for expansion                    [0]"
,  ""
,  "________ mcl info options"
,  "-z ........... shows current settings (takes other arguments into account)"
,  "--show-schemes shows the resource schemes accessible via the -scheme option"
,  "--version .... shows version and license information"
,  ""
,  "________ speed/quality control"
,  "-scheme <i> .. use a preset pruning scheme (i=1,2,3,4,5,6,7)             [7]"
,  "                 --show-schemes shows the preset schemes"
,  "                 higher schemes are costlier and possibly more accurate"
,  ""
,  "________ mcl cluster controls"
,  "-I <f> ....... inflation (varying this parameter affects granularity)  [2.0]"
,  "-o <fname> ... direct cluster output to file fname (- for stdout)  [out.mcl]"
,  "                 if omitted, mcl will create a meaningful output name -"
,  "                 try it out, it's convenient. (cf. -az)."
,  ""
,  "________ input modality"
,  "--abc ........ assume row-based input with format"
,  "                 LABEL1 <tab> LABEL2 <tab> WEIGHT"
,  "                 where the trailing <tab> WEIGHT part is optional"
,  ""
,  "This is a selection of the mcl options and their defaults. Try different"
,  "-I values for finding clusterings of different granularity, (e.g. in the"
,  "range 1.2 - 4.0). See 'man mcl' and 'man mclfaq' for more information."
,  ""
,  "====|   mcl <graph.mci> -I 2.0        |==== should get you going, or"
,  "====|   mcl <input.abc> --abc -I 2.0  |==== (abc format, see above)"
,  ""
,  "Usually you should only need the -I option - for large graphs the -scheme"
,  "option as well. mcl --apropos shows a summary listing of all options."
,  ""
,  NULL
}  ;


/*
 * fixme: improve doio mish-mash.
*/

void postprocess
(  mclAlgParam* mlp
,  mclMatrix* cl
)
   {  clmPerformanceTable pftable
   ;  mcxTing* fn2 = mcxTingEmpty(NULL, 30)
   ;  mcxIO* xf2 = mcxIOnew("�", "w")
   ;  mclx*  mx = NULL
   ;  const char* me = "mcl parlour"
   ;  mcxbool doio = mlp->modes & ALG_DO_IO
   ;  mcxbool reread
      =  (  mlp->modes
         &  (  ALG_DO_CHECK_CONNECTED
            |  ALG_DO_FORCE_CONNECTED
            |  ALG_DO_ANALYZE
            |  ALG_DO_SHADOW
            )
         )

   ;  if (reread)
      {  mcxLog(MCX_LOG_MODULE, "mcl", "re-reading matrix to do all kinds of stuff")
      ;  if (STATUS_OK == mclAlgorithmStart(mlp, reread))
         mx = mlp->mx_start
;if (mlp->tab)
fprintf(stderr, "postprocess read tab with %d entries: %p\n", (int) N_TAB(mlp->tab), (void*) mlp->tab)
   ;  }

      if (mx && (mlp->modes & ALG_DO_FORCE_CONNECTED))
      {  mclMatrix* cm  =  clmComponents(mx, cl)

      ;  if (N_COLS(cl) != N_COLS(cm))
         {  mcxLog
            (  MCX_LOG_MODULE
            ,  me
            ,  "splitting yields an additional %ld clusters at a total of %ld"
            ,  (long) N_COLS(cm) - (long) N_COLS(cl)
            ,  (long) N_COLS(cm)
            )
         ;  if (doio)
            {  mcxTingPrint(fn2, "%s-%s", mlp->xfout->fn->str, "rags")
            ;  mcxIOnewName(xf2, fn2->str)
            ;  mclxaWrite(cl, xf2, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
            ;  mcxIOclose(xf2)
         ;  }
            mcxLog(MCX_LOG_MODULE, me, "proceeding with split clustering")
         ;  mclxFree(&cl)
         ;  cl = cm
      ;  }
         else
            mcxLog(MCX_LOG_MODULE, me, "clustering induces connected components")
         ,  mclxFree(&cm)
   ;  }

      mlp->cl_result = cl

   ;  mcxIOfree(&xf2)
   ;  mcxTingFree(&fn2)

      /* write clustering only now */

   ;  if (doio && MCPVB(mlp->mpp, MCPVB_CAT))
      {  mclDumpMatrix(cl, mlp->mpp, "result", "", 0, FALSE)
      ;  mcxLog(MCX_LOG_APP, "mcl", "output is in %s", mlp->mpp->dump_stem->str)
      ;  return
   ;  }
      else if (doio && mlp->stream_write_labels)
      {  mclxIOdumper dumper
      ;  if (mcxIOopen(mlp->xfout, RETURN_ON_FAIL) != STATUS_OK)
         {  mcxWarn(me, "cannot open out stream <%s>", mlp->xfout->fn->str)
         ;  mcxWarn(me, "trying to fall back to default <out.mcl>")
         ;  mcxIOnewName(mlp->xfout, "out.mcl")
         ;  mcxIOopen(mlp->xfout, EXIT_ON_FAIL)
      ;  }
         mclxIOdumpSet(&dumper, MCLX_DUMP_LINES | MCLX_DUMP_NOLEAD, NULL, NULL, NULL)
      ;  mclxIOdump
         (  cl
         ,  mlp->xfout
         ,  &dumper
         ,  NULL
         ,  mlp->tab
         ,  MCLXIO_VALUE_GETENV
         ,  RETURN_ON_FAIL
         )
      ;  mcxLog(MCX_LOG_APP, "mcl", "output is in %s", mlp->xfout->fn->str)
   ;  }
      else
      {  if (doio && mcxIOopen(mlp->xfout, RETURN_ON_FAIL) != STATUS_OK)
         {  mcxWarn(me, "cannot open out stream <%s>", mlp->xfout->fn->str)
         ;  mcxWarn(me, "trying to fall back to default <out.mcl>")
         ;  mcxIOnewName(mlp->xfout, "out.mcl")
         ;  mcxIOopen(mlp->xfout, EXIT_ON_FAIL)
      ;  }

         if (doio)
         {  fprintf(mlp->xfout->fp, "# cline: mcl %s ", mlp->fnin->str)
         ;  fputs(mlp->cline->str, mlp->xfout->fp)
         ;  fputc('\n', mlp->xfout->fp)
         ;  mclxaWrite(cl, mlp->xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)
      ;  }
      }

      if (doio && mlp->modes & ALG_DO_APPEND_LOG)
      mclWriteLog(mlp->xfout->fp, mlp, cl)

   ;  if (doio)
      mcxIOclose(mlp->xfout)

      /* clustering written, now check matrix */

   ;  if (reread && !mx)
      mcxErr(me, "cannot re-read matrix")
   ;  else if (mlp->modes & ALG_DO_ANALYZE && doio)
      {  mcxTing* note = mcxTingEmpty(NULL, 60)
      ;  clmGranularityTable tbl
      ;  mcxIOrenew(mlp->xfout, NULL, "a")

      ;  if (mcxIOopen(mlp->xfout, RETURN_ON_FAIL))
         {  mcxWarn(me, "cannot append to file %s", mlp->xfout->fn->str)
         ;  return
      ;  }

         clmGranularity(cl, &tbl)
      ;  clmGranularityPrint (mlp->xfout->fp, note->str, &tbl)

      ;  clmPerformance (mx, cl, &pftable)
      ;  mcxTingPrint
         (  note
         ,  "target-name=%s\nsource-name=%s\n"
         ,  mlp->fnin->str
         ,  mlp->xfout->fn->str
         )
      ;  clmPerformancePrint (mlp->xfout->fp, note->str, &pftable)

      ;  mcxLog(MCX_LOG_APP, me, "included performance measures in cluster output")
      ;  mcxTingFree(&note)
   ;  }

      mcxLog(MCX_LOG_APP, "mcl", "%ld clusters found", (long) N_COLS(cl))

   ;  if (doio)
      mcxLog(MCX_LOG_APP, "mcl", "output is in %s", mlp->xfout->fn->str)
;  }


static void mcl_unshadow_matrix
(  mclx* mx
,  mclv* dom_cols             /* if NULL then mx is a clustering */
,  mclv* dom_rows
)
   {  if (!mx)
      return
   ;  mclxChangeDomains(mx, mclvClone(dom_cols), mclvClone(dom_rows))
   ;  if (!dom_cols)
      {  dim o, m, e
      ;  clmEnstrict(mx, &o, &m, &e, ENSTRICT_KEEP_OVERLAP)
   ;  }
;  }


static void mcl_unshadow
(  mclx* cluster
,  mclAlgParam*  mlp
)
   {  mclv* dom = mlp->shadow_cache_domain

   ;  mcxLog(MCX_LOG_MODULE, "mcl", "removing shadow loops")

   ;  mcl_unshadow_matrix(cluster, NULL, dom)
   ;  mcl_unshadow_matrix(mlp->mx_input, dom, dom)

   ;  mcl_unshadow_matrix(mlp->mx_start, dom, dom)
   ;  if (mlp->mx_start)
      mclxMakeStochastic(mlp->mx_start)
   ;  mcl_unshadow_matrix(mlp->mx_expanded, dom, dom)
   ;  if (mlp->mx_expanded)
      mclxMakeStochastic(mlp->mx_expanded)

   ;  mcl_unshadow_matrix(mlp->mx_limit, dom, dom)
   ;  mcl_unshadow_matrix(mlp->cl_result, NULL, dom)
   ;  mcl_unshadow_matrix(mlp->cl_assimilated, NULL, dom)

   ;  mcxLog(MCX_LOG_MODULE, "mcl", "done")
;  }


/* hierverder.
 * check shadowing: remap everything that was cached, remap clustering.
*/

mcxstatus mclAlgorithm
(  mclAlgParam*   mlp
)
   {  mclx *thecluster, *themx
   ;  mclProcParam*  mpp         =  mlp->mpp
   ;  const char*    me          =  "mcl"
   ;  dim o, m, e
   ;  mcxbits enstrict_modes = 0

   ;  if (mlp->overlap_mode == 's')
      enstrict_modes |= ENSTRICT_SPLIT_OVERLAP
   ;  else if (mlp->overlap_mode == 'k')
      enstrict_modes |= ENSTRICT_KEEP_OVERLAP
   ;  else if (mlp->overlap_mode == 'c')
      enstrict_modes |= ENSTRICT_CUT_OVERLAP

   ;  if (mclAlgorithmStart(mlp, FALSE))      /* reread = FALSE */
      {  mcxErr(me, "no jive")
      ;  return STATUS_FAIL
   ;  }

      if (mlp->modes & ALG_DO_SHOW_PID)
      mcxLog(MCX_LOG_APP, me, "pid %ld", (long) getpid())

                        /* Don't use &(mlp->mx_start) for &themx as
                         * mclProcess writes to its first argument
                         * This code is in a block because of the
                         * dependency.
                        */
   ;  {  themx = mlp->mx_start
      ;  thecluster
         =  mclProcess
            (  &themx
            ,  mpp
            ,  mlp->modes & ALG_CACHE_START
            ,  mlp->modes & ALG_CACHE_EXPANDED ? &(mlp->mx_expanded) : NULL
            ,  &(mlp->mx_limit)
            )
      ;  if (!(mlp->modes & ALG_CACHE_START) && !mpp->expansionVariant)
         mlp->mx_start = NULL   /* twas freed by mclProcess (fixme logic) */
   ;  }

      if (mlp->modes & ALG_DO_SHADOW)
      mcl_unshadow(thecluster, mlp)

   ;  if (mlp->expand_only)
      {  mclxFree(&thecluster)         /* lazy for now */
      ;  return STATUS_OK    /* mclProcess can not convey failure (yet) */
   ;  }


      if (mlp->modes & ALG_DO_OUTPUT_LIMIT)
      {  mcxTing* fnlm  =  mcxTingPrint
                           (NULL, "%s-%s", mlp->xfout->fn->str, "limit")
      ;  mcxIO*   xflm  =  mcxIOnew(fnlm->str, "w")
      ;  mclxWrite(mlp->mx_limit, xflm, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }

      if (mlp->mx_limit != mlp->mx_expanded)
      mclxFree(&(mlp->mx_limit))

   ;  if (mlp->modes & ALG_DO_SHOW_LOG)
      fprintf(stdout, mpp->massLog->str)

   ;  clmEnstrict
      (  thecluster
      ,  &o
      ,  &m
      ,  &e
      ,  enstrict_modes
      )

   ;  if (o>0)
      {  const char* did =    mlp->overlap_mode == 'k'
                           ?  "kept"
                           :     mlp->overlap_mode == 'c'
                              ?  "removed"
                              :  "split"
      ;  mcxWarn(me, "%s <%lu> instances of overlap", did, (ulong) o)
      ;  mlp->foundOverlap = TRUE
   ;  }

      if (m>0)
      mcxWarn(me, "added <%lu> garbage entries", (ulong) m)

   ;  if (N_COLS(thecluster) > 1)
      {  if (mlp->sort_mode == 's')
         mclxColumnsRealign(thecluster, mclvSizeCmp)
      ;  else if (mlp->sort_mode == 'S')
         mclxColumnsRealign(thecluster, mclvSizeRevCmp)
      ;  else if (mlp->sort_mode == 'l')
         mclxColumnsRealign(thecluster, mclvLexCmp)
   ;  }

     /* EO cluster enstriction
      */

      if (mlp->modes & ALG_DO_SHOW_JURY)
      {  mcxLog
         (  MCX_LOG_APP
         ,  me
         ,  "jury pruning marks (worst %d cases): <%d,%d,%d>, out of 100"
         ,  (int) mclDefaultWindowSizes[mpp->mxp->nj]
         ,  (int) mpp->marks[0]
         ,  (int) mpp->marks[1]
         ,  (int) mpp->marks[2]
         )
      ;  {  int i = 0
         ;  double f = (5*mpp->marks[0] + 2*mpp->marks[1] + mpp->marks[2]) / 8.0
         ;  if (f<0.0)
            f = 0.0
         ;  while (gradeDir[i].mark > f+0.001 && gradeDir[i].mark >= 0.0)
            i++
         ;  mcxLog
            (  MCX_LOG_APP
            ,  me
            ,  "jury pruning synopsis: <%.1f or %s> (cf -scheme, -do log)"
            ,  f
            ,  gradeDir[i].ind
            )
      ;  }
      }

      postprocess(mlp, thecluster)
   ;  return STATUS_OK
;  }


mcxbool set_bit
(  mclAlgParam*   mlp
,  const char*    opt
,  int            anch_id
,  const char*    clue
)
   {  mcxbool on = FALSE
   ;  long bit = 0
   
   ;  if (!clue || strchr("1yY", clue[0]))
      on = TRUE
   ;  else if (strchr("0nN", (unsigned char) clue[0]))
      on = FALSE
   ;  else
      {  mcxErr("mcl-lib", "option %s expects 1/0/Yes/yes/No/no value", opt)
      ;  return FALSE
   ;  }

      switch(anch_id)
      {  case  ALG_OPT_APPEND_LOG      : bit = ALG_DO_APPEND_LOG     ;  break
      ;  case  ALG_OPT_SHOW_LOG        : bit = ALG_DO_SHOW_LOG       ;  break
      ;  case  ALG_OPT_ANALYZE         : bit = ALG_DO_ANALYZE        ;  break
      ;  case  ALG_OPT_FORCE_CONNECTED : bit = ALG_DO_FORCE_CONNECTED;  break
      ;  case  ALG_OPT_CHECK_CONNECTED : bit = ALG_DO_CHECK_CONNECTED;  break
      ;  case  ALG_OPT_CACHE_MX        : bit = ALG_CACHE_START       ;  break
      ;  case  ALG_OPT_OUTPUT_LIMIT    : bit = ALG_DO_OUTPUT_LIMIT   ;  break
      ;  case  ALG_OPT_DISCARDLOOPS    : bit = ALG_DO_DISCARDLOOPS   ;  break
   ;  }

      mlp->modes |= bit
   ;  if (!on)
      mlp->modes ^= bit
   ;  return TRUE
;  }


void make_output_name
(  mclAlgParam* mlp
,  mcxTing* suf
,  const char* mkappend
,  const char* mkprefix
,  const char* dirout
)
   {  mcxTing* name = mcxTingEmpty(NULL, 40)
   ;  mclProcParam* mpp = mlp->mpp

   ;  mcxTingPrintAfter(suf, "I%.1f", (double) mpp->mainInflation)

   ;  if (mpp->initLoopLength)
         mcxTingPrintAfter(suf, "l%d", (int) mpp->initLoopLength)
      ,  mcxTingPrintAfter(suf, "i%.1f", (double) mpp->initInflation)
   ;  if (mlp->pre_inflationx >= 0.0)
      mcxTingPrintAfter(suf, "ph%.1f", (double) mlp->pre_inflationx)
   ;  if (mlp->pre_inflation >= 0.0)
      mcxTingPrintAfter(suf, "pi%.1f", (double) mlp->pre_inflation)
   ;  if (mlp->pre_maxnum)
      mcxTingPrintAfter(suf, "pn%d", (int) mlp->pre_maxnum)
   ;  if (mlp->center)
      mcxTingPrintAfter(suf, "c%.1f", (double) mlp->center)
   ;  if (mlp->modes & ALG_DO_SHADOW)
      mcxTingAppend(suf, "SH")

   ;  mcxTingTr(suf, NULL, NULL, ".", "", 0)

   ;  if (mkappend)
      mcxTingPrintAfter(suf, "%s", mkappend)

   ;  if (mkprefix)
      {  char* ph /* placeholder */
      ;  if ((ph = strchr(mkprefix, '=')))
         {  mcxTingPrint(name, "%.*s", (int) (ph-mkprefix), mkprefix)
         ;  mcxTingPrintAfter(name, "%s", mlp->fnin->str)
         ;  mcxTingPrintAfter(name, "%s", ph+1)
      ;  }
         else
         mcxTingPrint(name, "%s", mkprefix)
   ;  }
      else
      {  const char* slash = strrchr(mlp->fnin->str, '/')
      ;  if (slash)
         {  mcxTingPrint(name, mlp->fnin->str)
         ;  mcxTingSplice(name, "out.", slash - mlp->fnin->str + 1, 0, 4)
      ;  }
         else
         mcxTingPrint(name, "out.%s", mlp->fnin->str)
   ;  }

      mcxTingPrintAfter(name, ".%s", suf->str)

   ;  if (dirout)
      {  const char* slash = strrchr(name->str, '/')
      ;  dim diroutlen = strlen(dirout)
      ;  int delta = diroutlen && (uchar) dirout[diroutlen-1] == '/'
      ;  if (slash)
         {  if (!strcmp(dirout, "."))
            mcxTingDelete(name, 0, slash - name->str + 1)
         ;  else
            mcxTingSplice(name, dirout, 0, slash - name->str, diroutlen-delta)
      ;  }
         else if (strcmp(dirout, "."))
         {  if (!delta)
            mcxTingInsert(name, "/", 0)
         ;  mcxTingInsert(name, dirout, 0)
      ;  }
      }

      mcxTingWrite(mlp->xfout->fn, name->str)

   ;  if (!mpp->dump_stem->len)
      mcxTingPrint(mpp->dump_stem, "%s.%s", mlp->fnin->str, suf->str)

   ;  mcxTingFree(&name)
;  }



mcxstatus mclAlgorithmInit
(  const mcxOption*  opts
,  mcxHash*       myOpts
,  const char*    fname
,  mclAlgParam*   mlp
)
   {  mclProcParam*  mpp  =  mlp->mpp
   ;  const char* mkappend = NULL, *mkprefix = NULL
   ;  mcxTing* suf = mcxTingEmpty(NULL, 20)
   ;  const mcxOption* opt
   ;  int mkbounce = 0
   ;  float f, f_0  =  0.0
   ;  int i_1     =  1
   ;  int i_10    =  10
   ;  int i
   ;  long l
   ;  const char* dirout = NULL

   ;  if (fname)
      {  if (mlp->mx_input)
         {  mcxErr(__func__, "PBD cached matrix and file argument")
         ;  return ALG_INIT_FAIL
      ;  }
         mcxTingWrite(mlp->fnin, fname)
   ;  }
      else if (!mlp->mx_input)
      {  mcxErr(__func__, "PBD need cached matrix or file argument")
      ;  return ALG_INIT_FAIL
   ;  }

      for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = mcxOptFind(opt->anch->tag, myOpts)
      ;  mcxbool  vok   = TRUE        /* value ok */

      ;  if (!anch)     /* not in myOpts */
         continue

      ;  switch(anch->id)
         {  case ALG_OPT_EXPECT_ABC
         :  mlp->stream_modes |= MCLXIO_STREAM_ABC
         ;  break
         ;

            case ALG_OPT_ABC
         :  mlp->stream_modes |= MCLXIO_STREAM_ABC
         ;  mlp->stream_write_labels = TRUE
         ;  break
         ;

            case ALG_OPT_WRITE_XP
         :  mlp->mpp->fname_expanded =  mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_ABC_LOGTRANSFORM
         :  BIT_OFF(mlp->stream_modes, MCLXIO_STREAM_NEGLOGTRANSFORM)
         ;  BIT_ON(mlp->stream_modes, MCLXIO_STREAM_LOGTRANSFORM)
         ;  break
         ;

            case ALG_OPT_ABC_NEGLOGTRANSFORM
         :  BIT_ON(mlp->stream_modes, MCLXIO_STREAM_NEGLOGTRANSFORM)
         ;  BIT_OFF(mlp->stream_modes, MCLXIO_STREAM_LOGTRANSFORM)
         ;  break
         ;

            case ALG_OPT_ABC_TRANSFORM
         :  mlp->stream_transform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_TRANSFORM
         :  mlp->transform_spec  =  mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_WRITE_TAB
         :  mlp->fn_write_tab  =  mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_TAB_RESTRICT
         :  mcxTingWrite(mlp->fn_read_tab, opt->val)
         ;  mlp->stream_modes |= MCLXIO_STREAM_GTAB_RESTRICT
         ;  break
         ;

            case ALG_OPT_TAB_EXTEND
         :  mcxTingWrite(mlp->fn_read_tab, opt->val)
         ;  mlp->stream_modes |= MCLXIO_STREAM_GTAB_EXTEND
         ;  break
         ;

            case ALG_OPT_TAB_STRICT
         :  mcxTingWrite(mlp->fn_read_tab, opt->val)
         ;  mlp->stream_modes |= MCLXIO_STREAM_GTAB_STRICT
         ;  break
         ;

            case ALG_OPT_TAB_USE
         :  mcxTingWrite(mlp->fn_read_tab, opt->val)
         ;  mlp->stream_write_labels = TRUE
         ;  break
         ;

            case ALG_OPT_YIELD_ABC
         :  mlp->stream_write_labels = TRUE
         ;  break
         ;

            case ALG_OPT_WRITE_MXTF
         :  mlp->fn_write_start  =  mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_WRITE_MXIN
         :  mlp->fn_write_input  =  mcxTingNew(opt->val)
         ;  break
         ;

            case ALG_OPT_BINARY
         :  mclxSetBinaryIO()
         ;  break
         ;

            case ALG_OPT_UNCHECKED
         :  {  char* e = mcxStrDup("MCLXIOUNCHECKED=1")
            ;  putenv(e)
         ;  }
            break
         ;

            case ALG_OPT_SHOWVERSION
         :  app_report_version("mcl")
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_SHOWRAM
         :  l = atol(opt->val)
         ;  howMuchRam(l, mlp->mpp)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_AMOIXA
         :  mcxOptApropos
            (  stdout
            ,  "mcl-algorithm"
            ,  NULL
            ,  15
            ,  MCX_OPT_DISPLAY_HIDDEN |   MCX_OPT_DISPLAY_SKIP
            ,  mclAlgOptions
            )
         ;  mcxOptApropos
            (  stdout
            ,  "mcl-process"
            ,  NULL
            ,  15
            ,  MCX_OPT_DISPLAY_HIDDEN |   MCX_OPT_DISPLAY_SKIP
            ,  mclProcOptions
            )
         ;  fputs(legend, stdout)
;fputs("set MCL_DUMP_SHADOW to dump shadow matrix\n", stdout)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_APROPOS
         :  mcxOptApropos
            (  stdout
            ,  "mcl-algorithm"
            ,  NULL
            ,  15
            ,  MCX_OPT_DISPLAY_SKIP
            ,  mclAlgOptions
            )
         ;  mcxOptApropos
            (  stdout
            ,  "mcl-process"
            ,  NULL
            ,  15
            ,  MCX_OPT_DISPLAY_SKIP
            ,  mclProcOptions
            )
         ;  fputs(legend, stdout)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_SHOWHELP
         :  {  const char** h = mclHelp
            ;  while (*h)
               fprintf(stdout, "%s\n", *h++)
         ;  }
            return ALG_INIT_DONE
         ;

            case ALG_OPT_SHOWCHARTER
         :  juryCharter()
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_SETENV
         :  mcxSetenv(opt->val)
         ;  break
         ;

            case ALG_OPT_SHOWSETTINGS
         :  showSettings(mlp)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_OVERLAP
         :  mlp->overlap_mode
               =  !strcmp(opt->val, "cut")
                  ?  'c'
                  :     !strcmp(opt->val, "keep")
                     ?  'k'
                     :  's'      /* default */
         ;  break
         ;

            case ALG_OPT_SHOWSKID
         :  mclShowSchemes(TRUE)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_SCALELOOPS
         :  f = ABS(atof(opt->val))
         ;  mlp->center = ABS(f)
         ;  break
         ;

            case ALG_OPT_SHOWSCHEMES
         :  mclShowSchemes(FALSE)
         ;  return ALG_INIT_DONE
         ;

            case ALG_OPT_FORCE_CONNECTED
         :  case ALG_OPT_CHECK_CONNECTED
            :  case ALG_OPT_OUTPUT_LIMIT
               :  case ALG_OPT_APPEND_LOG
                  :  case ALG_OPT_SHOW_LOG
                     :  case ALG_OPT_ANALYZE
                        :  case ALG_OPT_CACHE_MX
                           :  case ALG_OPT_DISCARDLOOPS
                        :
            vok = set_bit(mlp, opt->anch->tag, anch->id, opt->val)
         ;  break
         ;

            case ALG_OPT_ADAPTTEST
         :  mcxSetenv("MCL_AUTO_TEST=1")
         ;  break
         ;

            case ALG_OPT_ADAPTSMOOTH
         :  mcxSetenv("MCL_AUTO_SMOOTH=1")
         ;  break
         ;

            case ALG_OPT_ADAPTLOCAL
         :  mcxSetenv("MCL_AUTO_LOCAL=1")
         ;  break
         ;

            case ALG_OPT_QUIET
         :  mcxLogLevelSetByString(opt->val)
         ;  break
         ;

            case ALG_OPT_ANNOT               /* its destination is cline */
         :  break
         ;

            case ALG_OPT_OUTPUTDIR
         :  dirout = opt->val
         ;  break
         ;

            case ALG_OPT_AUTOAPPEND
         :  mkappend = opt->val
         ;  break
         ;

            case ALG_OPT_AUTOBOUNCENAME
         :  mkbounce = opts[1].anch ? 2 : 1  /* check whether it's last option */
         ;  break
         ;

            case ALG_OPT_AUTOBOUNCEFIX
         :  mkbounce = 3
         ;  break
         ;

            case ALG_OPT_AUTOPREFIX
         :  mkprefix = opt->val
         ;  break
         ;

            case ALG_OPT_OUTPUTFILE
         :  mcxIOnewName(mlp->xfout, opt->val)
         ;  break
         ;

            case ALG_OPT_SORT
         :  if (!strcmp(opt->val, "lex"))
            mlp->sort_mode = 'l'
         ;  else if (!strcmp(opt->val, "size"))
            mlp->sort_mode = 's'
         ;  else if (!strcmp(opt->val, "revsize"))
            mlp->sort_mode = 'S'
         ;  else if (!strcmp(opt->val, "none"))
            mlp->sort_mode = 'n'
         ;  break
         ;

            case ALG_OPT_INFLATE_FIRST
         :  f = atof(opt->val)
         ;  if ((vok = chb(anch->tag, 'f', &f, fltGt, &f_0, NULL, NULL)))
            {  mlp->pre_inflation =  f
            ;  BIT_OFF(mlp->modes, ALG_DO_DISCARDLOOPS)
         ;  }
            break
         ;

            case ALG_OPT_PREINFLATION
         :  mlp->pre_inflation =  atof(opt->val)
         ;  break
         ;

            case ALG_OPT_PREINFLATIONX
         :  mlp->pre_inflationx =  atof(opt->val)
         ;  break
         ;

            case ALG_OPT_SHADOW_S
         :  mlp->shadow_s = atof(opt->val)
         ;  break
         ;

            case ALG_OPT_SHADOW_VL
         :  mlp->shadow_mode = 0
         ;  BIT_ON(mlp->shadow_mode, MCL_SHADOW_V_LOW)
         ;  mlp->modes |= ALG_DO_SHADOW
         ;  break
         ;

            case ALG_OPT_SHADOW_MODE
         :  mlp->shadow_mode = 0

         ;  if (strstr(opt->val, "ey"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_EARLY)
         ;  if (strstr(opt->val, "xx"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_MULTIPLY)
         ;  if (strstr(opt->val, "sf"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_SELF)

         ;  if (strstr(opt->val, "eh"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_E_HIGH)
         ;  if (strstr(opt->val, "el"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_E_LOW)
         ;  if (strstr(opt->val, "vh"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_V_HIGH)
         ;  if (strstr(opt->val, "vl"))
            BIT_ON(mlp->shadow_mode, MCL_SHADOW_V_LOW)

         ;  mlp->modes |= ALG_DO_SHADOW
         ;  break
         ;

            case ALG_OPT_DIGITS
         :  l = atol(opt->val)
         ;  i = l
         ;  if ((vok = chb(anch->tag,'i', &i, intGq, &i_1, intLq, &i_10)))
               mlp->expandDigits = i
            ,  mpp->printDigits = i
         ;  break
         ;

            case ALG_OPT_REGULARIZED
         :  mpp->expansionVariant = 1
         ;  break
         ;

            case ALG_OPT_CEILNB
         :  i = atoi(opt->val)
         ;  if ((vok = chb(anch->tag, 'i', &i, intGq, &i_1, NULL, NULL)))
            mlp->pre_maxnum = i
         ;  break
         ;

         }

         if (vok != TRUE)
         return ALG_INIT_FAIL
   ;  }

      if (!mlp->xfout->fn->len && mlp->modes & ALG_DO_IO)
      make_output_name(mlp, suf, mkappend, mkprefix, dirout)
            /* ^ in mcl mode */
   ;  else if (mlp->xfout->fn->len && !(mlp->modes & ALG_DO_IO))
      mlp->modes |= ALG_DO_IO
            /* ^ in mclcm mode */

   ;  if (!mpp->dump_stem->len)
      {  if (!strcmp(mlp->xfout->fn->str, "-"))
         mcxTingPrint(mpp->dump_stem, "%s", mlp->fnin->str)
      ;  else
         mcxTingPrint(mpp->dump_stem, "%s", mlp->xfout->fn->str)
   ;  }

              /* mkbounce == 2: -az was not the last option.
               * Print to stderr too as way of a small alert.
              */
      if (mkbounce)
      {  if (mkbounce == 2)
         fprintf(stderr, "%s\n", mlp->xfout->fn->str)
      ;  if (mkbounce == 3)
         fprintf
         (  stdout
         ,  "%s%s", suf->str
         ,  isatty(fileno(stdout)) ? "\n" : ""
         )
      ;  else
         fprintf
         (  stdout
         ,  "%s%s", mlp->xfout->fn->str
         ,  isatty(fileno(stdout)) ? "\n" : ""
         )
      ;  return ALG_INIT_DONE
   ;  }

                        /* truncate (cat operates in append mode */
      if (MCPVB(mpp, MCPVB_CAT))
      {  mcxIO* tmp = NULL
      ;  mcxTingWrite(mpp->dump_stem, mlp->xfout->fn->str)
      ;  tmp = mcxIOnew(mpp->dump_stem->str, "w")
      ;  mcxIOtestOpen(tmp, RETURN_ON_FAIL)
      ;  mcxIOfree(&tmp)
   ;  }

      if (mlp->fn_read_tab->len)
      {  mcxIO* xftmp = mcxIOnew(mlp->fn_read_tab->str, "r")
      ;  mlp->tab = mclTabRead(xftmp, NULL, EXIT_ON_FAIL)
      ;  mlp->mpp->dump_tab = mlp->tab
      ;  mcxIOfree(&xftmp)
   ;  }

      mcxTingFree(&suf)
   ;  return ALG_INIT_OK
;  }


void showSettings
(  mclAlgParam* mlp
)
   {  mclShowSettings(stdout, mlp->mpp, TRUE)
;  }


static mclAlgParam* mclAlgParamNew
(  mclProcParam*  mpp
,  mclx*          mx_input
)  
   {  mclAlgParam* mlp     =     mcxAlloc(sizeof(mclAlgParam), EXIT_ON_FAIL)
   ;  if (!mpp)
      mpp =  mclProcParamNew()

   ;  mlp->mpp             =     mpp

   ;  mlp->xfout           =     mcxIOnew("", "w")

   ;  mlp->expandDigits    =     8

   ;  mlp->pre_inflation   =    -1.0
   ;  mlp->pre_inflationx  =    -1.0
   ;  mlp->pre_maxnum      =     0

   ;  mlp->modes           =     ALG_DO_SHOW_PID | ALG_DO_SHOW_JURY | ALG_DO_DISCARDLOOPS
   ;  mlp->foundOverlap    =     FALSE

   ;  mlp->stream_modes    =     0
   ;  mlp->stream_write_labels =     FALSE
   ;  mlp->fn_read_tab     =     mcxTingEmpty(NULL, 0)
   ;  mlp->fn_write_tab    =     NULL
   ;  mlp->fn_write_input  =     NULL
   ;  mlp->fn_write_start  =     NULL
   ;  mlp->tab             =     NULL

   ;  mlp->mx_input        =     mx_input
   ;  mlp->mx_start        =     NULL
   ;  mlp->mx_start_sums   =     NULL
   ;  mlp->mx_expanded     =     NULL
   ;  mlp->mx_limit        =     NULL

   ;  mlp->cl_result       =     NULL
   ;  mlp->cl_assimilated  =     NULL
   ;  mlp->n_assimilated   =     0

   ;  mlp->stream_transform_spec  =     NULL
   ;  mlp->stream_transform       =    NULL
   ;  mlp->shadow_cache_domain   =     NULL
   ;  mlp->shadow_mode     =     0
   ;  mlp->shadow_s        =     1.0
   ;  mlp->center          =     0.0
   ;  mlp->expand_only     =     FALSE

   ;  mlp->transform_spec  =     NULL
   ;  mlp->transform       =     NULL

   ;  mlp->write_mode      =     'a'
   ;  mlp->sort_mode       =     'S'
   ;  mlp->overlap_mode    =     's'
   ;  mlp->fnin            =     mcxTingEmpty(NULL, 10)
   ;  mlp->cline           =     mcxTingEmpty(NULL, 10)
   ;  return mlp
;  }


void* mclAlgParamRelease
(  mclAlgParam *mlp
,  void*       what
)
   {  void* where = NULL

   ;  if (what == mlp->mx_input)
      where = mlp->mx_input, mlp->mx_input = NULL

   ;  else if (what == mlp->mx_start)
      where = mlp->mx_start, mlp->mx_start = NULL

   ;  else if (what == mlp->mx_expanded)
      {  where = mlp->mx_expanded
      ;  if (where == mlp->mx_limit)
         mlp->mx_limit = NULL
      ;  mlp->mx_expanded = NULL
   ;  }

      else if (what == mlp->cl_result)
      where = mlp->cl_result, mlp->cl_result = NULL

   ;  else if (what == mlp->cl_assimilated)
      where = mlp->cl_assimilated, mlp->cl_assimilated = NULL

   ;  else if (what == mlp->tab)
      where = mlp->tab, mlp->tab = NULL

   ;  else
      mcxErr(us, "PBD release request for unsupported member")

   ;  return where
;  }


void mclAlgParamFree
(  mclAlgParam** app
,  mcxbool free_composites
)
   {  mclAlgParam *mlp  = *app

   ;  if (!mlp)
      return

   ;  mclProcParamFree(&(mlp->mpp))
   ;  mcxTingFree(&(mlp->cline))
   ;  mcxTingFree(&(mlp->fnin))
   ;  mcxIOfree(&(mlp->xfout))

   ;  mcxTingFree(&(mlp->fn_write_tab))
   ;  mcxTingFree(&(mlp->fn_read_tab))
   ;  mcxTingFree(&(mlp->fn_write_input))
   ;  mcxTingFree(&(mlp->fn_write_start))

   ;  mclvFree(&(mlp->shadow_cache_domain))
   ;  mclvFree(&(mlp->mx_start_sums))

   ;  if (free_composites)
      {  mclTabFree(&(mlp->tab))
      ;  mclxFree(&(mlp->mx_input))
      ;  mclxFree(&(mlp->mx_start))
      ;  mclxFree(&(mlp->mx_limit))
      ;  mclxFree(&(mlp->mx_expanded))
      ;  mclxFree(&(mlp->cl_result))
      ;  mclxFree(&(mlp->cl_assimilated))
   ;  }

      mcxFree(mlp)
   ;  *app = NULL
;  }


void mclAlgPrintInfo
(  FILE* fp
,  mclAlgParam* mlp  
,  mclMatrix* cl
)
   {  fprintf(fp, "version <%s>\n", mclDateTag)
   ;  fprintf(fp, "input file name <%s>\n", mlp->fnin->str)
   ;  if (cl)
      fprintf(fp, "number of nodes <%ld>\n", (long) N_ROWS(cl))
   ;  if (cl)
      fprintf(fp, "number of clusters <%ld>\n", (long) N_COLS(cl))
   ;  fprintf(fp, "command line <%s>\n", mlp->cline->str)
   ;  fprintf(fp, "total time usage <%.2f>\n", (double) mlp->mpp->lap)  
   ;  fprintf(fp, "number of iterations <%d>\n", (int) mlp->mpp->n_ite)  
;  }


const char* juryBabble[]
=
{  ""
,  "The  'synopsis' reported  by  mcl when  it is  done  corresponds with  a"
,  "weighted average of  the reported /jury/ marks. The  average is computed"
,  "as (5*m1+2*m2+m3)/8 and the correspondence is shown in the table above."
,  ""
,  "There are  three jury marks for  the first three rounds  of expansion. A"
,  "jury mark  is the percentage of  kept stochastic mass averaged  over the"
,  "worst X cases (c.q. matrix  columns). The number of cases considered"
,  "can be  set using -nj,  so you  have some control  over the mood  of the"
,  "jury. If the synopsis is low,  just increase the -scheme parameter. Read"
,  "the mcl manual for more information. Do not feel intimidated: -scheme is"
,  "your friend, and the synopsis is there  to remind you of its existence -"
,  "you can forget the rest."
,  ""
,  NULL
}  ;

void juryCharter
(  void
)
   {  grade*   gr    =  gradeDir+1
   ;  const char** g =  juryBabble
   ;  fputc('\n', stdout)
   ;  while (gr->mark >= 0)
         fprintf(stdout, "%3d%20s\n", (int) gr->mark, gr->ind)
      ,  gr++
   ;  while (*g)
      fprintf(stdout, "%s\n", *g++)
   ;  return
;  }


void mclWriteLog
(  FILE* fp
,  mclAlgParam* mlp
,  mclMatrix* cl
)
   {  fputs("\n(mclruninfo\n\n", fp)
   ;  mclAlgPrintInfo(fp, mlp, cl)
   ;  fputc('\n', fp)
   ;  mclProcPrintInfo(fp, mlp->mpp)
   ;  fputc('\n', fp)
   ;  fprintf(mlp->xfout->fp, mlp->mpp->massLog->str)
   ;  fputs(")\n", fp)
;  }


void  howMuchRam
(  long  n
,  mclProcParam* mpp
)
   {  int x    =  MCX_MAX(mpp->mxp->num_select, mpp->mxp->num_recover)
   ;  int y    =  MCX_MIN(n, x)
   ;  mclIvp ivps[10]
   ;  int l    =  sizeof(ivps) / 10
   ;  double r =  (2.0 * l * y * n) / (1024.0 * 1024.0)
   ;  fprintf
      (  stdout
      ,  "The current settings require at most <%.2fM> RAM for a\n"
         "graph with <%ld> nodes, assuming the average node degree of\n"
         "the input graph does not exceed <%ld>. This (RAM number)\n"
         "will usually but not always be too pessimistic an estimate.\n"
      ,  (double) r
      ,  (long) n
      ,  (long) y
      )
;  }


void mclAlgOptionsInit
(  void
)
   {  mcxOptAnchorSortById
      (mclAlgOptions, sizeof(mclAlgOptions)/sizeof(mcxOptAnchor) -1)
;  }


/* This tests the matrix domain against the tab domain.
*/

static mclx* test_tab
(  mclx* mx
,  mclAlgParam* mlp
)
   {  mclv* dom
   ;  if (!mlp->tab)
      return mx

   ;  dom = mlp->tab->domain

                        /* DangerSign fixme TODO */
                        /* we are (ab)using the stream interface here */
   ;  if (mlp->stream_modes & MCLXIO_STREAM_GTAB_STRICT)
      {  if (!mcldEquate(mx->dom_cols, dom, MCLD_EQT_SUB))
         mcxDie(1, "mcl", "tab domain does not subsume matrix domain")
   ;  }
                        /* DangerSign fixme TODO */
                        /* we are (ab)using the stream interface here */
      else if (mlp->stream_modes & MCLXIO_STREAM_GTAB_RESTRICT)
      {  if (!mcldEquate(mx->dom_cols, dom, MCLD_EQT_EQUAL))
         {  mclx* sub = mclxSub(mx, dom, dom)
         ;  mclxFree(&mx)
         ;  mx = sub
      ;  }
      }
      return mx
;  }


static mclx* mclAlgorithmStreamIn
(  mcxIO* xfin
,  mclAlgParam* mlp
,  mcxbool reread
)
   {  mclx* mx = NULL
   ;  mcxbits stream_tab_modes =    MCLXIO_STREAM_GTAB_EXTEND
                                 |  MCLXIO_STREAM_GTAB_STRICT
                                 |  MCLXIO_STREAM_GTAB_RESTRICT
   ;  mclxIOstreamer streamer = { 0 }

   ;  streamer.tab_sym_in = mlp->tab

   ;  if (reread && mlp->tab)
      {  BIT_OFF(mlp->stream_modes, stream_tab_modes)
      ;  BIT_ON(mlp->stream_modes, MCLXIO_STREAM_GTAB_RESTRICT)
      ;  mcxLog(MCX_LOG_MODULE, "mclAlgorithmStreamIn", "reconstricting matrix")
;if (mlp->tab)
fprintf(stderr, "reconstrict tab with %d entries: %p\n", (int) N_TAB(mlp->tab), (void*) mlp->tab)
   ;  }

      mx
      =  mclxIOstreamIn
         (  xfin
         ,  mlp->stream_modes | MCLXIO_STREAM_MIRROR | MCLXIO_STREAM_SYMMETRIC
         ,  mlp->stream_transform
         ,  mclpMergeMax
         ,  &streamer
         ,  EXIT_ON_FAIL
         )

   ;  if (mlp->fn_write_tab)
      {  mcxIO* xftab = mcxIOnew(mlp->fn_write_tab->str, "w")
      ;  mcxIOopen(xftab, EXIT_ON_FAIL)
      ;  if (streamer.tab_sym_out)
         mclTabWrite(streamer.tab_sym_out, xftab, NULL, RETURN_ON_FAIL)
      ;  else
         mclTabWrite(streamer.tab_sym_in, xftab, NULL, RETURN_ON_FAIL)
      ;  mcxIOfree(&xftab)
   ;  }

      if (streamer.tab_sym_out)
      {  mcxLog(MCX_LOG_MODULE, "mcl", "new tab created")
;fprintf(stderr, "streamin tab_sym_out %p mlp->tab with  %d entries:\n", (void*) streamer.tab_sym_out, (int) N_TAB(streamer.tab_sym_out))
      ;  if (!(reread && mlp->tab))
         mclTabFree(&(mlp->tab))
      ;  mlp->tab = streamer.tab_sym_out
;fprintf(stderr, "streamin read tab with %d entries: %p\n", (int) N_TAB(mlp->tab), (void*) mlp->tab)
   ;  }

   /*  hierverder: when 'reconstricting' no real new tab is made, apparently.
    *  !@)#$&@!*(#&@*)#&*) design some interface for this.
    *  Or adhere to it, if it was designed.
    *  Or write a wrapper to manage the tabs, if it is hard.
   */

      mlp->mpp->dump_tab = mlp->tab

   ;  return mx
;  }


static int mclAlgorithmTransform
(  mclx* mx  
,  mclAlgParam* mlp
,  mcxbool reread
)
   {  int n_ops = 0
   ;  mclv* shadow_factors = NULL

         /* if we are rereading we do not want shadowing:
          * we reread to do postprocessing.
         */
   ;  if (mlp->modes & ALG_DO_DISCARDLOOPS)
      mclxAdjustLoops(mx, mclxLoopCBremove, NULL)

   ;  if (mlp->pre_maxnum)
      {  dim n_hub = 0, n_out = 0, n_in = 0
      ;  mclv* sel = mclgCeilNB(mx, mlp->pre_maxnum, &n_hub, &n_in, &n_out)
      ;  mcxLog
         (  MCX_LOG_FUNC
         ,  "mcl"
         ,  "considered %lu nodes in prepruning step (%lu needed, %lu edges out, %lu edges in)"
         ,  (ulong) sel->n_ivps
         ,  (ulong) n_hub
         ,  (ulong) n_out
         ,  (ulong) n_in
         )
      ;  n_ops++
      ;  mclvFree(&sel)
   ;  }

      if (mlp->transform)
      {  dim eb = mclxNrofEntries(mx), ea
      ;  mclxUnaryList(mx, mlp->transform)
      ;  ea = mclxNrofEntries(mx)
      ;  mcxLog
         (  MCX_LOG_APP
         ,  "mcl"
         ,  "transform: went from %lu to %lu entries"
         ,  (ulong) eb
         ,  (ulong) ea
         )
      ;  n_ops++
   ;  }


      if (!reread && (mlp->shadow_mode & MCL_SHADOW_EARLY))
      shadow_factors
      =  mcl_get_shadow_turtle_factors
         (  mx
         ,  mlp->shadow_mode
         ,  mlp->shadow_s
         )

   ;  if (mlp->pre_inflationx > 0)
      {  dim j
      ;  for (j=0;j<N_COLS(mx);j++)
         {  mclv* vec = mx->cols+j
         ;  if (vec->n_ivps)
            {  double vec_max = mclvMaxValue(vec)
            ;  mclvUnary(vec, fltxPower, &mlp->pre_inflationx)
            ;  if (vec_max > 0)
               {  double fac = pow(vec_max, mlp->pre_inflationx-1)
               ;  mclvUnary(vec, fltxScale, &fac)
            ;  }
            }
         }
         n_ops++
   ;  }

      if (!reread && (mlp->modes & ALG_DO_SHADOW))
      {  if (!shadow_factors) /* might already be computed */
         shadow_factors
            =  mcl_get_shadow_turtle_factors
               (  mx
               ,  mlp->shadow_mode
               ,  mlp->shadow_s
               )
      ;  mlp->shadow_cache_domain
         =  mcl_shadow_matrix
            (  mx
            ,  shadow_factors
            )

;  if(getenv("MCL_DUMP_SHADOW"))
   {  mcxIO* xf = mcxIOnew("-", "w")
   ;  double factor = 1000
   ;  mclx* mx2 = mclxCopy(mx)
;fprintf(stdout, "_________________________>\n")
   ;  mclxUnary(mx2, fltxMul, &factor)
   ;  mclxWrite(mx2, xf, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  mclxFree(&mx2)
   ;  mcxIOfree(&xf)
   ;
   }
      ;  mclvFree(&shadow_factors)
   ;  }

      else if (mlp->modes & ALG_DO_DISCARDLOOPS)
      mclxAdjustLoops(mx, mclxLoopCBmax, NULL)

   ;  if (mlp->center)
      {  dim  j
      ;  mcxbool ns = getenv("MCL_CENTER_NS") ? TRUE : FALSE
      ;  for (j=0;j<N_COLS(mx);j++)
         {  mclv* vec = mx->cols+j  
         ;  mclp* ivp = mclvGetIvp(vec, vec->vid, NULL)
         ;  if (ivp)
            ivp->val *= mlp->center
         ;  if (ns && mlp->modes & ALG_DO_SHADOW && 2*j >= N_COLS(mx))
            break
      ;  }
      }

      if (mlp->pre_inflation >= 0)
         mclxInflate(mx, mlp->pre_inflation)
      ,  n_ops++

   ;  if (!reread)
      mlp->mx_start_sums = mclxColNums(mx, mclvSum, MCL_VECTOR_COMPLETE)

   ;  mclxMakeStochastic(mx)
   ;  return n_ops
;  }


static mcxstatus mclAlgorithmWriteGraph
(  mclx* mx
,  mclAlgParam* mlp
,  char ord
)
   {  mcxTing* name
      =        ord == 'a'
         ?  mlp->fn_write_input
         :     ord == 'b'
         ?  mlp->fn_write_start
         :  NULL

   ;  if (name)
      {  mcxIO* xfmx = mcxIOnew(name->str, "w")
      ;  if (mcxIOopen(xfmx, RETURN_ON_FAIL))
         return STATUS_FAIL
      ;  if (mclxWrite(mx, xfmx, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL))
         return STATUS_FAIL
      ;  mcxIOclose(xfmx)
      ;  mcxIOfree(&xfmx)
   ;  }
      return STATUS_OK
;  }


   /* fixme the reread+cache logic is hard to follow */

mcxstatus mclAlgorithmStart
(  mclAlgParam* mlp
,  mcxbool reread          /* can this be state-encoded in mlp? */
)
   {  const char* me =  "mclAlgorithmStart"
   ;  mcxTing* cache_name = NULL
   ;  mclx* mx_input =  NULL
   ;  mclx* mx_start =  NULL
   ;  mcxIO* xfin    =  mcxIOnew(mlp->fnin->str, "r")

   ;  while (1)
      {  if (mlp->mx_start)
         {  mcxLog
            (  MCX_LOG_MODULE
            ,  me
            ,  "using cached input matrix (%lu nodes)"
            ,  (ulong) N_COLS(mlp->mx_start)
            )
         ;  mx_start = mlp->mx_start
         ;  break
      ;  }

      ;  if
         (  (mlp->transform_spec && !mlp->transform)
         && !(mlp->transform = mclpTFparse(NULL, mlp->transform_spec))
         )
         {  mcxErr("mcl", "errors in tf-spec")
         ;  break
      ;  }

         if (mlp->mx_input)
         {  mcxLog
            (  MCX_LOG_MODULE
            ,  me
            ,  "using cached input matrix (%lu nodes)"
            ,  (ulong) N_COLS(mlp->mx_input)
            )
         ;  mx_input = mlp->mx_input
         ;  break
      ;  }
                                    /* if reread and xfin no good try cache */
         if (reread && (mcxIOopen(xfin, RETURN_ON_FAIL) || mcxIOstdio(xfin)))
         {  cache_name  =     mlp->fn_write_input
                           ?  mlp->fn_write_input
                           :  mlp->fn_write_start
         ;  if (cache_name)
            {  mcxIOclose(xfin)
            ;  mcxIOrenew(xfin, cache_name->str, NULL)
            ;  mcxLog
               (  MCX_LOG_MODULE
               ,  me
               ,  "fall-back, trying to read cached graph <%s>"
               ,  cache_name->str
               )
            ;  if (mcxIOopen(xfin, RETURN_ON_FAIL))
               {  mcxLog(MCX_LOG_MODULE, me, "fall-back failed")
               ;  mcxIOfree(&xfin)
            ;  }
            }
            else
            mcxIOfree(&xfin)
         ;  if (xfin)
            mlp->stream_modes = 0      /* cached graph is mcl format */
      ;  }

         if (!xfin)
         break

      ;  if
         (  (mlp->stream_transform_spec && !mlp->stream_transform)
         && !(mlp->stream_transform = mclpTFparse(NULL, mlp->stream_transform_spec))
         )
         {  mcxErr("mcl", "errors in stream tf-spec")
         ;  break
      ;  }

         if (mlp->stream_modes & MCLXIO_STREAM_ABC)
         mx_input = mclAlgorithmStreamIn(xfin, mlp, reread)
      ;  else
         {  mx_input = mclxReadx(xfin, RETURN_ON_FAIL, MCLX_REQUIRE_GRAPH)
         ;  if (mx_input)
            mx_input = test_tab(mx_input, mlp)
      ;  }

         break
   ;  }

      mcxIOfree(&xfin)

   ;  if (!mx_input && !mx_start)
      return STATUS_FAIL

   ;  if (mx_start)
      return STATUS_OK

      /* now mx_input && !mx_start */

              /* might be a noop */
   ;  mclAlgorithmWriteGraph(mx_input, mlp, 'a')

   ;  if (mlp->modes & ALG_CACHE_INPUT)                  /* mq cache input */
      {  mx_start = mclxCopy(mx_input)
      ;  mlp->mx_input = mx_input
   ;  }
      else                      /* fixme somewhat ugly input/start+cache semantics */
         mx_start = mx_input
      ,  mlp->mx_input = NULL

   ;  mclAlgorithmTransform(mx_start, mlp, reread)

   ;  mclAlgorithmWriteGraph(mx_start, mlp, 'b')         /* might be a noop */
   ;  mclSetProgress(N_COLS(mx_start), mlp->mpp)
   ;  if (!N_COLS(mx_start))
      mcxErr(me, "attempting to cluster the void")

   ;  mlp->mx_start = mx_start
   ;  return STATUS_OK
;  }



mcxstatus mclAlgInterface
(  mclAlgParam** mlppp
,  char** argv2
,  int argc2
,  const char* fn_input
,  mclx*       mx_input
,  mcxbits modes
)
   {  mclProcParam*  mpp         =  mclProcParamNew()
   ;  mcxstatus      libStatus   =  STATUS_FAIL
   ;  mcxstatus      mainStatus  =  STATUS_FAIL
   ;  mcxstatus      parseStatus =  STATUS_FAIL
   ;  const char*    me          =  "mcl"
   ;  mclAlgParam*   mlp         =  mclAlgParamNew(mpp, mx_input)

   ;  mcxHash        *procOpts, *algOpts, *mergedOpts
   ;  mcxOption      *opts

   ;  mlp->modes |= modes                 /* _ fixme ugly cast */

   ;  mcxTingFree(&mlp->cline)
   ;  mlp->cline = mcxOptArgLine((const char**) argv2, argc2, '"')

   ;  *mlppp = mlp

   ;  mclProcOptionsInit()
   ;  mclAlgOptionsInit()

   ;  procOpts    =  mcxOptHash(mclProcOptions, NULL)
   ;  algOpts     =  mcxOptHash(mclAlgOptions, NULL)
   ;  mergedOpts  =  mcxHashMerge(procOpts, algOpts, NULL, NULL)

   ;  if
      (  !argc2 && !mx_input && fn_input[0] == '-'
      && mcxOptIsInfo(fn_input, mclAlgOptions)
      )
         argv2 = (char**) &fn_input
      ,  argc2 = 1

   ;  opts =  mcxHOptParse
              (mergedOpts, (char**) argv2, argc2, 0, 0, &parseStatus)

   ;  do
      {  if (parseStatus != MCX_OPT_STATUS_OK)
         {  mcxErr(me, "error while parsing options")
         ;  mcxTell(me, "do 'mcl - -h' or 'man mcl'")
         ;  break
      ;  }
            
         libStatus = mclProcessInit(opts, procOpts, mpp)

      ;  if (libStatus == STATUS_FAIL)
         {  mcxErr(me, "initialization failed")
         ;  mcxTell(me, "do 'mcl -h' or 'man mcl'")
         ;  break
      ;  }

         mainStatus = mclAlgorithmInit(opts, algOpts, fn_input, mlp)

      ;  if (mainStatus == ALG_INIT_FAIL)
         {  mcxErr(me, "initialization failed")
         ;  mcxTell(me, "do 'mcl -h' or 'man mcl'")
      ;  }
      }
      while (0)

   ;  mcxOptFree(&opts)
   ;  mcxOptHashFree(&algOpts)
   ;  mcxOptHashFree(&procOpts)
   ;  mcxOptHashFree(&mergedOpts)

   ;  return mainStatus
;  }



