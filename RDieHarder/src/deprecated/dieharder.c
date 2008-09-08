
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <R.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include "dieharder.h"		/* from the front-end sources */

SEXP dieharder(SEXP genS, SEXP testS, SEXP seedS, SEXP psamplesS, 
	       SEXP rngdrawS, SEXP verbS) {
  int verb;
  unsigned int i;
  SEXP result = NULL, vec, pv, name, desc, nkps;
  extern int optind;

  /*
   * Parse command line and set global variables
   */
  char *argv[] = { "dieharder" };
  optind = 0;
  parsecl(1, argv); 

  generator  = INTEGER_VALUE(genS);
  diehard = INTEGER_VALUE(testS);
  Seed = INTEGER_VALUE(seedS); /* (user-select) Seed, not (save switch) seed */
  psamples = INTEGER_VALUE(psamplesS);  /* samples for overall KuiperKS test */
  tsamples = INTEGER_VALUE(rngdrawS);   /* draws for one run of chosen rng */
  verb = INTEGER_VALUE(verbS);

  if (Seed == 0) {
    seed = random_seed();
  } else {
    seed = (unsigned int) Seed;
  }

  if (verb) {
    Rprintf("Dieharder called with gen=%d test=%d seed=%u\n", 
	    generator, diehard, seed);
    quiet = 0;
    hist_flag = 1;
  } else {
    quiet = 1; 			/* override dieharder command-line default */
    hist_flag = 0;
  }

  /*
   * Note that most of my cpu_rates (except the terminally simple/stupid) 
   * have three phases after parsecl():
   *
   * Startup: Allocate memory, initialize all derivative variables from
   * command line values.  
   */
  startup();

  /*
   * Work: Do all the work.  In a complicated cpu_rate, project_work would
   * itself be a shell for a lot of other modular routines.
   */
  work();

  /* vector of size three: [0] is scalar ks_pv, [1] is pvalues vec, [2] name */
  PROTECT(result = allocVector(VECSXP, 3)); 
  /* alloc scalar, and set it */
  PROTECT(pv = allocVector(REALSXP, 1));
  REAL(pv)[0] = testptr->ks_pvalue;
  /* vector and set it */
  PROTECT(vec = allocVector(REALSXP, testptr->psamples)); 
  for (i = 0; i < testptr->psamples; i++) {
    REAL(vec)[i] = testptr->pvalues[i];
  }
  PROTECT(name = allocVector(STRSXP, 1));
  SET_STRING_ELT(name, 0, mkChar(dtestptr->name));
  //PROTECT(desc = allocVector(STRSXP, 1));
  //SET_STRING_ELT(desc, 0, mkChar(dtestptr->description));
  //PROTECT(nkps = allocVector(REALSXP, 1));
  //REAL(nkps)[0] = (double) testptr->nkps;

  /* insert scalar and vector */
  SET_VECTOR_ELT(result, 0, pv);
  SET_VECTOR_ELT(result, 1, vec);
  SET_VECTOR_ELT(result, 2, name);
  
  //SET_VECTOR_ELT(result, 3, desc);  /* too long, and C formatted */
  //SET_VECTOR_ELT(result, 4, nkps);  /* not sure this is needed or useful */
  
  UNPROTECT(4);
  return result;
}

SEXP dieharderVectorised(SEXP genS, SEXP testS, SEXP verbS) {
  //int *genvec = INTEGER(genS);
  int ngen, i;
  diehard = INTEGER_VALUE(testS);
  int verb = INTEGER_VALUE(verbS);
  SEXP result = NULL;
  ngen = length(genS);

  if (verb) {
    Rprintf("C function dieharder called with gen=%d test=%d\n", 
	    generator, diehard);
  }

  /*
   * Parse command line and set global variables
   */
  /*parsecl(argc,argv); */

  quiet = 1; 			/* override dieharder command-line default */

  PROTECT(result = allocVector(REALSXP, ngen));

  for (i=0; i<ngen; i++) {
    generator = INTEGER( VECTOR_ELT(genS, i) )[0];


  /*
   * Note that most of my cpu_rates (except the terminally simple/stupid) 
   * have three phases after parsecl():
   *
   * Startup: Allocate memory, initialize all derivative variables from
   * command line values.  
   */
    startup();

  /*
   * Work: Do all the work.  In a complicated cpu_rate, project_work would
   * itself be a shell for a lot of other modular routines.
   */

    work();
    Rprintf("C function dieharder called with gen=%d test=%d -> %f\n", generator, diehard, testptr->ks_pvalue);
    REAL(result)[i] = testptr->ks_pvalue;
  }

  UNPROTECT(1);
  return result;
}

SEXP dieharderGenerators(void) {
  SEXP result, gens;
  int i;

  /* from startup.c */
  types = gsl_rng_types_setup ();
  i = 0;
  while(types[i] != NULL){
    i++;
  }
  num_gsl_rngs = i;

  /*
   * Now add my own types and count THEM.
   */
  add_my_types();
  while(types[i] != NULL){
    i++;
  }
  num_rngs = i;

  /* vector of size onetwo: [0] is scalar ks_pv, [1] is vector of pvalues */
  PROTECT(result = allocVector(VECSXP, 1)); 
  PROTECT(gens = allocVector(STRSXP, num_rngs)); 
  for (i = 0; i < num_rngs; i++) {
    SET_STRING_ELT(gens, i, mkChar(types[i]->name));
  }
  SET_VECTOR_ELT(result, 0, gens);
  
  UNPROTECT(2);
  return result;
}