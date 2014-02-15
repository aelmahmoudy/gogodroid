/*
-----------------------------------------------------------------------------
 $Id: xmlparse.c,v 1.1 2009/11/20 16:53:43 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2005 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#define _XMLParse_
#include "xmlparse.h"

#ifndef XML_DEBUG
#define XML_DEBUG 0
#endif

int debug = XML_DEBUG;

static tNode * findNode(char *name, tNode nodes[])
{
  tNode *n;

  n = nodes;

  while (n->name[0]) {
    if (strcmp(name, n->name) == 0) return n;
    n++;
  }

  return 0;
}

static tAttribute *findAttribute(char *name, tAttribute attributes[])
{
  tAttribute *a;

  a = attributes;

  while (a->name[0]) {
    if (strcmp(name, a->name) == 0) return a;
    a++;
  }

  return 0;
}

static int SkipBlanks(char *str, int pos)
{
  while (str[pos] && 
	 ((str[pos] == ' ') || (str[pos] == '\t') || (str[pos] == '\r') || (str[pos] == '\n'))) {
    pos += 1;
  }

  return pos;
}

/*
 * return values:
 *
 *    0 : Success
 *    n : Parsing error (position in the string where the parsing error occured)
 */

int XMLParse(char *str, tNode nodes[])
{
  char       *string;
  int         pos;
  int         simple;  /* 1 = complete node in a single <> like <name ..../> */
  char       *tagName;
  char       *attrName;
  char       *attrValue;
  char       *endTag;
  tNode      *n;
  tAttribute *a;
  char        endToken[100];
  int         res;
  
  if (debug) printf("Beginning of XMLParse\n");
  
  string = (char *) malloc(strlen(str) + 1);
  
  if (!string) {
    fprintf(stderr, "xmlparse: Error malloc\n");
    return -1;
  }
  
  strcpy(string, str);
  
  pos = 0;

  while (string[pos]) {

    simple = 0;
  
    pos = SkipBlanks(string, pos);

    /*
     * If not at the beginning of a node tag, return with current position
     */
  
    if (string[pos] == 0) { free(string); return 0; }
  
    if (string[pos] != '<') { free(string); return pos; }
  
    pos += 1;
  
    /*
     * We are not expecting here the end node (like </...> )
     */
  
    if (!isalpha(string[pos])) { free(string); return pos; }
  
    /*
     * We now retrieve the node name and the attributes.
     * It must be ended by a space when there is some attributes,
     * a '/' if it's a simple node, or '>'.
     */
  
    tagName = &string[pos];
    while (string[pos] && (isalnum(string[pos]) || (string[pos] == '_'))) {
      pos += 1;
    }
  
    if (!string[pos]) { free(string); return pos; }
  
    if (string[pos] == '/') {
  
      simple      = 1;
      string[pos] = 0;
      pos        += 1;

      if (string[pos] != '>') { free(string); return pos; }

      string[pos] = 0;

    } else if (string[pos] == ' ') {
  
      string[pos]  = 0;
      pos         += 1;
  
    } else if (string[pos] == '>') {
  
      string[pos] = 0; /* we do not advance pos as it will indicate if '>' found */

    } else {

      free(string);
      return pos;
    }
  
    /*
     * The nodename must at least contain one character
     */
     
    if (!tagName[0]) { free(string); return pos; }

    if (debug) printf("tagName = %s\n", tagName);
        
    /*
     * Here we try to find the node structure corresponding to the
     * tag read from the xml containt
     */
  
    if ((n = findNode(tagName, nodes)) != NULL) {
  
      /*
       * We found the node structure...
       */
  
      if (debug) printf("Found corresponding structure: %s\n", n->name);

      /*
       * A null character indicate that there is no attribute available
       */
      
      if (string[pos] == 0) {

        pos += 1;

      } else {
      
        /*
         * There may be some attributes. Parse them...
         */

        pos = SkipBlanks(string, pos);

        while (isalpha(string[pos])) {

          attrName = &string[pos];

          while (string[pos] && 
		 (isalnum(string[pos]) || (string[pos] == '_'))) pos += 1;

          if (string[pos] != '=') { free(string); return pos; }

          string[pos] = 0;
          pos        += 1;

          a = findAttribute(attrName, n->attributes);

          if (debug) printf("Attribute %sfound in structure: %s", a == NULL ? "not " : "", attrName);
          
          if (string[pos] != '"') { free(string); return pos; }

          pos += 1;

          attrValue = &string[pos];

          while (string[pos] && 
		 (string[pos] != '\n') &&
		 (string[pos] != '\r') &&
		 (string[pos] != '>' ) &&
		 (string[pos] != '"' )) pos += 1;

          if (string[pos] != '"') { free(string); return pos; }
            
	  string[pos] = 0;
          pos        += 1;
          
          if (a != NULL) {
	    a->value = attrValue;
	  }
      
          pos = SkipBlanks(string, pos);
	}

        if (string[pos] == '/') {

          simple  = 1;
          pos    += 1;
        }

        if (string[pos] == '>') {

          pos += 1;

        } else {

          free(string);
          return pos;
	}
      }

    } else {
  
      /*
       * There is no node structure entry corresponding to the tag found
       * we just skip the contain of the node up to the end of it.
       */
  
      if (debug) printf("No structure definition: %s\n", tagName);

      if (string[pos] == 0) {

        pos += 1;

      } else {

        while (string[pos] && (string[pos] != '>')) pos += 1;
        if (string[pos] == '>') {
          pos += 1;
        }
      }
  
      if ((string[pos] == 0) && !simple) { free(string); return pos; }
    }
    
    if (simple) {
      
      /*
       * Call the processing function, if it exists...
       */
  
      if (n && n->p) {
        res = (*n->p)(n, &string[pos]);
        if (res) {
          free(string);
	  if (res > 0) res += pos;
	  return res;
	}
      }
      
    } else {

      strcpy(endToken, "</");
      strcat(endToken, tagName);
      strcat(endToken, ">");
  
      endTag = strstr(&string[pos], endToken);
    
      if (debug) printf("Computed end tag: %s\n", endToken);
    
      if (endTag) {
      
        if (debug) printf("Found node end: %s\n", endToken);
      
        *endTag = 0;
    
        /*
         * Call the processing function, if it exists...
         */
  
        if (n && n->p) {
	  res = (*n->p)(n, &string[pos]);
	  if (res) {
	    free(string);
	    if (res > 0) res += pos;
	    return res;
	  }
	}
       
        pos = (int)(endTag - string) + (int)strlen(endToken);
      
      } else {
      
        free(string);
        return pos;
      }

    }
  
    //return 0;
  }
  
  free(string);
  
  return 0;
}

/*----- xmlparse.c --------------------------------------------------------------------*/

