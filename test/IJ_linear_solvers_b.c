/*--------------------------------------------------------------------------
 * Test driver for unstructured matrix interface (IJ_matrix interface).
 * -- modified to use new Babel-generated interface, jfp 0400 --
 * Do `driver -help' for usage info.
 * This driver started from the driver for parcsr_linear_solvers, and it
 * works by first building a parcsr matrix as before and then "copying"
 * that matrix row-by-row into the IJMatrix interface. AJC 7/99.
 *--------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "utilities.h"
#include "HYPRE.h"
#include "HYPRE_parcsr_mv.h"

#include "HYPRE_IJ_mv.h"
#include "IJ_matrix_vector.h"
#include "HYPRE_parcsr_ls.h"

#include "Hypre_LinearOperator_Stub.h"
#include "Hypre_ParCSRMatrixBuilder_Stub.h"
#include "Hypre_ParCSRMatrix_Stub.h"
#include "Hypre_ParCSRVectorBuilder_Stub.h"
#include "Hypre_ParCSRVector_Stub.h"
#include "Hypre_ParAMG_Stub.h"
#include "Hypre_MPI_Com_Stub.h"
#include "Hypre_PCG_Stub.h"
#include "Hypre_GMRES_Stub.h"

int BuildParFromFile (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );
int BuildParLaplacian (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );
int BuildParDifConv (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );
int BuildParFromOneFile (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );
int BuildRhsParFromOneFile (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix A , HYPRE_ParVector *b_ptr );
int BuildParLaplacian9pt (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );
int BuildParLaplacian27pt (int argc , char *argv [], int arg_index , HYPRE_ParCSRMatrix *A_ptr );

#define SECOND_TIME 0
 
int
main( int   argc,
      char *argv[] )
{
   int                 arg_index;
   int                 print_usage;
   int                 generate_matrix = 0;
   int                 build_matrix_type;
   int                 build_matrix_arg_index;
   int                 build_rhs_type;
   int                 build_rhs_arg_index;
   int                 solver_id;
   int                 ioutdat;
   int                 debug_flag;
   int                 ierr,i,j; 
   int                 max_levels = 25;
   int                 num_iterations; 
   double              norm;
   double              final_res_norm;
   double              dummy_double;
   double * pdouble = &dummy_double;

   Hypre_ParCSRMatrixBuilder     MatBuilder;
   Hypre_ParCSRMatrix      ij_matrix_Hypre;
   Hypre_ParCSRVectorBuilder     VecBuilder;
   Hypre_LinearOperator linop;

   HYPRE_IJMatrix      ij_matrix;
   HYPRE_IJVector      ij_b;
   HYPRE_IJVector      ij_x;

   /* concrete underlying type for ij_matrix defaults to parcsr. AJC. */
   int                 ij_matrix_storage_type=HYPRE_PARCSR;
   int                 ij_vector_storage_type=HYPRE_PARCSR;

   Hypre_ParCSRMatrix  parcsr_A_Hypre;
   Hypre_ParCSRVector  b_Hypre;
   Hypre_ParCSRVector  x_Hypre;
   Hypre_Vector  b_HypreV;
   Hypre_Vector  x_HypreV;

   HYPRE_ParCSRMatrix  parcsr_A;
   HYPRE_ParCSRMatrix  A;

   Hypre_ParAMG        AMG_Solver;
   Hypre_PCG           PCG_Solver;
   Hypre_Solver        PCG_Precond;
   Hypre_GMRES         GMRES_Solver;
   Hypre_Solver        GMRES_Precond;

   HYPRE_ParVector     b;
   HYPRE_ParVector     x;

   HYPRE_Solver        amg_solver;
   HYPRE_Solver        pcg_solver;
   HYPRE_Solver        pcg_precond;

   int                 num_procs, myid;
   int                 global_n;
   int                 local_row;
   const int          *partitioning;
   int                *part_b;
   int                *part_x;
   int                *row_sizes;
   int                *diag_sizes;
   int                *offdiag_sizes;

   int		       time_index;

   Hypre_MPI_Com Hcomm;
   MPI_Comm comm;

   int M, N;
   int first_local_row, last_local_row;
   int first_local_col, last_local_col;
   int size, *col_ind;
   double *values;

   /* parameters for BoomerAMG */
   double   strong_threshold;
   double   trunc_factor;
   int      cycle_type;
   int      coarsen_type = 6;
   int      hybrid = 1;
   int      measure_type = 0;
   int     *num_grid_sweeps;  
   int     *grid_relax_type;   
   int    **grid_relax_points;
   int      relax_default;
   double  *relax_weight; 
   double   tol = 1.0e-6;
   array1int Num_Grid_Sweeps;
   array1int Grid_Relax_Type;
   array2int Grid_Relax_Points;
   array1double Relax_Weight;
   array1int testint;
   array1double testdouble;

   /* parameters for PILUT */
   double   drop_tol = -1;
   int      nonzeros_to_keep = -1;

   /* parameters for GMRES */
   int	    k_dim;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   /* Initialize MPI */
   MPI_Init(&argc, &argv);

   MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
   MPI_Comm_rank(MPI_COMM_WORLD, &myid );
   /* Make a MPI_Com object. */
   Hcomm = Hypre_MPI_Com_Constructor( MPI_COMM_WORLD );

   MatBuilder = Hypre_ParCSRMatrixBuilder_Constructor( Hcomm, 0,0 );
   VecBuilder = Hypre_ParCSRVectorBuilder_Constructor( Hcomm, 0 );
