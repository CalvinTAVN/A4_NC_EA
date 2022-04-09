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


/*
   Starting at the parameter curr, traverses as far down
   the hierarchy as possible while still matching the path
   parameter.
   Returns a pointer to the farthest matching node down that path,
   or NULL if there is no node in curr's hierarchy that matches
   a prefix of the path
*/
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
         found = FT_traversePathFrom(path,
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
static Node_T FT_traversePath(char* path) 
{
   assert(path != NULL);
   return FT_traversePathFrom(path, root);
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
static int FT_linkParentToChild(Node_T parent, Node_T child) {

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
    if (strstr(dirToken, "/" ) == NULL)
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
         result = FT_linkParentToChild(curr, new);
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
      result = FT_linkParentToChild(parent, firstNew);
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

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   curr = FT_traversePath(path);

   if (checkIsFile(curr) == TRUE)
   {
       return NOT_A_DIRECTORY;
   }

   result = FT_insertRestOfPath(path, curr, FALSE, NULL, 0);
   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   return result;
}

/* see ft.h for specification */
boolean FT_containsDir(char* path) {
   Node_T curr;
   boolean result;

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   assert(path != NULL);

   if(!isInitialized)
      return FALSE;

   curr = FT_traversePath(path);

   if(curr == NULL)
      result = FALSE;
   else if (checkIsFile(curr) == FALSE)
      return FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else
      result = TRUE;

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   return result;
}

/*
  Removes the directory hierarchy rooted at path starting from
  curr. If curr is the data structure's root, root becomes NULL.
  Returns NO_SUCH_PATH if curr is not the node for path,
  and SUCCESS otherwise.
 */
static int FT_rmDirAt(char* path, Node_T curr) 
{

   Node_T parent;

   assert(path != NULL);
   assert(curr != NULL);
   if(checkIsFile(curr) == TRUE)
        return NOT_A_DIRECTORY;

   parent = Node_getParent(curr);

   if(!strcmp(path,Node_getPath(curr))) {
      if(parent == NULL)
         root = NULL;
      else
         Node_unlinkChild(parent, curr);

      FT_removePathFrom(curr);

      return SUCCESS;
   }
   else
      return NO_SUCH_PATH;

}

/* see ft.h for specification */
int FT_rmDir(char* path) 
{
   Node_T curr;
   int result;

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   curr = FT_traversePath(path);
   if(checkIsFile(curr) == TRUE)
        return NOT_A_DIRECTORY;

   if(curr == NULL)
      result =  NO_SUCH_PATH;
   else
      result = FT_rmDirAt(path, curr);

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   return result;
}

/* see ft.h for specification*/
int FT_insertFile(char *path, void *contents, size_t length)
{
    Node_T curr;
   int result;

   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   assert(path != NULL);
   
   if(!isInitialized)
      return INITIALIZATION_ERROR;
   curr = FT_traversePath(path);

   if (curr == NULL)
   {
      return CONFLICTING_PATH;
   }
   if (checkIsFile(curr) == TRUE)
   {
       return NOT_A_DIRECTORY;
   }

   result = FT_insertRestOfPath(path, curr, TRUE, contents, length);
   /* assert(CheckerDT_isValid(isInitialized,root,count)); */
   return result;
}


/*
  Returns TRUE if the tree contains the full path parameter as a
  file and FALSE otherwise.
*/
boolean FT_containsFile(char *path)
{
    Node_T curr;
   boolean result;

   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   assert(path != NULL);

   if(!isInitialized)
      return FALSE;

   curr = FT_traversePath(path);


   if(curr == NULL)
      result = FALSE;
   else if (checkIsFile(curr) != TRUE)
      return FALSE;
   else if(strcmp(path, Node_getPath(curr)))
      result = FALSE;
   else
      result = TRUE;

   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   return result;
}

/*
  Removes the FILE rooted at path starting from
  curr. If curr is the data structure's root, return 
  NOT_A_FILE Returns NO_SUCH_PATH if curr is not the node for path,
  and SUCCESS otherwise.
  if File 
 */
static int FT_rmFileAt(char* path, Node_T curr) {

   Node_T parent;

   assert(path != NULL);
   assert(curr != NULL);

   parent = Node_getParent(curr);
   assert(checkIsFile(parent) == FALSE);
   
   /* if the node to be removed is a directory,
      its not afile*/
   if (checkIsFile(curr) == FALSE)
   {
       return NOT_A_FILE;
   }
   if(!strcmp(path,Node_getPath(curr))) 
   {
      Node_unlinkChild(parent, curr);

      FT_removePathFrom(curr);

      return SUCCESS;
   }
   else
      return NO_SUCH_PATH;
}

/*
  Removes the FT file at path.
  Returns SUCCESS if found and removed.
  Returns INITIALIZATION_ERROR if not in an initialized state.
  Returns NOT_A_FILE if path exists but is a directory not a file.
  Returns NO_SUCH_PATH if the path does not exist in the hierarchy.
*/
int FT_rmFile(char *path)
{
   Node_T curr;
   int result;

   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   curr = FT_traversePath(path);
   if(curr == NULL)
      result =  NO_SUCH_PATH;
   else
      result = FT_rmFileAt(path, curr);

   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   return result;   
}

/*
  Returns the contents of the file at the full path parameter.
  Returns NULL if the path does not exist or is a directory.
  Note: checking for a non-NULL return is not an appropriate
  contains check -- the contents of a file may be NULL.
*/
void *FT_getFileContents(char *path)
{
    Node_T curr;
    
    assert(path != NULL);

    curr = FT_traversePath(path);
    if (curr == NULL)
    {
        return NULL;
    }
    if (checkIsFile(curr) == FALSE)
    {
        return NULL;
    }
    return Node_getContents(curr);
}

/*
  Replaces current contents of the file at the full path parameter with
  the parameter newContents of size newLength.
  Returns the old contents if successful. (Note: contents may be NULL.)
  Returns NULL if the path does not already exist or is a directory.
*/
void *FT_replaceFileContents(char *path, void *newContents, size_t newLength)
{
    Node_T curr;

    assert(path != NULL);
    assert(newContents != NULL);

    curr = FT_traversePath(path);
    if (curr == NULL)
    {
        return NULL;
    }
    if (checkIsFile(curr) == FALSE)
    {
        return NULL;
    }
    return Node_populateContents(curr, newContents, newLength);
}

/*
  Returns SUCCESS if path exists in the hierarchy,
  returns NO_SUCH_PATH if it does not, and
  returns INITIALIZATION_ERROR if the structure is not initialized.
  When returning SUCCESS,
  if path is a directory: *type is set to FALSE, *length is unchanged
  if path is a file: *type is set to TRUE, and
                     *length is set to the length of file's contents.
  When returning a non-SUCCESS status, *type and *length are unchanged.
 */
int FT_stat(char *path, boolean *type, size_t *length)
{
    Node_T curr;

    assert(path != NULL);
    assert(type != NULL);
    assert(length != NULL);

    if (!isInitialized)
    {
        return INITIALIZATION_ERROR;
    }

    curr = FT_traversePath(path);
    if (curr == NULL)
    {
        return NO_SUCH_PATH;
    }

    *type = checkIsFile(curr);
    *length = Node_getFileLength(curr);
    return SUCCESS;
}

/* see dt.h for specification */
int FT_init(void) 
{
   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   if(isInitialized)
      return INITIALIZATION_ERROR;
   isInitialized = 1;
   root = NULL;
   count = 0;
   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   return SUCCESS;
}

/* see dt.h for specification */
int FT_destroy(void) 
{
   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   if(!isInitialized)
      return INITIALIZATION_ERROR;

   FT_removePathFrom(root);
   root = NULL;
   isInitialized = 0;
   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   return SUCCESS;
}

/*
   Performs a pre-order traversal of the tree rooted at n,
   inserting each payload to DynArray_T d beginning at index i.
   Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) 
   {
      (void) DynArray_set(d, i, Node_getPath(n));
      i++;
      /* Should only be recursive if it is a directory*/
      if (checkIsFile(n) == FALSE)
      {
        for(c = 0; c < Node_getNumChildren(n); c++)
            i = FT_preOrderTraversal(Node_getChild(n, c), d, i);
      }
   }
   return i;
}

/*
   Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length.
*/
static void FT_strlenAccumulate(char* str, size_t* pAcc) 
{
   assert(pAcc != NULL);

   if(str != NULL)
      *pAcc += (strlen(str) + 1);
}

/*
   Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string.
*/
static void FT_strcatAccumulate(char* str, char* acc) 
{
   assert(acc != NULL);

   if(str != NULL)
      strcat(acc, str); strcat(acc, "\n");
}


/* see dt.h for specification */
char* FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   /*assert(CheckerDT_isValid(isInitialized,root,count));*/

   if(!isInitialized)
      return NULL;

   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate, (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      /*assert(CheckerDT_isValid(isInitialized,root,count));*/
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate, (void *) result);

   DynArray_free(nodes);
   /*assert(CheckerDT_isValid(isInitialized,root,count));*/
   return result;
}
