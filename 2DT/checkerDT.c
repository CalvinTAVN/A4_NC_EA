/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dynarray.h"
#include "checkerDT.h"


/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T n) {
   Node_T parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   /*const char* compareParent;*/
   size_t i;

   /* Sample check: a NULL pointer is not a valid node */
   if(n == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }
 
   parent = Node_getParent(n);
   if(parent != NULL)
   {
      npath = Node_getPath(n);
      /* Sample check that parent's path must be prefix of n's path */
      ppath = Node_getPath(parent);
      
      /* dtBad2*/
      if (ppath == NULL)
      {
         fprintf(stderr, "parents path is NULL\n");
         return FALSE;
      }

      /* changed*/
      if(strncmp(Node_getPath(n), Node_getPath(parent), strlen(Node_getPath(parent))) != 0)
      {                                       
         fprintf(stderr, "child's path is not the same as parent's path");                                                               
         return FALSE;                                                                                                                   
      }
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* Sample check that n's path after parent's path + '/'
         must have no further '/' characters */
      /* check here for dtBad4*/
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL)
      {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at n.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T n) {
   size_t c;

   size_t i;

   Node_T p;
   
   if(n != NULL) {

      /* Sample check on each non-root node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(n))
         return FALSE;


      for(c = 0; c < Node_getNumChildren(n); c++)
      {
         Node_T child = Node_getChild(n, c);

         /*dtBad3*/
         /*checking if child nodes are in lexicographic*/
            if (Node_getNumChildren(n) > 1){
               for (i = 0; i < Node_getNumChildren(n) - 1; i++)
               {
                  p = Node_getChild(n, i);
                  if(strcmp(Node_getPath(p), Node_getPath(Node_getChild(n, i + 1))) > 1){
                     fprintf(stderr, "Tree is not in lexicographical order");
                     return FALSE;
                  }
               }
            }
         

            /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(child))
            return FALSE;
      }

      
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean isInit, Node_T root, size_t count) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!isInit)
   {
         if(count != 0)
         {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
         }
   }
   /* changed, if isInit, countis 0 but root is not, return false*/
   if (isInit)
   {
      if (count == 0)
      {
         if (root != NULL)
         {
            fprintf(stderr, "root is not NULL when count is 0\n");
            return FALSE;
         }
      }

      /* dtBad5*/
      if (root == NULL)
      {
         if (count != 0)
         {
            fprintf(stderr, "count is not equal to 0 when root is NULL\n" );
            return FALSE;
         }
      }

      /*dtBad4*/
      if (root != NULL)
      {
         if (Node_getParent(root) != NULL)
         {
            fprintf(stderr, "Root's parent is not NULL\n");
            return FALSE;
         }
      }
   }


   
   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(root);
}