/*
   hypre_InitMemoryDebug(myid);
*/
   /*-----------------------------------------------------------
    * Set defaults
    *-----------------------------------------------------------*/
 
   build_matrix_type      = 1;
   build_matrix_arg_index = argc;
   build_rhs_type = 0;
   build_rhs_arg_index = argc;
   relax_default = 3;
   debug_flag = 0;

   solver_id = 0;

   ioutdat = 3;

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/
 
   print_usage = 0;
   arg_index = 1;

   while ( (arg_index < argc) && (!print_usage) )
   {
      if ( strcmp(argv[arg_index], "-fromfile") == 0 )
      {
         arg_index++;
         build_matrix_type      = 0;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-fromonefile") == 0 )
      {
         arg_index++;
         build_matrix_type      = 2;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-laplacian") == 0 )
      {
         arg_index++;
         build_matrix_type      = 1;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-9pt") == 0 )
      {
         arg_index++;
         build_matrix_type      = 3;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-27pt") == 0 )
      {
         arg_index++;
         build_matrix_type      = 4;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-difconv") == 0 )
      {
         arg_index++;
         build_matrix_type      = 5;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-exact_size") == 0 )
      {
         arg_index++;
         generate_matrix = 1;
      }
      else if ( strcmp(argv[arg_index], "-storage_low") == 0 )
      {
         arg_index++;
         generate_matrix = 2;
      }
      else if ( strcmp(argv[arg_index], "-concrete_parcsr") == 0 )
      {
         arg_index++;
         ij_matrix_storage_type      = HYPRE_PARCSR;
         build_matrix_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-solver") == 0 )
      {
         arg_index++;
         solver_id = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-rhsfromfile") == 0 )
      {
         arg_index++;
         build_rhs_type      = 1;
         build_rhs_arg_index = arg_index;
      }
      else if ( strcmp(argv[arg_index], "-rhsfromonefile") == 0 )
      {
         arg_index++;
         build_rhs_type      = 2;
         build_rhs_arg_index = arg_index;
      }      
      else if ( strcmp(argv[arg_index], "-rhsrand") == 0 )
      {
         arg_index++;
         build_rhs_type      = 3;
         build_rhs_arg_index = arg_index;
      }    
      else if ( strcmp(argv[arg_index], "-cljp") == 0 )
      {
         arg_index++;
         coarsen_type      = 0;
      }    
      else if ( strcmp(argv[arg_index], "-ruge") == 0 )
      {
         arg_index++;
         coarsen_type      = 1;
      }    
      else if ( strcmp(argv[arg_index], "-ruge2b") == 0 )
      {
         arg_index++;
         coarsen_type      = 2;
      }    
      else if ( strcmp(argv[arg_index], "-ruge3") == 0 )
      {
         arg_index++;
         coarsen_type      = 3;
      }    
      else if ( strcmp(argv[arg_index], "-ruge3c") == 0 )
      {
         arg_index++;
         coarsen_type      = 4;
      }    
      else if ( strcmp(argv[arg_index], "-rugerlx") == 0 )
      {
         arg_index++;
         coarsen_type      = 5;
      }    
      else if ( strcmp(argv[arg_index], "-falgout") == 0 )
      {
         arg_index++;
         coarsen_type      = 6;
      }    
      else if ( strcmp(argv[arg_index], "-nohybrid") == 0 )
      {
         arg_index++;
         hybrid      = -1;
      }    
      else if ( strcmp(argv[arg_index], "-gm") == 0 )
      {
         arg_index++;
         measure_type      = 1;
      }    
      else if ( strcmp(argv[arg_index], "-xisone") == 0 )
      {
         arg_index++;
         build_rhs_type      = 4;
         build_rhs_arg_index = arg_index;
      }    
      else if ( strcmp(argv[arg_index], "-rlx") == 0 )
      {
         arg_index++;
         relax_default = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-dbg") == 0 )
      {
         arg_index++;
         debug_flag = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-help") == 0 )
      {
         print_usage = 1;
      }
      else
      {
         arg_index++;
      }
   }

   /* for CGNR preconditioned with Boomeramg, only relaxation scheme 2 is
      implemented, i.e. Jacobi relaxation with Matvec */
   if (solver_id == 5) relax_default = 2;

   /* defaults for BoomerAMG */
   if (solver_id == 0 || solver_id == 1 || solver_id == 3 || solver_id == 5)
   {
   strong_threshold = 0.25;
   trunc_factor = 0.0;
   cycle_type = 1;

   /* TO DO: release all this space ... */
   num_grid_sweeps   = hypre_CTAlloc(int,4);
   grid_relax_type   = hypre_CTAlloc(int,4);
   grid_relax_points = hypre_CTAlloc(int *,4);
   relax_weight      = hypre_CTAlloc(double, max_levels);
   Num_Grid_Sweeps.lower = hypre_CTAlloc(int,1);
   Num_Grid_Sweeps.upper = hypre_CTAlloc(int,1);
   Grid_Relax_Type.lower = hypre_CTAlloc(int,1);
   Grid_Relax_Type.upper = hypre_CTAlloc(int,1);
   Grid_Relax_Points.lower = hypre_CTAlloc(int,2);
   Grid_Relax_Points.upper = hypre_CTAlloc(int,2);
   Relax_Weight.lower = hypre_CTAlloc(int,1);
   Relax_Weight.upper = hypre_CTAlloc(int,1);

   Num_Grid_Sweeps.lower[0] = 0;
   Num_Grid_Sweeps.upper[0] = 4;
   Num_Grid_Sweeps.data = num_grid_sweeps;
   Grid_Relax_Type.lower[0] = 0;
   Grid_Relax_Type.upper[0] = 4;
   Grid_Relax_Type.data = grid_relax_type;
   Grid_Relax_Points.lower[0] = 0;
   Grid_Relax_Points.upper[0] = 4;
   Grid_Relax_Points.lower[1] = 0;
   Grid_Relax_Points.upper[1] = 4;
   Grid_Relax_Points.data = hypre_CTAlloc(int,4*4);
   Relax_Weight.lower[0] = 0;
   Relax_Weight.upper[0] = 4;
   Relax_Weight.data = relax_weight;

   for (i=0; i < max_levels; i++)
	relax_weight[i] = 1.0;

   if (coarsen_type == 5)
   {
      /* fine grid */
      num_grid_sweeps[0] = 3;
      grid_relax_type[0] = relax_default; 
      grid_relax_points[0] = hypre_CTAlloc(int, 4); 
      grid_relax_points[0][0] = -2;
      grid_relax_points[0][1] = -1;
      grid_relax_points[0][2] = 1;
   
      /* down cycle */
      num_grid_sweeps[1] = 4;
      grid_relax_type[1] = relax_default; 
      grid_relax_points[1] = hypre_CTAlloc(int, 4); 
      grid_relax_points[1][0] = -1;
      grid_relax_points[1][1] = 1;
      grid_relax_points[1][2] = -2;
      grid_relax_points[1][3] = -2;
   
      /* up cycle */
      num_grid_sweeps[2] = 4;
      grid_relax_type[2] = relax_default; 
      grid_relax_points[2] = hypre_CTAlloc(int, 4); 
      grid_relax_points[2][0] = -2;
      grid_relax_points[2][1] = -2;
      grid_relax_points[2][2] = 1;
      grid_relax_points[2][3] = -1;
   }
   else
   {   
      /* fine grid */
      num_grid_sweeps[0] = 2;
      grid_relax_type[0] = relax_default; 
      grid_relax_points[0] = hypre_CTAlloc(int, 2); 
      grid_relax_points[0][0] = 1;
      grid_relax_points[0][1] = -1;
  
      /* down cycle */
      num_grid_sweeps[1] = 2;
      grid_relax_type[1] = relax_default; 
      grid_relax_points[1] = hypre_CTAlloc(int, 2); 
      grid_relax_points[1][0] = 1;
      grid_relax_points[1][1] = -1;
   
      /* up cycle */
      num_grid_sweeps[2] = 2;
      grid_relax_type[2] = relax_default; 
      grid_relax_points[2] = hypre_CTAlloc(int, 2); 
      grid_relax_points[2][0] = -1;
      grid_relax_points[2][1] = 1;
   }
   /* coarsest grid */
   num_grid_sweeps[3] = 1;
   grid_relax_type[3] = 9;
   grid_relax_points[3] = hypre_CTAlloc(int, 1);
   grid_relax_points[3][0] = 0;
   }
   for ( i=0; i<4; ++i )
      for ( j=0; j<4; ++j )
         Grid_Relax_Points.data[i+4*j] = grid_relax_points[i][j];



   /* defaults for GMRES */

   k_dim = 5;

   arg_index = 0;
   while (arg_index < argc)
   {
      if ( strcmp(argv[arg_index], "-k") == 0 )
      {
         arg_index++;
         k_dim = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-w") == 0 )
      {
         arg_index++;
        if (solver_id == 0 || solver_id == 1 || solver_id == 3 
		|| solver_id == 5 )
        {
         relax_weight[0] = atof(argv[arg_index++]);
         for (i=1; i < max_levels; i++)
	   relax_weight[i] = relax_weight[0];
        }
      }
      else if ( strcmp(argv[arg_index], "-th") == 0 )
      {
         arg_index++;
         strong_threshold  = atof(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-tol") == 0 )
      {
         arg_index++;
         tol  = atof(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-drop_tol") == 0 )
      {
         arg_index++;
         drop_tol  = atof(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-nonzeros_to_keep") == 0 )
      {
         arg_index++;
         nonzeros_to_keep  = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-tr") == 0 )
      {
         arg_index++;
         trunc_factor  = atof(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-iout") == 0 )
      {
         arg_index++;
         ioutdat  = atoi(argv[arg_index++]);
      }
      else
      {
         arg_index++;
      }
   }

   /*-----------------------------------------------------------
    * Print usage info
    *-----------------------------------------------------------*/
 
   if ( (print_usage) && (myid == 0) )
   {
      printf("\n");
      printf("Usage: %s [<options>]\n", argv[0]);
      printf("\n");
      printf("  -fromfile <filename>   : problem defining matrix from distributed file\n");
      printf("  -fromonefile <filename>: problem defining matrix from standard CSR file\n");
      printf("\n");
      printf("  -laplacian [<options>] : build laplacian problem\n");
      printf("  -9pt [<opts>] : build 9pt 2D laplacian problem\n");
      printf("  -27pt [<opts>] : build 27pt 3D laplacian problem\n");
      printf("  -difconv [<opts>]      : build convection-diffusion problem\n");
      printf("    -n <nx> <ny> <nz>    : problem size per processor\n");
      printf("    -P <Px> <Py> <Pz>    : processor topology\n");
      printf("    -c <cx> <cy> <cz>    : diffusion coefficients\n");
      printf("    -a <ax> <ay> <az>    : convection coefficients\n");
      printf("\n");
      printf("   -exact_size           : inserts immediately into ParCSR structure\n");
      printf("   -storage_low          : allocates not enough storage for aux struct\n");
      printf("   -concrete_parcsr      : use parcsr matrix type as concrete type\n");
      printf("\n");
      printf("   -rhsfromfile          : from distributed file (NOT YET)\n");
      printf("   -rhsfromonefile       : from vector file \n");
      printf("   -rhsrand              : rhs is random vector\n");
      printf("   -xisone               : solution of all ones\n");
      printf("\n");
      printf("  -solver <ID>           : solver ID\n");
      printf("       0=AMG         1=AMG-PCG       \n");
      printf("       2=DS-PCG      3=AMG-GMRES     \n");
      printf("       4=DS-GMRES    5=AMG-CGNR      \n");     
      printf("       6=DS-CGNR     7=PILUT-GMRES   \n");     
      printf("       8=ParaSails-PCG \n");     
      printf("\n");
      printf("   -cljp                 : CLJP coarsening \n");
      printf("   -ruge                 : Ruge coarsening (local)\n");
      printf("   -ruge3                : third pass on boundary\n");
      printf("   -ruge3c               : third pass on boundary, keep c-points\n");
      printf("   -ruge2b               : 2nd pass is global\n");
      printf("   -rugerlx              : relaxes special points\n");
      printf("   -falgout              : local ruge followed by LJP\n");
      printf("   -nohybrid             : no switch in coarsening\n");
      printf("   -gm                   : use global measures\n");
      printf("\n");
      printf("  -rlx <val>             : relaxation type\n");
      printf("       0=Weighted Jacobi  \n");
      printf("       1=Gauss-Seidel (very slow!)  \n");
      printf("       3=Hybrid Jacobi/Gauss-Seidel  \n");
      printf("\n");  
      printf("  -th <val>              : set AMG threshold Theta = val \n");
      printf("  -tr <val>              : set AMG interpolation truncation factor = val \n");
      printf("  -tol <val>             : set AMG convergence tolerance to val\n");
      printf("  -w  <val>              : set Jacobi relax weight = val\n");
      printf("  -k  <val>              : dimension Krylov space for GMRES\n");
      printf("\n");  
      printf("  -drop_tol  <val>       : set threshold for dropping in PILUT\n");
      printf("  -nonzeros_to_keep <val>: number of nonzeros in each row to keep\n");
      printf("\n");  
      printf("  -iout <val>            : set output flag\n");
      printf("       0=no output    1=matrix stats\n"); 
      printf("       2=cycle stats  3=matrix & cycle stats\n"); 
      printf("\n");  
      printf("  -dbg <val>             : set debug flag\n");
      printf("       0=no debugging\n       1=internal timing\n       2=interpolation truncation\n       3=more detailed timing in coarsening routine\n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Check a few things
    *-----------------------------------------------------------*/

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("Running with these driver parameters:\n");
      printf("  solver ID    = %d\n", solver_id);
   }

   /*-----------------------------------------------------------
    * Set up matrix
    *-----------------------------------------------------------*/

   time_index = hypre_InitializeTiming("IJ Interface");
   hypre_BeginTiming(time_index);

/* jfp 0400: The following matrix build functions are interfaces to
   hypre-level functions which do the work, and return pointers to new
   HYPRE-level objects.  We have the following options for Babelizing them:

   1. Use these functions, then stick the HYPRE matrix into the Babel
   interface's Hypre matrix.  That is what I'm doing now.  Advantage: quick,
   simple, not-too-dirty way to convert the HYPRE-level driver into
   something which can test most of the new Babel interfaces.
   Disadvantages: some parts of the Babel interface are not tested, notably
   matrix element loading functions. The need to do this suggests that we
   need to put more into the Babel interface.  This cannot be done in
   Fortran.

   2. Redo the hypre-level matrix build functions using the Babel interface,
   e.g. InsertBlock.  This is feasible, but involves a lot of time for
   little payoff (it's just a demo).

   3. Write an interface to each of the existing construction functions.
   This messes up the SIDL file to an extent which overwhelms any advantages.

   4. Write a generic interface to construction functions.  Implement it by
   dispatching to one of these functions.  I don't have a good argument
   against this, but I don't feel good about it - it feels like it's not
   a real change, it just hides things.

   Most of the above comments apply to vector building as well.

  5.  Replace the HYPRE_IJ setrow stuff with Babel setrow.
  This should Babelize as fully as we will want.
*/

   if ( build_matrix_type == 0 )
   {
      BuildParFromFile(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else if ( build_matrix_type == 1 )
   {
      BuildParLaplacian(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else if ( build_matrix_type == 2 )
   {
      BuildParFromOneFile(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else if ( build_matrix_type == 3 )
   {
      BuildParLaplacian9pt(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else if ( build_matrix_type == 4 )
   {
      BuildParLaplacian27pt(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else if ( build_matrix_type == 5 )
   {
      BuildParDifConv(argc, argv, build_matrix_arg_index, &parcsr_A);
   }
   else
   {
      printf("You have asked for an unsupported problem, problem = \n.", build_matrix_type);
      return(-1);
   }

    
   /*-----------------------------------------------------------
    * Copy the parcsr matrix into the IJMatrix through interface calls
    *-----------------------------------------------------------*/

   ierr = HYPRE_ParCSRMatrixGetComm( parcsr_A, &comm );
   ierr += HYPRE_ParCSRMatrixGetDims( parcsr_A, &M, &N );

   ierr += HYPRE_IJMatrixCreate( comm, &ij_matrix, M, N );

   ierr += HYPRE_IJMatrixSetLocalStorageType(
                 ij_matrix, HYPRE_PARCSR );
   ierr = HYPRE_ParCSRMatrixGetLocalRange( parcsr_A,
             &first_local_row, &last_local_row ,
             &first_local_col, &last_local_col );

   ierr = HYPRE_IJMatrixSetLocalSize( ij_matrix,
                last_local_row-first_local_row+1,
                last_local_col-first_local_col+1 );

/* the following shows how to build an ij_matrix if one has only an
   estimate for the row sizes */
   if (generate_matrix == 1)
   {   
/*  build ij_matrix using exact row_sizes for diag and offdiag */

      diag_sizes = hypre_CTAlloc(int, last_local_row - first_local_row + 1);
      offdiag_sizes = hypre_CTAlloc(int, last_local_row - first_local_row + 1);
      local_row = 0;
      for (i=first_local_row; i<= last_local_row; i++)
      {
         ierr += HYPRE_ParCSRMatrixGetRow( parcsr_A, i, &size, 
		&col_ind, &values );

         for (j=0; j < size; j++)
         {
	 if (col_ind[j] < first_local_row || col_ind[j] > last_local_row)
	       offdiag_sizes[local_row]++;
	 else
	       diag_sizes[local_row]++;
         }
         local_row++;
         ierr += HYPRE_ParCSRMatrixRestoreRow( parcsr_A, i, &size, 
		&col_ind, &values );
      }
      ierr += HYPRE_IJMatrixSetDiagRowSizes ( ij_matrix, 
					(const int *) diag_sizes );
      ierr += HYPRE_IJMatrixSetOffDiagRowSizes ( ij_matrix, 
					(const int *) offdiag_sizes );
      hypre_TFree(diag_sizes);
      hypre_TFree(offdiag_sizes);
      
      ierr = HYPRE_IJMatrixInitialize( ij_matrix );
      
      for (i=first_local_row; i<= last_local_row; i++)
      {
         ierr += HYPRE_ParCSRMatrixGetRow( parcsr_A, i, &size, 
		&col_ind, &values );

         ierr += HYPRE_IJMatrixInsertRow( ij_matrix, size, i, col_ind, values );

         ierr += HYPRE_ParCSRMatrixRestoreRow( parcsr_A, i, &size, 
		&col_ind, &values );

      }
      ierr += HYPRE_IJMatrixAssemble( ij_matrix );
   }
   else
   {
      row_sizes = hypre_CTAlloc(int, last_local_row - first_local_row + 1);

      size = 5; /* this is in general too low, and supposed to test
		   the capability of the reallocation of the interface */ 
      if (generate_matrix == 0) /* tries a more accurate estimate of the
				   storage */
      {
	 if (build_matrix_type == 1) size = 7;
	 if (build_matrix_type == 3) size = 9;
	 if (build_matrix_type == 4) size = 27;
      }

      for (i=0; i < last_local_row - first_local_row + 1; i++)
         row_sizes[i] = size;

      ierr = HYPRE_IJMatrixSetRowSizes ( ij_matrix, (const int *) row_sizes );

      hypre_TFree(row_sizes);

      ierr = HYPRE_IJMatrixInitialize( ij_matrix );

      /* Loop through all locally stored rows and insert them into ij_matrix */
      for (i=first_local_row; i<= last_local_row; i++)
      {
         ierr += HYPRE_ParCSRMatrixGetRow( parcsr_A, i, &size, 
		&col_ind, &values );

         ierr += HYPRE_IJMatrixInsertRow( ij_matrix, size, i, col_ind, values );

         ierr += HYPRE_ParCSRMatrixRestoreRow( parcsr_A, i, &size, 
		&col_ind, &values );
      }
      ierr += HYPRE_IJMatrixAssemble( ij_matrix );
   }
   if (ierr)
   {
       printf("Error in driver building IJMatrix from parcsr matrix. \n");
       return(-1);
   }

   /*-----------------------------------------------------------
    * Fetch the resulting underlying matrix out
    *-----------------------------------------------------------*/

    A = (HYPRE_ParCSRMatrix) HYPRE_IJMatrixGetLocalStorage( ij_matrix);

    ierr += Hypre_ParCSRMatrixBuilder_New_fromHYPRE( MatBuilder, &ij_matrix );
    ierr += Hypre_ParCSRMatrixBuilder_GetConstructedObject
       ( MatBuilder, &linop );
    ij_matrix_Hypre = (Hypre_ParCSRMatrix)
       Hypre_LinearOperator_castTo( linop, "Hypre.ParCSRMatrix" );

#if 0
    /* test Babel-based GetRow/RestoreRow interface ... */
    for ( i=first_local_row; i<=last_local_row; i=i+100 )
    {
       ierr += HYPRE_ParCSRMatrixGetRow( parcsr_A, i, &size, &col_ind, &values );
       ierr += HYPRE_ParCSRMatrixRestoreRow( parcsr_A, i, &size, &col_ind, &values );
       printf("HYPRE matrix\n row %i: ", i);
       for ( j=0; j<size; ++j ) printf(" %i, %f ", col_ind[j], values[j] );
       printf("\n");
       testint.data = col_ind;
       testdouble.data = values;
       ierr += Hypre_ParCSRMatrix_GetRow
          ( ij_matrix_Hypre, i, &size, &testint, &testdouble );
       ierr += Hypre_ParCSRMatrix_RestoreRow
          ( ij_matrix_Hypre, i, size, testint, testdouble );
       printf("Hypre matrix\n row %i: ", i);
       for ( j=0; j<size; ++j ) printf(" %i, %f ", col_ind[j], values[j] );
       printf("\n");
    };
#endif

#if 0
    /* compare the two matrices that should be the same */
    HYPRE_ParCSRMatrixPrint(parcsr_A, "driver.out.parcsr_A");
    HYPRE_ParCSRMatrixPrint(A, "driver.out.A");
#endif

    HYPRE_ParCSRMatrixDestroy(parcsr_A);
   /*-----------------------------------------------------------
    * Set up the RHS and initial guess
    *-----------------------------------------------------------*/

   ierr = HYPRE_IJMatrixGetRowPartitioning( ij_matrix, &partitioning);

   part_b = hypre_CTAlloc(int, num_procs+1);
   part_x = hypre_CTAlloc(int, num_procs+1);
   for (i=0; i < num_procs+1; i++)
   {
      part_b[i] = partitioning[i];
      part_x[i] = partitioning[i];
   }
   /* HYPRE_ParCSRMatrixGetRowPartitioning(A, &partitioning); */
   HYPRE_ParCSRMatrixGetDims(A, &global_n, &global_n);

   if (build_rhs_type == 1)
   {
      /* BuildRHSParFromFile(argc, argv, build_rhs_arg_index, &b); */
      printf("Rhs from file not yet implemented.  Defaults to b=0\n");
      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_b, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_b,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_b, (const int *) part_b);
      HYPRE_IJVectorInitialize(ij_b);
      HYPRE_IJVectorZeroLocalComponents(ij_b); 

      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_x, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_x,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_x, (const int *) part_x);
      HYPRE_IJVectorInitialize(ij_x);
      HYPRE_IJVectorZeroLocalComponents(ij_x);
      values = hypre_CTAlloc(double, part_x[myid+1] - part_x[myid]);

      for (i = 0; i < part_x[myid+1] - part_x[myid]; i++)
         values[i] = 1.0;

      HYPRE_IJVectorSetLocalComponentsInBlock(ij_x, 
					      part_x[myid], 
					      part_x[myid+1]-1,
                                              NULL,
					      values);
      hypre_TFree(values);

   /*-----------------------------------------------------------
    * Fetch the resulting underlying vectors out
    *-----------------------------------------------------------*/

      b = (HYPRE_ParVector) HYPRE_IJVectorGetLocalStorage( ij_b );
      x = (HYPRE_ParVector) HYPRE_IJVectorGetLocalStorage( ij_x );

   }
   else if ( build_rhs_type == 2 )
   {
      BuildRhsParFromOneFile(argc, argv, build_rhs_arg_index, A, &b);

      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_x, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_x,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_x, (const int *) part_x);
      HYPRE_IJVectorInitialize(ij_x);
      HYPRE_IJVectorZeroLocalComponents(ij_x); 
      x = (HYPRE_ParVector) HYPRE_IJVectorGetLocalStorage( ij_x );

   }
   else if ( build_rhs_type == 3 )
   {

      HYPRE_ParVectorCreate(MPI_COMM_WORLD, global_n, part_b,&b);
      HYPRE_ParVectorInitialize(b);
      HYPRE_ParVectorSetRandomValues(b, 22775);
      HYPRE_ParVectorInnerProd(b,b,&norm);
      norm = 1.0/sqrt(norm);
      ierr = HYPRE_ParVectorScale(norm, b);      

      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_x, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_x,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_x, (const int *) part_x);
      HYPRE_IJVectorInitialize(ij_x);
      HYPRE_IJVectorZeroLocalComponents(ij_x); 
      x = (HYPRE_ParVector) HYPRE_IJVectorGetLocalStorage( ij_x );
   }
   else if ( build_rhs_type == 4 )
   {

      HYPRE_ParVectorCreate(MPI_COMM_WORLD, global_n, part_x, &x);
      HYPRE_ParVectorInitialize(x);
      HYPRE_ParVectorSetConstantValues(x, 1.0);

      HYPRE_ParVectorCreate(MPI_COMM_WORLD, global_n, part_b, &b);
      HYPRE_ParVectorInitialize(b);
      HYPRE_ParCSRMatrixMatvec(1.0,A,x,0.0,b);

      HYPRE_ParVectorSetConstantValues(x, 0.0);
   }
   else /* if ( build_rhs_type == 0 ) */
   {
      HYPRE_ParVectorCreate(MPI_COMM_WORLD, global_n, part_b, &b);
      HYPRE_ParVectorInitialize(b);
      HYPRE_ParVectorSetConstantValues(b, 1.0);

      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_x, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_x,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_x, (const int *) part_x);
      HYPRE_IJVectorInitialize(ij_x);
      HYPRE_IJVectorZeroLocalComponents(ij_x); 
      x = (HYPRE_ParVector) HYPRE_IJVectorGetLocalStorage( ij_x );
   }

   hypre_EndTiming(time_index);
   hypre_PrintTiming("IJ Interface", MPI_COMM_WORLD);
   hypre_FinalizeTiming(time_index);
   hypre_ClearTiming();
 
   if ( ij_b==0 ) {
      /* a kludge upon a kludge. right thing is to have no HYPRE/hypre
         wrong thing is to build it all backwards, as I'm doing */
      /* This block sets up a new HYPRE_IJVector ij_b the standard way, then
         sets the HYPRE_ParVector which it points to to the
         already-constructed b.
      */
      HYPRE_IJVectorCreate(MPI_COMM_WORLD, &ij_b, global_n);
      HYPRE_IJVectorSetLocalStorageType(ij_b,ij_vector_storage_type );
      HYPRE_IJVectorSetPartitioning(ij_b, (const int *) part_b);
      HYPRE_IJVectorInitialize(ij_b);
      hypre_IJVectorLocalStorage((hypre_IJVector*)ij_b) = b;
   };
   ierr += Hypre_ParCSRVectorBuilder_New_fromHYPRE( VecBuilder, Hcomm, &ij_x );
   ierr += Hypre_ParCSRVectorBuilder_GetConstructedObject
      ( VecBuilder, &x_HypreV );
   x_Hypre = (Hypre_ParCSRVector) Hypre_Vector_castTo
      ( x_HypreV, "Hypre.ParCSRVector" );
   ierr += Hypre_ParCSRVectorBuilder_New_fromHYPRE( VecBuilder, Hcomm, &ij_b );
   ierr += Hypre_ParCSRVectorBuilder_GetConstructedObject
      ( VecBuilder, &b_HypreV );
   b_Hypre = (Hypre_ParCSRVector) Hypre_Vector_castTo
      ( b_HypreV, "Hypre.ParCSRVector" );

   /*-----------------------------------------------------------
    * Solve the system using AMG
    *-----------------------------------------------------------*/

   if (solver_id == 0)
   {
      if (myid == 0) printf("Solver:  AMG\n");
      time_index = hypre_InitializeTiming("BoomerAMG Setup");
      hypre_BeginTiming(time_index);

      AMG_Solver = Hypre_ParAMG_Constructor( Hcomm );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "coarsen type",
                                    (hybrid*coarsen_type) );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "measure type", measure_type );
      Hypre_ParAMG_SetParameterDouble( AMG_Solver, "tol", tol );
      Hypre_ParAMG_SetParameterDouble( AMG_Solver, "strong threshold",
                                       strong_threshold );
      Hypre_ParAMG_SetParameterDouble( AMG_Solver, "trunc factor", trunc_factor );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "logging", ioutdat );
      Hypre_ParAMG_SetParameterString( AMG_Solver, "log file name",
                                       "driver.out.log" );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "cycle type", cycle_type );
      Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "num grid sweeps",
                                         Num_Grid_Sweeps );
      Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "grid relax type",
                                         Grid_Relax_Type );
      Hypre_ParAMG_SetParameterDoubleArray( AMG_Solver, "relax weight",
                                            Relax_Weight );
      Hypre_ParAMG_SetParameterIntArray2( AMG_Solver, "grid relax points",
                                          Grid_Relax_Points );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "max levels", max_levels );
      Hypre_ParAMG_SetParameterInt( AMG_Solver, "debug", debug_flag );

/*
      amg_solver = *(AMG_Solver->d_table->Hsolver);
      HYPRE_BoomerAMGCreate(&amg_solver); 
      HYPRE_BoomerAMGSetCoarsenType(amg_solver, (hybrid*coarsen_type));
      HYPRE_BoomerAMGSetMeasureType(amg_solver, measure_type);
      HYPRE_BoomerAMGSetTol(amg_solver, tol);
      HYPRE_BoomerAMGSetStrongThreshold(amg_solver, strong_threshold);
      HYPRE_BoomerAMGSetTruncFactor(amg_solver, trunc_factor);
*/
/* note: log is written to standard output, not to file */
/*
      HYPRE_BoomerAMGSetLogging(amg_solver, ioutdat, "driver.out.log"); 
      HYPRE_BoomerAMGSetCycleType(amg_solver, cycle_type);
      HYPRE_BoomerAMGSetNumGridSweeps(amg_solver, num_grid_sweeps);
      HYPRE_BoomerAMGSetGridRelaxType(amg_solver, grid_relax_type);
      HYPRE_BoomerAMGSetRelaxWeight(amg_solver, relax_weight);
      HYPRE_BoomerAMGSetGridRelaxPoints(amg_solver, grid_relax_points);
      HYPRE_BoomerAMGSetMaxLevels(amg_solver, max_levels);
      HYPRE_BoomerAMGSetDebugFlag(amg_solver, debug_flag);

      HYPRE_BoomerAMGSetup(amg_solver, A, b, x);
*/
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Setup phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
 
      time_index = hypre_InitializeTiming("BoomerAMG Solve");
      hypre_BeginTiming(time_index);

      linop = (Hypre_LinearOperator) Hypre_ParCSRMatrix_castTo(
         ij_matrix_Hypre, "Hypre.LinearOperator" );
      ierr += Hypre_ParAMG_Setup( AMG_Solver, linop, b_HypreV, x_HypreV );
      ierr += Hypre_ParAMG_Apply( AMG_Solver, b_HypreV, &x_HypreV );
/*
      HYPRE_BoomerAMGSolve(amg_solver, A, b, x);
*/
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Solve phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();

#if SECOND_TIME
      /* run a second time to check for memory leaks */
      HYPRE_ParVectorSetRandomValues(x, 775);
      ierr += Hypre_ParAMG_Setup( AMG_Solver, ij_matrix_Hypre, b_HypreV, x_HypreV );
      ierr += Hypre_ParAMG_Apply( AMG_Solver, ij_matrix_Hypre, b_HypreV, x_HypreV );
/*
      HYPRE_BoomerAMGSetup(amg_solver, A, b, x);
      HYPRE_BoomerAMGSolve(amg_solver, A, b, x);
*/
#endif

   Hypre_ParAMG_destructor(AMG_Solver);
/*
      HYPRE_BoomerAMGDestroy(amg_solver);
*/
   }

   /*-----------------------------------------------------------
    * Solve the system using PCG 
    *-----------------------------------------------------------*/

   if (solver_id == 1 || solver_id == 2 || solver_id == 8)
   {
      time_index = hypre_InitializeTiming("PCG Setup");
      hypre_BeginTiming(time_index);
 
      PCG_Solver = Hypre_PCG_Constructor( Hcomm );
      ierr += Hypre_PCG_SetParameterInt( PCG_Solver, "max iter", 500 );
      ierr += Hypre_PCG_SetParameterDouble( PCG_Solver, "tol", tol );
      ierr += Hypre_PCG_SetParameterInt( PCG_Solver, "2-norm", 1 );
      ierr += Hypre_PCG_SetParameterInt( PCG_Solver,
                                         "relative change test", 0 );
      ierr += Hypre_PCG_SetParameterInt( PCG_Solver, "logging", 1 );
/* 
      HYPRE_ParCSRPCGCreate(MPI_COMM_WORLD, &pcg_solver);
      HYPRE_ParCSRPCGSetMaxIter(pcg_solver, 500);
      HYPRE_ParCSRPCGSetTol(pcg_solver, tol);
      HYPRE_ParCSRPCGSetTwoNorm(pcg_solver, 1);
      HYPRE_ParCSRPCGSetRelChange(pcg_solver, 0);
      HYPRE_ParCSRPCGSetLogging(pcg_solver, 1);
*/
 
      if (solver_id == 1)
      {
         /* use BoomerAMG as preconditioner */
         if (myid == 0) printf("Solver: AMG-PCG\n");
         AMG_Solver = Hypre_ParAMG_Constructor( Hcomm );
         PCG_Precond = (Hypre_Solver) Hypre_ParAMG_castTo(
            AMG_Solver, "Hypre.Solver" );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "coarsen type",
                                       (hybrid*coarsen_type) );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "measure type", measure_type );
         Hypre_ParAMG_SetParameterDouble( AMG_Solver, "strong threshold",
                                          strong_threshold );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "max iter", 1 );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "logging", ioutdat );
         Hypre_ParAMG_SetParameterString( AMG_Solver, "log file name",
                                          "driver.out.log" );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "cycle type", cycle_type );
         Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "num grid sweeps",
                                            Num_Grid_Sweeps );
         Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "grid relax type",
                                            Grid_Relax_Type );
         Hypre_ParAMG_SetParameterDoubleArray( AMG_Solver, "relax weight",
                                               Relax_Weight );
         Hypre_ParAMG_SetParameterIntArray2( AMG_Solver, "grid relax points",
                                             Grid_Relax_Points );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "max levels", max_levels );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "debug", debug_flag );

/*
         amg_solver = *(AMG_Solver->d_table->Hsolver);
         HYPRE_BoomerAMGCreate(&pcg_precond); 
         HYPRE_BoomerAMGSetCoarsenType(pcg_precond, (hybrid*coarsen_type));
         HYPRE_BoomerAMGSetMeasureType(pcg_precond, measure_type);
         HYPRE_BoomerAMGSetStrongThreshold(pcg_precond, strong_threshold);
         HYPRE_BoomerAMGSetLogging(pcg_precond, ioutdat, "driver.out.log");
         HYPRE_BoomerAMGSetMaxIter(pcg_precond, 1);
         HYPRE_BoomerAMGSetCycleType(pcg_precond, cycle_type);
         HYPRE_BoomerAMGSetNumGridSweeps(pcg_precond, num_grid_sweeps);
         HYPRE_BoomerAMGSetGridRelaxType(pcg_precond, grid_relax_type);
         HYPRE_BoomerAMGSetRelaxWeight(pcg_precond, relax_weight);
         HYPRE_BoomerAMGSetGridRelaxPoints(pcg_precond, grid_relax_points);
         HYPRE_BoomerAMGSetMaxLevels(pcg_precond, max_levels);
*/
         linop = (Hypre_LinearOperator) Hypre_ParCSRMatrix_castTo(
            ij_matrix_Hypre, "Hypre.LinearOperator" );
         Hypre_ParAMG_Setup( AMG_Solver, linop, b_HypreV, x_HypreV );
         Hypre_PCG_SetPreconditioner( PCG_Solver, PCG_Precond );
/*
         HYPRE_ParCSRPCGSetPrecond(pcg_solver,
                                   HYPRE_BoomerAMGSolve,
                                   HYPRE_BoomerAMGSetup,
                                   pcg_precond);
*/
      }
      else if (solver_id == 2)
      {
         
         /* use diagonal scaling as preconditioner */
         if (myid == 0) printf("Solver: DS-PCG\n");
         pcg_precond = NULL;

         HYPRE_ParCSRPCGSetPrecond(pcg_solver,
                                   HYPRE_ParCSRDiagScale,
                                   HYPRE_ParCSRDiagScaleSetup,
                                   pcg_precond);
      }
      else if (solver_id == 8)
      {
         /* use ParaSails preconditioner */
         if (myid == 0) printf("Solver: ParaSails-PCG\n");

	 HYPRE_ParCSRParaSailsCreate(MPI_COMM_WORLD, &pcg_precond);
	 HYPRE_ParCSRParaSailsSetParams(pcg_precond, 0.1, 1);

         HYPRE_ParCSRPCGSetPrecond(pcg_solver,
                                   HYPRE_ParCSRParaSailsSolve,
                                   HYPRE_ParCSRParaSailsSetup,
                                   pcg_precond);
      }
 
      linop = (Hypre_LinearOperator) Hypre_ParCSRMatrix_castTo(
         ij_matrix_Hypre, "Hypre.LinearOperator" );

      ierr += Hypre_PCG_Setup( PCG_Solver, linop, b_HypreV, x_HypreV );
/*      HYPRE_ParCSRPCGSetup(pcg_solver, A, b, x); */
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Setup phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
   
      time_index = hypre_InitializeTiming("PCG Solve");
      hypre_BeginTiming(time_index);
 
      Hypre_PCG_Apply( PCG_Solver, b_HypreV, &x_HypreV );
/*      HYPRE_ParCSRPCGSolve(pcg_solver, A, b, x); */
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Solve phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
 
      ierr += Hypre_PCG_GetConvergenceInfo( PCG_Solver, "number of iterations",
                                            pdouble );
      num_iterations = *pdouble;
      ierr += Hypre_PCG_GetConvergenceInfo( PCG_Solver, "residual norm",
                                            &final_res_norm );
/*
      HYPRE_ParCSRPCGGetNumIterations(pcg_solver, &num_iterations);
      HYPRE_ParCSRPCGGetFinalRelativeResidualNorm(pcg_solver, &final_res_norm);
*/
#if SECOND_TIME
      /* run a second time to check for memory leaks */
      HYPRE_ParVectorSetRandomValues(x, 775);
      HYPRE_ParCSRPCGSetup(pcg_solver, A, b, x);
      HYPRE_ParCSRPCGSolve(pcg_solver, A, b, x);
#endif

      Hypre_PCG_destructor( PCG_Solver );
/*      HYPRE_ParCSRPCGDestroy(pcg_solver);*/
 
      if (solver_id == 1)
      {
         Hypre_ParAMG_destructor( AMG_Solver );
/*         HYPRE_BoomerAMGDestroy(pcg_precond);*/
      }
      else if (solver_id == 8)
      {
	 HYPRE_ParCSRParaSailsDestroy(pcg_precond);
      }

      if (myid == 0)
      {
         printf("\n");
         printf("Iterations = %d\n", num_iterations);
         printf("Final Relative Residual Norm = %e\n", final_res_norm);
         printf("\n");
      }
 
   }

   /*-----------------------------------------------------------
    * Solve the system using GMRES 
    *-----------------------------------------------------------*/

   if (solver_id == 3 || solver_id == 4 || solver_id == 7)
   {
      time_index = hypre_InitializeTiming("GMRES Setup");
      hypre_BeginTiming(time_index);
 
      GMRES_Solver = Hypre_GMRES_Constructor( Hcomm );
      ierr += Hypre_GMRES_SetParameterInt( GMRES_Solver, "k_dim", k_dim );
      ierr += Hypre_GMRES_SetParameterInt( GMRES_Solver, "max iter", 100 );
      ierr += Hypre_GMRES_SetParameterDouble( GMRES_Solver, "tol", tol );
      ierr += Hypre_GMRES_SetParameterInt( GMRES_Solver, "logging", 1 );
      
/*
      HYPRE_ParCSRGMRESCreate(MPI_COMM_WORLD, &pcg_solver);
      HYPRE_ParCSRGMRESSetKDim(pcg_solver, k_dim);
      HYPRE_ParCSRGMRESSetMaxIter(pcg_solver, 100);
      HYPRE_ParCSRGMRESSetTol(pcg_solver, tol);
      HYPRE_ParCSRGMRESSetLogging(pcg_solver, 1);
*/ 
      if (solver_id == 3)
      {
         /* use BoomerAMG as preconditioner */
         if (myid == 0) printf("Solver: AMG-GMRES\n");

         AMG_Solver = Hypre_ParAMG_Constructor( Hcomm );
         GMRES_Precond = (Hypre_Solver) Hypre_ParAMG_castTo(
            AMG_Solver, "Hypre.Solver" );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "coarsen type",
                                       (hybrid*coarsen_type) );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "measure type", measure_type );
         Hypre_ParAMG_SetParameterDouble( AMG_Solver, "strong threshold",
                                          strong_threshold );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "logging", ioutdat );
         Hypre_ParAMG_SetParameterString( AMG_Solver, "log file name",
                                          "driver.out.log" );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "max iter", 1 );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "cycle type", cycle_type );
         Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "num grid sweeps",
                                            Num_Grid_Sweeps );
         Hypre_ParAMG_SetParameterIntArray( AMG_Solver, "grid relax type",
                                            Grid_Relax_Type );
         Hypre_ParAMG_SetParameterDoubleArray( AMG_Solver, "relax weight",
                                               Relax_Weight );
         Hypre_ParAMG_SetParameterIntArray2( AMG_Solver, "grid relax points",
                                             Grid_Relax_Points );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "max levels", max_levels );
         Hypre_ParAMG_SetParameterInt( AMG_Solver, "debug", debug_flag );

/*
         HYPRE_BoomerAMGCreate(&pcg_precond); 
         HYPRE_BoomerAMGSetCoarsenType(pcg_precond, (hybrid*coarsen_type));
         HYPRE_BoomerAMGSetMeasureType(pcg_precond, measure_type);
         HYPRE_BoomerAMGSetStrongThreshold(pcg_precond, strong_threshold);
         HYPRE_BoomerAMGSetLogging(pcg_precond, ioutdat, "driver.out.log");
         HYPRE_BoomerAMGSetMaxIter(pcg_precond, 1);
         HYPRE_BoomerAMGSetCycleType(pcg_precond, cycle_type);
         HYPRE_BoomerAMGSetNumGridSweeps(pcg_precond, num_grid_sweeps);
         HYPRE_BoomerAMGSetGridRelaxType(pcg_precond, grid_relax_type);
         HYPRE_BoomerAMGSetRelaxWeight(pcg_precond, relax_weight);
         HYPRE_BoomerAMGSetGridRelaxPoints(pcg_precond, grid_relax_points);
         HYPRE_BoomerAMGSetMaxLevels(pcg_precond, max_levels);
*/
         linop = (Hypre_LinearOperator) Hypre_ParCSRMatrix_castTo(
            ij_matrix_Hypre, "Hypre.LinearOperator" );
         Hypre_ParAMG_Setup( AMG_Solver, linop, b_HypreV, x_HypreV );
         Hypre_GMRES_SetPreconditioner( GMRES_Solver, GMRES_Precond );
/*
         HYPRE_ParCSRGMRESSetPrecond(pcg_solver,
                                     HYPRE_BoomerAMGSolve,
                                     HYPRE_BoomerAMGSetup,
                                     pcg_precond);
*/
      }
      else if (solver_id == 4)
      {
         /* use diagonal scaling as preconditioner */
         if (myid == 0) printf("Solver: DS-GMRES\n");
         pcg_precond = NULL;

         HYPRE_ParCSRGMRESSetPrecond(pcg_solver,
                                     HYPRE_ParCSRDiagScale,
                                     HYPRE_ParCSRDiagScaleSetup,
                                     pcg_precond);
      }
      else if (solver_id == 7)
      {
         /* use PILUT as preconditioner */
         if (myid == 0) printf("Solver: Pilut-GMRES\n");

         ierr = HYPRE_ParCSRPilutCreate( MPI_COMM_WORLD, &pcg_precond ); 
         if (ierr) {
	   printf("Error in ParPilutCreate\n");
         }

         HYPRE_ParCSRGMRESSetPrecond(pcg_solver,
                                     HYPRE_ParCSRPilutSolve,
                                     HYPRE_ParCSRPilutSetup,
                                     pcg_precond);

         if (drop_tol >= 0 )
            HYPRE_ParCSRPilutSetDropTolerance( pcg_precond,
               drop_tol );

         if (nonzeros_to_keep >= 0 )
            HYPRE_ParCSRPilutSetFactorRowSize( pcg_precond,
               nonzeros_to_keep );
      }
 
      linop = (Hypre_LinearOperator) Hypre_ParCSRMatrix_castTo(
         ij_matrix_Hypre, "Hypre.LinearOperator" );

      ierr += Hypre_GMRES_Setup( GMRES_Solver, linop, b_HypreV, x_HypreV );
/*      HYPRE_ParCSRGMRESSetup(pcg_solver, A, b, x); */
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Setup phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
   
      time_index = hypre_InitializeTiming("GMRES Solve");
      hypre_BeginTiming(time_index);
 
      Hypre_GMRES_Apply( GMRES_Solver, b_HypreV, &x_HypreV );
/*      HYPRE_ParCSRGMRESSolve(pcg_solver, A, b, x); */
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Solve phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
 
      ierr += Hypre_GMRES_GetConvergenceInfo(
         GMRES_Solver, "number of iterations", pdouble );
      num_iterations = *pdouble;
      ierr += Hypre_GMRES_GetConvergenceInfo(
         GMRES_Solver, "relative residual norm", &final_res_norm );
/*      HYPRE_ParCSRGMRESGetNumIterations(pcg_solver, &num_iterations);
      HYPRE_ParCSRGMRESGetFinalRelativeResidualNorm(pcg_solver,&final_res_norm);
*/
#if SECOND_TIME
      /* run a second time to check for memory leaks */
      HYPRE_ParVectorSetRandomValues(x, 775);
      HYPRE_ParCSRGMRESSetup(pcg_solver, A, b, x);
      HYPRE_ParCSRGMRESSolve(pcg_solver, A, b, x);
#endif

      Hypre_GMRES_destructor( GMRES_Solver );
/*      HYPRE_ParCSRGMRESDestroy(pcg_solver); */
 
      if (solver_id == 3)
      {
         Hypre_ParAMG_destructor( AMG_Solver );
/*         HYPRE_BoomerAMGDestroy(pcg_precond);*/
      }

      if (solver_id == 7)
      {
         HYPRE_ParCSRPilutDestroy(pcg_precond);
      }

      if (myid == 0)
      {
         printf("\n");
         printf("GMRES Iterations = %d\n", num_iterations);
         printf("Final GMRES Relative Residual Norm = %e\n", final_res_norm);
         printf("\n");
      }
   }
   /*-----------------------------------------------------------
    * Solve the system using CGNR 
    *-----------------------------------------------------------*/

   if (solver_id == 5 || solver_id == 6)
   {
      time_index = hypre_InitializeTiming("CGNR Setup");
      hypre_BeginTiming(time_index);
 
      HYPRE_ParCSRCGNRCreate(MPI_COMM_WORLD, &pcg_solver);
      HYPRE_ParCSRCGNRSetMaxIter(pcg_solver, 1000);
      HYPRE_ParCSRCGNRSetTol(pcg_solver, tol);
      HYPRE_ParCSRCGNRSetLogging(pcg_solver, 1);
 
      if (solver_id == 5)
      {
         /* use BoomerAMG as preconditioner */
         if (myid == 0) printf("Solver: AMG-CGNR\n");

         HYPRE_BoomerAMGCreate(&pcg_precond); 
         HYPRE_BoomerAMGSetCoarsenType(pcg_precond, (hybrid*coarsen_type));
         HYPRE_BoomerAMGSetMeasureType(pcg_precond, measure_type);
         HYPRE_BoomerAMGSetStrongThreshold(pcg_precond, strong_threshold);
         HYPRE_BoomerAMGSetLogging(pcg_precond, ioutdat, "driver.out.log");
         HYPRE_BoomerAMGSetMaxIter(pcg_precond, 1);
         HYPRE_BoomerAMGSetCycleType(pcg_precond, cycle_type);
         HYPRE_BoomerAMGSetNumGridSweeps(pcg_precond, num_grid_sweeps);
         HYPRE_BoomerAMGSetGridRelaxType(pcg_precond, grid_relax_type);
         HYPRE_BoomerAMGSetRelaxWeight(pcg_precond, relax_weight);
         HYPRE_BoomerAMGSetGridRelaxPoints(pcg_precond, grid_relax_points);
         HYPRE_BoomerAMGSetMaxLevels(pcg_precond, max_levels);
         HYPRE_ParCSRCGNRSetPrecond(pcg_solver,
                                   HYPRE_BoomerAMGSolve,
                                   HYPRE_BoomerAMGSolveT,
                                   HYPRE_BoomerAMGSetup,
                                   pcg_precond);
      }
      else if (solver_id == 6)
      {
         /* use diagonal scaling as preconditioner */
         if (myid == 0) printf("Solver: DS-CGNR\n");
         pcg_precond = NULL;

         HYPRE_ParCSRCGNRSetPrecond(pcg_solver,
                                   HYPRE_ParCSRDiagScale,
                                   HYPRE_ParCSRDiagScale,
                                   HYPRE_ParCSRDiagScaleSetup,
                                   pcg_precond);
      }
 
      HYPRE_ParCSRCGNRSetup(pcg_solver, A, b, x);
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Setup phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
   
      time_index = hypre_InitializeTiming("CGNR Solve");
      hypre_BeginTiming(time_index);
 
      HYPRE_ParCSRCGNRSolve(pcg_solver, A, b, x);
 
      hypre_EndTiming(time_index);
      hypre_PrintTiming("Solve phase times", MPI_COMM_WORLD);
      hypre_FinalizeTiming(time_index);
      hypre_ClearTiming();
 
      HYPRE_ParCSRCGNRGetNumIterations(pcg_solver, &num_iterations);
      HYPRE_ParCSRCGNRGetFinalRelativeResidualNorm(pcg_solver,&final_res_norm);

#if SECOND_TIME
      /* run a second time to check for memory leaks */
      HYPRE_ParVectorSetRandomValues(x, 775);
      HYPRE_ParCSRCGNRSetup(pcg_solver, A, b, x);
      HYPRE_ParCSRCGNRSolve(pcg_solver, A, b, x);
#endif

      HYPRE_ParCSRCGNRDestroy(pcg_solver);
 
      if (solver_id == 5)
      {
         HYPRE_BoomerAMGDestroy(pcg_precond);
      }
      if (myid == 0)
      {
         printf("\n");
         printf("Iterations = %d\n", num_iterations);
         printf("Final Relative Residual Norm = %e\n", final_res_norm);
         printf("\n");
      }
   }
   /*-----------------------------------------------------------
    * Print the solution and other info
    *-----------------------------------------------------------*/

#if 0
   HYPRE_PrintCSRVector(x, "driver.out.x");
#endif


   /*-----------------------------------------------------------
    * Finalize things
    *-----------------------------------------------------------*/

   Hypre_ParCSRMatrixBuilder_destructor(MatBuilder);
   Hypre_ParCSRMatrix_destructor(ij_matrix_Hypre);
   HYPRE_IJMatrixDestroy(ij_matrix);
   Hypre_ParCSRVectorBuilder_destructor(VecBuilder);
   Hypre_ParCSRVector_destructor(b_Hypre);
   Hypre_ParCSRVector_destructor(x_Hypre);
   if (build_rhs_type == 1)
      HYPRE_IJVectorDestroy(ij_b);
   else
      HYPRE_ParVectorDestroy(b);
   if (build_rhs_type > -1 && build_rhs_type < 4)
      HYPRE_IJVectorDestroy(ij_x);
   else
      HYPRE_ParVectorDestroy(x);
/*
   hypre_FinalizeMemoryDebug();
*/
   /* Finalize MPI */
   MPI_Finalize();

   return (0);
}

/*----------------------------------------------------------------------
 * Build matrix from file. Expects three files on each processor.
 * filename.D.n contains the diagonal part, filename.O.n contains
 * the offdiagonal part and filename.INFO.n contains global row
 * and column numbers, number of columns of offdiagonal matrix
 * and the mapping of offdiagonal column numbers to global column numbers.
 * Parameters given in command line.
 *----------------------------------------------------------------------*/

int
BuildParFromFile( int                  argc,
                  char                *argv[],
                  int                  arg_index,
                  HYPRE_ParCSRMatrix  *A_ptr     )
{
   char               *filename;

   HYPRE_ParCSRMatrix A;

   int                 myid;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/

   if (arg_index < argc)
   {
      filename = argv[arg_index];
   }
   else
   {
      printf("Error: No filename specified \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  FromFile: %s\n", filename);
   }

   /*-----------------------------------------------------------
    * Generate the matrix 
    *-----------------------------------------------------------*/
 
   HYPRE_ParCSRMatrixRead(MPI_COMM_WORLD, filename,&A);

   *A_ptr = A;

   return (0);
}

/*----------------------------------------------------------------------
 * Build standard 7-point laplacian in 3D with grid and anisotropy.
 * Parameters given in command line.
 *----------------------------------------------------------------------*/

int
BuildParLaplacian( int                  argc,
                   char                *argv[],
                   int                  arg_index,
                   HYPRE_ParCSRMatrix  *A_ptr     )
{
   int                 nx, ny, nz;
   int                 P, Q, R;
   double              cx, cy, cz;

   HYPRE_ParCSRMatrix  A;

   int                 num_procs, myid;
   int                 p, q, r;
   double             *values;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Set defaults
    *-----------------------------------------------------------*/
 
   nx = 10;
   ny = 10;
   nz = 10;

   P  = 1;
   Q  = num_procs;
   R  = 1;

   cx = 1.0;
   cy = 1.0;
   cz = 1.0;

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/
   arg_index = 0;
   while (arg_index < argc)
   {
      if ( strcmp(argv[arg_index], "-n") == 0 )
      {
         arg_index++;
         nx = atoi(argv[arg_index++]);
         ny = atoi(argv[arg_index++]);
         nz = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-P") == 0 )
      {
         arg_index++;
         P  = atoi(argv[arg_index++]);
         Q  = atoi(argv[arg_index++]);
         R  = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-c") == 0 )
      {
         arg_index++;
         cx = atof(argv[arg_index++]);
         cy = atof(argv[arg_index++]);
         cz = atof(argv[arg_index++]);
      }
      else
      {
         arg_index++;
      }
   }

   /*-----------------------------------------------------------
    * Check a few things
    *-----------------------------------------------------------*/

   if ((P*Q*R) != num_procs)
   {
      printf("Error: Invalid number of processors or processor topology \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  Laplacian:\n");
      printf("    (nx, ny, nz) = (%d, %d, %d)\n", nx, ny, nz);
      printf("    (Px, Py, Pz) = (%d, %d, %d)\n", P,  Q,  R);
      printf("    (cx, cy, cz) = (%f, %f, %f)\n", cx, cy, cz);
   }

   /*-----------------------------------------------------------
    * Set up the grid structure
    *-----------------------------------------------------------*/

   /* compute p,q,r from P,Q,R and myid */
   p = myid % P;
   q = (( myid - p)/P) % Q;
   r = ( myid - p - P*q)/( P*Q );

   /*-----------------------------------------------------------
    * Generate the matrix 
    *-----------------------------------------------------------*/
 
   values = hypre_CTAlloc(double, 4);

   values[1] = -cx;
   values[2] = -cy;
   values[3] = -cz;

   values[0] = 0.0;
   if (nx > 1)
   {
      values[0] += 2.0*cx;
   }
   if (ny > 1)
   {
      values[0] += 2.0*cy;
   }
   if (nz > 1)
   {
      values[0] += 2.0*cz;
   }

   A = (HYPRE_ParCSRMatrix) GenerateLaplacian(MPI_COMM_WORLD, 
		nx, ny, nz, P, Q, R, p, q, r, values);

   hypre_TFree(values);

   *A_ptr = A;

   return (0);
}

/*----------------------------------------------------------------------
 * Build standard 7-point convection-diffusion operator 
 * Parameters given in command line.
 * Operator:
 *
 *  -cx Dxx - cy Dyy - cz Dzz + ax Dx + ay Dy + az Dz = f
 *
 *----------------------------------------------------------------------*/

int
BuildParDifConv( int                  argc,
                 char                *argv[],
                 int                  arg_index,
                 HYPRE_ParCSRMatrix  *A_ptr     )
{
   int                 nx, ny, nz;
   int                 P, Q, R;
   double              cx, cy, cz;
   double              ax, ay, az;
   double              hinx,hiny,hinz;

   HYPRE_ParCSRMatrix  A;

   int                 num_procs, myid;
   int                 p, q, r;
   double             *values;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Set defaults
    *-----------------------------------------------------------*/
 
   nx = 10;
   ny = 10;
   nz = 10;

   hinx = 1.0/(nx+1);
   hiny = 1.0/(ny+1);
   hinz = 1.0/(nz+1);

   P  = 1;
   Q  = num_procs;
   R  = 1;

   cx = 1.0;
   cy = 1.0;
   cz = 1.0;

   ax = 1.0;
   ay = 1.0;
   az = 1.0;

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/
   arg_index = 0;
   while (arg_index < argc)
   {
      if ( strcmp(argv[arg_index], "-n") == 0 )
      {
         arg_index++;
         nx = atoi(argv[arg_index++]);
         ny = atoi(argv[arg_index++]);
         nz = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-P") == 0 )
      {
         arg_index++;
         P  = atoi(argv[arg_index++]);
         Q  = atoi(argv[arg_index++]);
         R  = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-c") == 0 )
      {
         arg_index++;
         cx = atof(argv[arg_index++]);
         cy = atof(argv[arg_index++]);
         cz = atof(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-a") == 0 )
      {
         arg_index++;
         ax = atof(argv[arg_index++]);
         ay = atof(argv[arg_index++]);
         az = atof(argv[arg_index++]);
      }
      else
      {
         arg_index++;
      }
   }

   /*-----------------------------------------------------------
    * Check a few things
    *-----------------------------------------------------------*/

   if ((P*Q*R) != num_procs)
   {
      printf("Error: Invalid number of processors or processor topology \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  Convection-Diffusion: \n");
      printf("    -cx Dxx - cy Dyy - cz Dzz + ax Dx + ay Dy + az Dz = f\n");  
      printf("    (nx, ny, nz) = (%d, %d, %d)\n", nx, ny, nz);
      printf("    (Px, Py, Pz) = (%d, %d, %d)\n", P,  Q,  R);
      printf("    (cx, cy, cz) = (%f, %f, %f)\n", cx, cy, cz);
      printf("    (ax, ay, az) = (%f, %f, %f)\n", ax, ay, az);
   }

   /*-----------------------------------------------------------
    * Set up the grid structure
    *-----------------------------------------------------------*/

   /* compute p,q,r from P,Q,R and myid */
   p = myid % P;
   q = (( myid - p)/P) % Q;
   r = ( myid - p - P*q)/( P*Q );

   /*-----------------------------------------------------------
    * Generate the matrix 
    *-----------------------------------------------------------*/
 
   values = hypre_CTAlloc(double, 7);

   values[1] = -cx/(hinx*hinx);
   values[2] = -cy/(hiny*hiny);
   values[3] = -cz/(hinz*hinz);
   values[4] = -cx/(hinx*hinx) + ax/hinx;
   values[5] = -cy/(hiny*hiny) + ay/hiny;
   values[6] = -cz/(hinz*hinz) + az/hinz;

   values[0] = 0.0;
   if (nx > 1)
   {
      values[0] += 2.0*cx/(hinx*hinx) - 1.0*ax/hinx;
   }
   if (ny > 1)
   {
      values[0] += 2.0*cy/(hiny*hiny) - 1.0*ay/hiny;
   }
   if (nz > 1)
   {
      values[0] += 2.0*cz/(hinz*hinz) - 1.0*az/hinz;
   }

   A = (HYPRE_ParCSRMatrix) GenerateDifConv(MPI_COMM_WORLD,
                               nx, ny, nz, P, Q, R, p, q, r, values);

   hypre_TFree(values);

   *A_ptr = A;

   return (0);
}

/*----------------------------------------------------------------------
 * Build matrix from one file on Proc. 0. Expects matrix to be in
 * CSR format. Distributes matrix across processors giving each about
 * the same number of rows.
 * Parameters given in command line.
 *----------------------------------------------------------------------*/

int
BuildParFromOneFile( int                  argc,
                     char                *argv[],
                     int                  arg_index,
                     HYPRE_ParCSRMatrix  *A_ptr     )
{
   char               *filename;

   HYPRE_ParCSRMatrix  A;
   HYPRE_CSRMatrix  A_CSR;

   int                 myid;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/

   if (arg_index < argc)
   {
      filename = argv[arg_index];
   }
   else
   {
      printf("Error: No filename specified \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  FromFile: %s\n", filename);

      /*-----------------------------------------------------------
       * Generate the matrix 
       *-----------------------------------------------------------*/
 
      A_CSR = HYPRE_CSRMatrixRead(filename);
   }
   HYPRE_CSRMatrixToParCSRMatrix(MPI_COMM_WORLD, A_CSR, NULL, NULL, &A);

   *A_ptr = A;

   HYPRE_CSRMatrixDestroy(A_CSR);

   return (0);
}

/*----------------------------------------------------------------------
 * Build Rhs from one file on Proc. 0. Distributes vector across processors 
 * giving each about using the distribution of the matrix A.
 *----------------------------------------------------------------------*/

int
BuildRhsParFromOneFile( int                  argc,
                        char                *argv[],
                        int                  arg_index,
                        HYPRE_ParCSRMatrix   A,
                        HYPRE_ParVector     *b_ptr     )
{
   char           *filename;

   HYPRE_ParVector b;
   HYPRE_Vector    b_CSR;

   int             myid;
   int            *partitioning;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/

   if (arg_index < argc)
   {
      filename = argv[arg_index];
   }
   else
   {
      printf("Error: No filename specified \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  Rhs FromFile: %s\n", filename);

      /*-----------------------------------------------------------
       * Generate the matrix 
       *-----------------------------------------------------------*/
 
      b_CSR = HYPRE_VectorRead(filename);
   }
   HYPRE_ParCSRMatrixGetRowPartitioning(A, &partitioning);
   HYPRE_VectorToParVector(MPI_COMM_WORLD, b_CSR, partitioning,&b); 

   *b_ptr = b;

   HYPRE_VectorDestroy(b_CSR);

   return (0);
}

/*----------------------------------------------------------------------
 * Build standard 9-point laplacian in 2D with grid and anisotropy.
 * Parameters given in command line.
 *----------------------------------------------------------------------*/

int
BuildParLaplacian9pt( int                  argc,
                      char                *argv[],
                      int                  arg_index,
                      HYPRE_ParCSRMatrix  *A_ptr     )
{
   int                 nx, ny;
   int                 P, Q;

   HYPRE_ParCSRMatrix  A;

   int                 num_procs, myid;
   int                 p, q;
   double             *values;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Set defaults
    *-----------------------------------------------------------*/
 
   nx = 10;
   ny = 10;

   P  = 1;
   Q  = num_procs;

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/
   arg_index = 0;
   while (arg_index < argc)
   {
      if ( strcmp(argv[arg_index], "-n") == 0 )
      {
         arg_index++;
         nx = atoi(argv[arg_index++]);
         ny = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-P") == 0 )
      {
         arg_index++;
         P  = atoi(argv[arg_index++]);
         Q  = atoi(argv[arg_index++]);
      }
      else
      {
         arg_index++;
      }
   }

   /*-----------------------------------------------------------
    * Check a few things
    *-----------------------------------------------------------*/

   if ((P*Q) != num_procs)
   {
      printf("Error: Invalid number of processors or processor topology \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  Laplacian 9pt:\n");
      printf("    (nx, ny) = (%d, %d)\n", nx, ny);
      printf("    (Px, Py) = (%d, %d)\n", P,  Q);
   }

   /*-----------------------------------------------------------
    * Set up the grid structure
    *-----------------------------------------------------------*/

   /* compute p,q from P,Q and myid */
   p = myid % P;
   q = ( myid - p)/P;

   /*-----------------------------------------------------------
    * Generate the matrix 
    *-----------------------------------------------------------*/
 
   values = hypre_CTAlloc(double, 2);

   values[1] = -1.0;

   values[0] = 0.0;
   if (nx > 1)
   {
      values[0] += 2.0;
   }
   if (ny > 1)
   {
      values[0] += 2.0;
   }
   if (nx > 1 && ny > 1)
   {
      values[0] += 4.0;
   }

   A = (HYPRE_ParCSRMatrix) GenerateLaplacian9pt(MPI_COMM_WORLD,
                                  nx, ny, P, Q, p, q, values);

   hypre_TFree(values);

   *A_ptr = A;

   return (0);
}
/*----------------------------------------------------------------------
 * Build 27-point laplacian in 3D, 
 * Parameters given in command line.
 *----------------------------------------------------------------------*/

int
BuildParLaplacian27pt( int                  argc,
                       char                *argv[],
                       int                  arg_index,
                       HYPRE_ParCSRMatrix  *A_ptr     )
{
   int                 nx, ny, nz;
   int                 P, Q, R;

   HYPRE_ParCSRMatrix  A;

   int                 num_procs, myid;
   int                 p, q, r;
   double             *values;

   /*-----------------------------------------------------------
    * Initialize some stuff
    *-----------------------------------------------------------*/

   MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
   MPI_Comm_rank(MPI_COMM_WORLD, &myid );

   /*-----------------------------------------------------------
    * Set defaults
    *-----------------------------------------------------------*/
 
   nx = 10;
   ny = 10;
   nz = 10;

   P  = 1;
   Q  = num_procs;
   R  = 1;

   /*-----------------------------------------------------------
    * Parse command line
    *-----------------------------------------------------------*/
   arg_index = 0;
   while (arg_index < argc)
   {
      if ( strcmp(argv[arg_index], "-n") == 0 )
      {
         arg_index++;
         nx = atoi(argv[arg_index++]);
         ny = atoi(argv[arg_index++]);
         nz = atoi(argv[arg_index++]);
      }
      else if ( strcmp(argv[arg_index], "-P") == 0 )
      {
         arg_index++;
         P  = atoi(argv[arg_index++]);
         Q  = atoi(argv[arg_index++]);
         R  = atoi(argv[arg_index++]);
      }
      else
      {
         arg_index++;
      }
   }

   /*-----------------------------------------------------------
    * Check a few things
    *-----------------------------------------------------------*/

   if ((P*Q*R) != num_procs)
   {
      printf("Error: Invalid number of processors or processor topology \n");
      exit(1);
   }

   /*-----------------------------------------------------------
    * Print driver parameters
    *-----------------------------------------------------------*/
 
   if (myid == 0)
   {
      printf("  Laplacian_27pt:\n");
      printf("    (nx, ny, nz) = (%d, %d, %d)\n", nx, ny, nz);
      printf("    (Px, Py, Pz) = (%d, %d, %d)\n", P,  Q,  R);
   }

   /*-----------------------------------------------------------
    * Set up the grid structure
    *-----------------------------------------------------------*/

   /* compute p,q,r from P,Q,R and myid */
   p = myid % P;
   q = (( myid - p)/P) % Q;
   r = ( myid - p - P*q)/( P*Q );

   /*-----------------------------------------------------------
    * Generate the matrix 
    *-----------------------------------------------------------*/
 
   values = hypre_CTAlloc(double, 2);

   values[0] = 26.0;
   if (nx == 1 || ny == 1 || nz == 1)
	values[0] = 8.0;
   if (nx*ny == 1 || nx*nz == 1 || ny*nz == 1)
	values[0] = 2.0;
   values[1] = -1.0;

   A = (HYPRE_ParCSRMatrix) GenerateLaplacian27pt(MPI_COMM_WORLD,
                               nx, ny, nz, P, Q, R, p, q, r, values);

   hypre_TFree(values);

   *A_ptr = A;

   return (0);
}
