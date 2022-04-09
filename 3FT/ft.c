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

static Node_T FT_traversePathFrom(char* path, Node_T curr) {
   Node_T found;
   size_t i;

   assert(path != NULL);

   if(curr == NULL)
      return NULL;

   else if(!strcmp(path,Node_getPath(curr)))
      return curr;
   
   else if(checkIsFile(curr) == TRUE)
        return NULL;

   else if(!strncmp(path, Node_getPath(curr), strlen(Node_getPath(curr)))) {
      for(i = 0; i < Node_getNumChildren(curr); i++) {
         found = DT_traversePathFrom(path,
                                Node_getChild(curr, i));
         if(found != NULL)
            return found;
      }
      return curr;
   }
   return NULL;
}

/*
   Returns the farthest node reachable from the root following a given
   path, or NULL if there is no node in the hierarchy that matches a
   prefix of the path.
*/
static Node_T DT_traversePath(char* path) 
{
   assert(path != NULL);
   return DT_traversePathFrom(path, root);
}

/*
   Destroys the entire hierarchy of nodes rooted at curr,
   including curr itself.
*/
static void FT_removePathFrom(Node_T curr) 
{
   if(curr != NULL) {
      count -= Node_destroy(curr);
   }
}

/*
   Given a prospective parent and child node,
   adds child to parent's children list, if possible
   If not possible, destroys the hierarchy rooted at child
   and returns PARENT_CHILD_ERROR, otherwise, returns SUCCESS.
*/
static int FT_linkParentToChild(Node_T parent, Node_T child, ) {

   assert(parent != NULL);
   assert(checkIsFile(parent) == FALSE);

   if(Node_linkChild(parent, child) != SUCCESS) {
      (void) Node_destroy(child);
      return PARENT_CHILD_ERROR;
   }

   return SUCCESS;
}

/*
   Inserts a new path into the tree rooted at parent, or, if
   parent is NULL, as the root of the data structure.
   If a node representing path already exists, returns ALREADY_IN_TREE
   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR
   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR
   Otherwise, returns SUCCESS

   Note* the last 3 parameters determines if the last node in the
   path is a file or a directory, all preceding nodes are automatically
   directories.
*/
static int FT_insertRestOfPath(char* path, Node_T parent, boolean isFile, void *contents, size_t fileLength) {

   Node_T curr = parent;
   Node_T firstNew = NULL;
   Node_T new;
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   int result;
   size_t newCount = 0;

   assert(path != NULL);
   if (parent != NULL)
   {
       assert(checkIsFile(parent) == FALSE);
   }

   if(curr == NULL) 
   {
      if(root != NULL) 
      {
        return CONFLICTING_PATH;
      }
   }
   else 
   {
      if(!strcmp(path, Node_getPath(curr)))
      {
        return ALREADY_IN_TREE;
      }
        

      restPath += (strlen(Node_getPath(curr)) + 1);
   }

   copyPath = malloc(strlen(restPath)+1);
   if(copyPath == NULL)
      return MEMORY_ERROR;
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");

   while(dirToken != NULL) 
   {
    /* Checks if the token is the last token*/
    if (strstr(dirToken, '/' ) == NULL)
    {
        /* if the last token is a file,
           create it as a file and allocate
           its contents*/
        if (isFile)
        {
            new = Node_create(dirToken, curr, isFile);
            if (new == NULL)
            {
                (void)Node_destroy(firstNew);
            }
            (void)Node_populateContents(new, contents, fileLength);
        }
    }
    else
    {
      new = Node_create(dirToken, curr, FALSE);
    }

      if(new == NULL) {
         if(firstNew != NULL)
            (void) Node_destroy(firstNew);
         free(copyPath);
         return MEMORY_ERROR;
      }

      newCount++;

      if(firstNew == NULL)
         firstNew = new;
      else {
         result = DT_linkParentToChild(curr, new);
         if(result != SUCCESS) {
            (void) Node_destroy(new);
            (void) Node_destroy(firstNew);
            free(copyPath);
            return result;
         }
      }

      curr = new;
      dirToken = strtok(NULL, "/");
   }

   free(copyPath);

   if(parent == NULL) {
      root = firstNew;
      count = newCount;
      return SUCCESS;
   }
   else {
      result = DT_linkParentToChild(parent, firstNew);
      if(result == SUCCESS)
         count += newCount;
      else
         (void) Node_destroy(firstNew);

      return result;
   }
}

/* see ft.h for specification */
int FT_insertDir(char* path) {

   Node_T curr;
   int result;

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   assert(path != NULL);
   assert(path->isFile == FALSE);

   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = DT_traversePath(path);
   result = DT_insertRestOfPath(path, curr, FALSE, NULL, NULL);
   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   return result;
}




