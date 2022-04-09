/*--------------------------------------------------------------------*/
/* node.c                                                             */
/* Author: Calvin and Alex                                            */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "node.h"
#include "a4def.h"

/*
   A node structure represents a directory in the directory tree
*/
struct node {
   /* the full path of this directory */
   char* path;

   /* the parent directory of this directory
      NULL for the root of the directory tree */
   Node_T parent;

   /* the subdirectories of this directory
      stored in sorted order by pathname */
   DynArray_T children;

   /* boolean isFile that determines if the node is a
      file or directory*/
  boolean isFile;

  /* void * that contains the pointer to the contents
     if the node is a file*/
  void *contents;

  /* size_t that contains the length in char of the file*/
  /* will be 0 to start*/
  size_t fileLength;
};

/*
  returns a path with contents
  n->path/dir
  or NULL if there is an allocation error.
  Allocates memory for the returned string,
  which is then owned by the caller!
*/
static char* Node_buildPath(Node_T n, const char* dir) {
   /* char that represents what n's path will be*/
   char* path;

   assert(dir != NULL);

   /* for root node, else, it adds the length of the path of the parent
      with the size of the str of dir*/
   if(n == NULL)
      path = malloc(strlen(dir)+1);
   else
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);

   if(path == NULL)
      return NULL;
   *path = '\0';

   if(n != NULL) {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}

/* see node.h for specification */
Node_T Node_create(const char* dir, Node_T parent, boolean isFile){
   /* variable that represents the new node*/
   Node_T new;

   /* parent can be null if its a root, and not NULL if its a child*/
   
   assert(dir != NULL);

   new = malloc(sizeof(struct node));
   if(new == NULL) {
      return NULL;
   }

   new->path = Node_buildPath(parent, dir);

   if(new->path == NULL) {
      free(new);
      return NULL;
   }

   new->parent = parent;

   /* if the new node to be created is a File, then set the booleanisFile to true,
      set the contents and fileLength initially to 0, and children permamently
      to null, conents and fileLength is populated by function Node_populateContents

      if its a  directory, set all file related variables to NULL and 0
      for fileLength and children to
      DynArray_new(0)*/
   if (isFile)
   {
     new->isFile = isFile; /*TRUE*/
     new->contents = NULL;
     new->fileLength = 0;
     new->children = NULL;
   }
   else
   {
      new->isFile = FALSE;
      new->contents = NULL;
      new->fileLength = 0;
      new->children = DynArray_new(0);
      if(new->children == NULL)
      {
        free(new->path);
        free(new);
        return NULL;
      }

   }
   return new;
}

/* see node.h for specification*/
void *Node_populateContents(Node_T fileNode, void *newContent, size_t newLength)
{
   /* variable that contains the old contents of fileNode*/
  void *oldContents;


  assert(fileNode != NULL);
  assert(fileNode->isFile == TRUE);


  /* Leaves the freeing of old space to the client*/
  oldContents = fileNode->contents;
  fileNode->contents = newContent;
  fileNode->fileLength = newLength;

  return oldContents;
}

/* see node.h for specification */
size_t Node_destroy(Node_T n) {
   /* variable used to traverse the children*/
   size_t i;
   /* counts the number of nodes destroyed*/
   size_t count = 0;
   /* recursived node of children*/
   Node_T c;

   assert(n != NULL);

   /* destroys all children nodes of n if n is not a file */
   if(n->isFile == FALSE)
   {
       for(i = 0; i < DynArray_getLength(n->children); i++)
       {
            c = DynArray_get(n->children, i);
            count += Node_destroy(c);
       }
       DynArray_free(n->children);
   }

   free(n->path);
   free(n);
   count++;

   return count;
}

/*returns the path of node n*/
const char* Node_getPath(Node_T n)
{
   assert(n != NULL);
   return n->path;
}

/* see node.h for specification */
int Node_compare(Node_T node1, Node_T node2)
{
   assert(node1 != NULL);
   assert(node2 != NULL);
   /* if node1 is a file and node2 is a directory,
      automatically prioritize the file,
      else if node1 is a directory and node2 is a
      file, prioritize file again*/
   if ((node1->isFile == TRUE) && (node2->isFile == FALSE) )
   {
      return -1;
   }
   else if ((node1->isFile == FALSE)  && (node2->isFile == TRUE))
   {
      return 1;
   }

   return strcmp(node1->path, node2->path);
}

/* see node.h for specification */
size_t Node_getNumChildren(Node_T n)
{
   assert(n != NULL);
   assert(n->isFile == FALSE);
   assert(n->contents == NULL);
   assert(n->fileLength == 0);

   return DynArray_getLength(n->children);
}

