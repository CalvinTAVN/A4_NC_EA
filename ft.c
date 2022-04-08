#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "node.h"
#include "a4def.h"

/* A Directory Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to the root node in the hierarchy */
static Node_T root;
/* a counter of the number of nodes in the hierarchy */
static size_t count;

/* A counter for the number of file nodes in the hierarchy*/
static size_t fileCount;

/* a counter for the number of directory nodes in the hierarchy*/
static size_t directoryCount;


