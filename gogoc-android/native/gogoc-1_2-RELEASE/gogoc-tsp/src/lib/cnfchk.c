/*
---------------------------------------------------------------------------
 $Id: cnfchk.c,v 1.1 2009/11/20 16:53:36 jasminko Exp $
---------------------------------------------------------------------------
  Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
---------------------------------------------------------------------------
*/

#include "platform.h"

#include "cnfchk.h"
#include "config.h"
#include "log.h"
#include "hex_strings.h"

char *template_mappings[] = {	"freebsd44", "freebsd", "freebsd4", "freebsd", "windows2000", "windows",
								"windows2003", "windows", "windowsXP", "windows",
								"solaris8", "solaris", NULL };

static int readfixwrite(char *, char *, char *, char *);

/* This next function has to return -1 in any case - it exits the client and 
   does nothing else. 
   */

int 
tspFixConfig(void) {
	tConf t;
	char **tm = template_mappings;
	int worked = 0;

	Display(LOG_LEVEL_1, ELInfo, "tspFixConfig", GOGO_STR_VERIF_AND_FIX_CFG_FILE);

	memset(&t, 0, sizeof(tConf));
	if (tspReadConfigFile(FileName, &t) != 0) {
		Display(LOG_LEVEL_1, ELError, "tspFixConfig", GOGO_STR_ERR_FROM_CFG_FILE, FileName);
		return -1;
	}

	if (t.template == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspFixConfig", GOGO_STR_CANT_READ_TEMPLATE_FROM_CFG, FileName);
		return -1;
	}

	/* fix template names from older version */

	while (*tm != NULL) {
		//Display(0, ELInfo, "tspFixConfig", "Checkout out %s, config is %s", *tm, t.template);

		if (strcmp(t.template, *tm) == 0) {
			Display(LOG_LEVEL_1, ELInfo, "tspFixConfig", GOGO_STR_WRONG_TEMPLATE_NAME, FileName, *tm, *(tm+1));
			/* needs a replacement in the config file right here.
			   rename the config file to .old and recreate one
			   with the mapped template name.
			   */
			readfixwrite(FileName, "gogoc.conf.new", "template=", *(tm+1));
			worked = 1;
		}
		tm+=2;
	}

	if (worked == 0) 
		Display(LOG_LEVEL_1, ELInfo, "tspFixConfig", GOGO_STR_NOOP_CFG_VALID);

	return -1;
}

static
int readfixwrite(char *in, char *out, char *pname, char *pvalue) {

	FILE *f_in, *f_out;
	char *line;

	line = (char *)malloc(256);

	if ( (f_in = fopen(in, "r")) == NULL)
		return 1;
	if ( (f_out = fopen(out, "w")) == NULL)
		return 1;

	while (fgets(line, 256, f_in) != NULL) {
		char buf[256];
		strncpy(buf, line, strlen(pname));
		if (strcmp(buf, pname) == 0) {
			memset(buf, 0, sizeof(buf));
			strcat(buf, pname);
			strcat(buf, pvalue);
			strcat(buf, "\n");
			fputs(buf, f_out);
		}
		else fputs(line, f_out);
	}

	fclose(f_in);
	fclose(f_out);

	unlink(in);
	rename(out, in);

	return 0;
}