/* see node.h for specification */
int Node_hasChild(Node_T n, const char* path, size_t* childID)
{
   /* index at which the child is located
      in the index*/
   size_t index;
   /* variable that represents the success of the function*/
   int result;
   /* temporary node to check upon*/
   Node_T checker;

   assert(n != NULL);
   assert(path != NULL);
   assert(n->isFile == FALSE);
   assert(n->contents == NULL);
   assert(n->fileLength == 0);

   /* we are assuming the child is a directory, but it doesn't
      matter since it compares the paths*/
   checker = Node_create(path, NULL, FALSE);
   if(checker == NULL) {
      return -1;
   }

   result = DynArray_bsearch(n->children, checker, &index,
                    (int (*)(const void*, const void*)) Node_compare);
   (void) Node_destroy(checker);

   if(childID != NULL)
      *childID = index;

   return result;
}

/* see node.h for specification */
Node_T Node_getChild(Node_T n, size_t childID) {
   assert(n != NULL);
   assert(n->isFile == FALSE);
   assert(n->contents == NULL);
   assert(n->fileLength == 0);

   if(DynArray_getLength(n->children) > childID) {
      return DynArray_get(n->children, childID);
   }
   else {
      return NULL;
   }
}

/* see node.h for specification */
Node_T Node_getParent(Node_T n) {
   assert(n != NULL);

   return n->parent;
}


/* see node.h for specification */
int Node_linkChild(Node_T parent, Node_T child) {
   /* index of the specified child in dynArray*/
   size_t i;
   /* Variable that represents the success of the function*/
   int result;
   /* variable that represents the rest of the pathway
      apart from parent*/
   char* rest;

   assert(parent != NULL);
   assert(parent->isFile == FALSE);
   assert(parent->contents == NULL);
   assert(parent->fileLength == 0);

   assert(child != NULL);

   result = Node_hasChild(parent, child->path, NULL);
   if(result == 1)
   {
      return ALREADY_IN_TREE;
   }
   if(result == -1)
   {
      return PARENT_CHILD_ERROR;
   }

   i = strlen(parent->path);

   if(strncmp(child->path, parent->path, i))
   {
      return PARENT_CHILD_ERROR;
   }
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/')
   {
      return PARENT_CHILD_ERROR;
   }

   rest++;

   if(strstr(rest, "/") != NULL)
   {
      return PARENT_CHILD_ERROR;
   }
   child->parent = parent;

   if(DynArray_bsearch(parent->children, child, &i,
         (int (*)(const void*, const void*)) Node_compare) == 1)
   {
      return ALREADY_IN_TREE;
   }

   if(DynArray_addAt(parent->children, i, child) == TRUE)
   {
      return SUCCESS;
   }
   else
   {
      return PARENT_CHILD_ERROR;
   }
}

/* see node.h for specification */
int  Node_unlinkChild(Node_T parent, Node_T child) {
   /* Index at which to remove child in dynarray*/
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);

   if(DynArray_bsearch(parent->children, child, &i,
         (int (*)(const void*, const void*)) Node_compare) == 0)
   {
      return PARENT_CHILD_ERROR;
   }

   (void) DynArray_removeAt(parent->children, i);

   return SUCCESS;
}

/* see node.h for specification */
int Node_addChild(Node_T parent, const char* dir, boolean isFile, void *inputtedContents, size_t lengthOfFile) {
   /* Node that represents the child to be removed*/
   Node_T new;
   /* variable that represents the success of the function*/
   int result;

   assert(parent != NULL);
   assert(dir != NULL);

   new = Node_create(dir, parent, isFile);
   if(new == NULL)
   {
      return PARENT_CHILD_ERROR;
   }
   if ((isFile) && (inputtedContents != NULL))
   {
     (void)Node_populateContents(new, inputtedContents, lengthOfFile);
   }


   result = Node_linkChild(parent, new);
   if(result != SUCCESS)
      (void) Node_destroy(new);
   return result;
}


/* see node.h for specification */
char* Node_toString(Node_T n) {
   /* char that represents the 
      lexicographic representation
       of the node at its children*/
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL) {
      return NULL;
   }
   else {
      return strcpy(copyPath, n->path);
   }
}

/* returns the boolean value*/
boolean checkIsFile(Node_T n)
{
   return n->isFile;
}

/* returns the contents of a file n*/
void * Node_getContents(Node_T n)
{
   assert(checkIsFile(n) == TRUE);
   return n->contents;
}

/* if the node is a file, get its length*/
size_t Node_getFileLength(Node_T n)
{
   return n->fileLength;
}
