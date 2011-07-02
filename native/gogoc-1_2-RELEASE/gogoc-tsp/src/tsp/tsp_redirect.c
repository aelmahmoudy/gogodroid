/*
-----------------------------------------------------------------------------
 $Id: tsp_redirect.c,v 1.1 2009/11/20 16:53:41 jasminko Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2008 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"
#include "gogoc_status.h"

#include "log.h"
#include "config.h"
#include "tsp_redirect.h"
#include "tsp_client.h"
#include "xml_tun.h"
#include "hex_strings.h"

/* Determine if a TSP status code means that */
/* redirection should be performed. */
sint32_t tspIsRedirectStatus(sint32_t status) {
	return (status > REDIRECT_STATUS_CODE_BASE);
}

/* Free a broker list */
tRedirectStatus tspFreeBrokerList(tBrokerList *broker_list) {
	tBrokerList *next_broker = NULL;
	
	/* Loop through the list, freeing each element */
	while (broker_list != NULL) {
		next_broker = broker_list->next;
		pal_free(broker_list);
		broker_list = next_broker;
	}

	return TSP_REDIRECT_OK;
}

/* Add a new broker element to a list of brokers */
tRedirectStatus tspAddBrokerToList(tBrokerList **broker_list, char *address, tBrokerAddressType address_type, uint32_t distance) {
	tBrokerList *new_broker = NULL;

	/* Some internal error checking */
	if (broker_list == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspAddBrokerToList", GOGO_STR_RDR_ADD_BROKER_INTERNAL_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	if (address == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspAddBrokerToList", GOGO_STR_RDR_ADD_BROKER_INTERNAL_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	/* Allocate a new broker element */
	if ((new_broker = (tBrokerList *)pal_malloc(sizeof(tBrokerList))) == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspAddBrokerToList", GOGO_STR_RDR_ADD_BROKER_NO_MEM);
		return TSP_REDIRECT_CANT_ALLOCATE_MEM;
	}

	/* Set the broker address */
	pal_snprintf(new_broker->address, MAX_REDIRECT_ADDRESS_LENGTH, address);

	/* Validate that the address was set correctly */
	if (strlen(new_broker->address) != strlen(address)) {
		pal_free(new_broker);
		Display(LOG_LEVEL_1, ELError, "tspAddBrokerToList", GOGO_STR_RDR_ADD_BROKER_ADDRESS_TRUNC);
		return TSP_REDIRECT_ADDRESS_TRUNCATED;
	}

	/* Set the broker distance */
	new_broker->distance = distance;

	/* Set the broker address type */
	new_broker->address_type = address_type;

	/* Add the new broker element at the end of the list */
	new_broker->next = NULL;

	if (*broker_list == NULL) {
		*broker_list = new_broker;
	}
	else {
		while ((*broker_list)->next != NULL) {
			broker_list = &((*broker_list)->next);
		}

		(*broker_list)->next = new_broker;
	}

	return TSP_REDIRECT_OK;
}

/* Log a redirection broker list */
tRedirectStatus tspLogRedirectionList(tBrokerList *broker_list, sint32_t sorted) {
	char broker_list_string[MAX_BROKER_LIST_STRING_LENGTH];
	char *index = broker_list_string;
	sint32_t first_broker = 1;
	size_t space_left = MAX_BROKER_LIST_STRING_LENGTH - 1;
	size_t space_needed = 0;

	/* Calculate the space we have for the string version of the broker list. */
	/* We need to keep enough space to add '...' if the list is too long. */
	space_left -= (	strlen(BROKER_LIST_STRING_START) +
					strlen(BROKER_LIST_STRING_SEPARATOR) +
					strlen(BROKER_LIST_STRING_SUSPENSION) +
					strlen(BROKER_LIST_STRING_END));

	/* Start of list marker */
	index += sprintf(index, "%s", BROKER_LIST_STRING_START);

	/* Loop through the list */
	while (broker_list != NULL) {
		/* First element has no separator before it */
		if (first_broker) {
			/* We need space for the broker's address */
			space_needed = strlen(broker_list->address);

			/* If it fits in the space we have, put it there */
			if (space_left >= space_needed) {
				index += sprintf(index, "%s", broker_list->address);
			}
			/* If it doesn't, indicate there's actually more to the list */
			else {
				index += sprintf(index, "%s", BROKER_LIST_STRING_SUSPENSION);
				break;
			}

			/* Next broker won't be the first one anymore */
			first_broker = 0;
		}
		else {
			/* We need space for a separator, and the broker's address */
			space_needed = strlen(BROKER_LIST_STRING_SEPARATOR) + strlen(broker_list->address);

			/* If it fits in the space we have, put it there */
			if (space_left >= space_needed) {
				index += sprintf(index, "%s%s", BROKER_LIST_STRING_SEPARATOR, broker_list->address);
			}
			/* If it doesn't, indicate there's actually more to the list */
			else {
				index += sprintf(index, "%s%s", BROKER_LIST_STRING_SEPARATOR, BROKER_LIST_STRING_SUSPENSION);
				break;
			}
		}

		/* Move to the next element */
		broker_list = broker_list->next;
	}

	/* End of list marker */
	index += sprintf(index, "%s", BROKER_LIST_STRING_END);

	if (sorted) {
		Display(LOG_LEVEL_1, ELInfo, "tspLogRedirection", GOGO_STR_RDR_SORTED_BROKER_LIST_IS, broker_list_string);
	}
	else {
		Display(LOG_LEVEL_1, ELInfo, "tspLogRedirection", GOGO_STR_RDR_BROKER_LIST_IS, broker_list_string);
	}

	return TSP_REDIRECT_OK;
}

/* Create a broker list from parsed XML information */
tRedirectStatus tspCreateBrokerList(tTunnel *tunnel_info, tBrokerList **broker_list, sint32_t *broker_count) {
	tLinkedList *current_broker = NULL;

	/* Make sure we start from an empty list */
	tspFreeBrokerList(*broker_list);
	*broker_list = NULL;

	/* Initially no element in the list */
	*broker_count = 0;

	/* For each IPv4 broker in the message */
	for (current_broker = tunnel_info->broker_redirect_ipv4; current_broker != NULL; current_broker = current_broker->next) {
		if (*broker_count < MAX_REDIRECT_BROKERS_IN_LIST) { 
			/* Add a new element to the list */
			if (tspAddBrokerToList(broker_list, current_broker->Value, TSP_REDIRECT_BROKER_TYPE_IPV4, 0) != TSP_REDIRECT_OK) {
				Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_CREATE_LIST_CANT_ADD);
				tspFreeBrokerList(*broker_list);
				*broker_list = NULL;
				return TSP_REDIRECT_CANT_ADD_BROKER_TO_LIST;
			}
			else {
				(*broker_count)++;
			}
		}
		else {
			Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_TOO_MANY_BROKERS, MAX_REDIRECT_BROKERS_IN_LIST);
			tspFreeBrokerList(*broker_list);
			*broker_list = NULL;
			return TSP_REDIRECT_TOO_MANY_BROKERS;
		}
	}
	
	/* For each IPv6 broker in the message */
	for (current_broker = tunnel_info->broker_redirect_ipv6; current_broker != NULL; current_broker = current_broker->next) {
		if (*broker_count < MAX_REDIRECT_BROKERS_IN_LIST) {
			/* Add a new element to the list */
			if (tspAddBrokerToList(broker_list, current_broker->Value, TSP_REDIRECT_BROKER_TYPE_IPV6, 0) != TSP_REDIRECT_OK) {
				Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_CREATE_LIST_CANT_ADD);
				tspFreeBrokerList(*broker_list);
				*broker_list = NULL;
				return TSP_REDIRECT_CANT_ADD_BROKER_TO_LIST;
			}
			else {
				(*broker_count)++;
			}
		}
		else {
			Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_TOO_MANY_BROKERS, MAX_REDIRECT_BROKERS_IN_LIST);
			tspFreeBrokerList(*broker_list);
			*broker_list = NULL;
			return TSP_REDIRECT_TOO_MANY_BROKERS;
		}
	}

	/* For each FQDN broker in the message */
	for (current_broker = tunnel_info->broker_redirect_dn; current_broker != NULL; current_broker = current_broker->next) {
		if (*broker_count < MAX_REDIRECT_BROKERS_IN_LIST) {
			/* Add a new element to the list */
			if (tspAddBrokerToList(broker_list, current_broker->Value, TSP_REDIRECT_BROKER_TYPE_FQDN, 0) != TSP_REDIRECT_OK) {
				Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_CREATE_LIST_CANT_ADD);
				tspFreeBrokerList(*broker_list);
				*broker_list = NULL;
				return TSP_REDIRECT_CANT_ADD_BROKER_TO_LIST;
			}
			else {
				(*broker_count)++;
			}
		}
		else {
			Display(LOG_LEVEL_1, ELError, "tspCreateBrokerList", GOGO_STR_RDR_TOO_MANY_BROKERS, MAX_REDIRECT_BROKERS_IN_LIST);
			tspFreeBrokerList(*broker_list);
			*broker_list = NULL;
			return TSP_REDIRECT_TOO_MANY_BROKERS;
		}
	}

	return TSP_REDIRECT_OK;
}

/* Insert a broker element in the list, ordering it based on the distance value */
tRedirectStatus tspInsertInBrokerList(tBrokerList *new_element, tBrokerList **sorted_list) {
	tBrokerList *compared_broker = NULL;
	tBrokerList *precedent_broker = NULL;

	/* If the list is empty, the inserted element becomes the list */
	if (*sorted_list == NULL) {
		*sorted_list = new_element;
	}
	else {
		compared_broker = *sorted_list;

		/* Move to the new element's ordered position in the list */
		while ((compared_broker != NULL) && (new_element->distance > compared_broker->distance)) {
			precedent_broker = compared_broker;
			compared_broker = compared_broker->next;
		}

		/* Adjust the pointers so that the new element becomes part of the list */
		new_element->next = compared_broker;

		if (precedent_broker != NULL) {
			precedent_broker->next = new_element;
		}
		else {
			*sorted_list = new_element;
		}
	}

	return TSP_REDIRECT_OK;
}

/* Sort a list of brokers based on the distance value (roundtrip time) */
tRedirectStatus tspSortBrokerList(tBrokerList **broker_list, tConf *conf, sint32_t broker_count) {
	tBrokerList *sorted_list = NULL;
	tBrokerList *current_broker = NULL;
	tBrokerList *new_element = NULL;

	Display(LOG_LEVEL_2, ELInfo, "tspSortBrokerList", GOGO_STR_RDR_SORTING_BROKER_LIST);

	/* Get the distance values */
	if (tspGetBrokerDistances(*broker_list, broker_count, conf) != TSP_REDIRECT_OK) {
		Display(LOG_LEVEL_1, ELError, "tspSortBrokerList", GOGO_STR_RDR_SORT_LIST_CANT_GET_DIST);
		return TSP_REDIRECT_CANT_GET_DISTANCES;
	}

	current_broker = *broker_list;
	
	/* For each broker in the original list */
	while (current_broker != NULL) {
		/* Create a new broker element */
		if ((new_element = (tBrokerList *)pal_malloc(sizeof(tBrokerList))) == NULL) {
			tspFreeBrokerList(sorted_list);
			Display(LOG_LEVEL_1, ELError, "tspSortBrokerList", GOGO_STR_RDR_SORT_LIST_CANT_ALLOC);
			return TSP_REDIRECT_CANT_ALLOCATE_MEM;
		}	

		/* Copy the address from the broker in the original list */
		strcpy(new_element->address, current_broker->address);
		/* Copy the distance from the broker in the original list */
		new_element->distance = current_broker->distance;
                /* Copy the address_type as well! */
                new_element->address_type = current_broker->address_type;
		new_element->next = NULL;

		/* Insert (sorted) the new element in the sorted list */
		if (tspInsertInBrokerList(new_element, &sorted_list) != TSP_REDIRECT_OK) {
			tspFreeBrokerList(sorted_list);
			Display(LOG_LEVEL_1, ELError, "tspSortBrokerList", GOGO_STR_RDR_SORT_LIST_CANT_INSERT);
			return TSP_REDIRECT_CANT_INSERT_BROKER_IN_LIST;
		}

		/* Move to the next broker in the original list */
		current_broker = current_broker->next;
	}

	/* Free the original list */
	tspFreeBrokerList(*broker_list);

	/* Replace the original list with the sorted one */
	*broker_list = sorted_list;

	return TSP_REDIRECT_OK;
}

/* A simple function to log the fact that we received a redirection message */
tRedirectStatus tspLogReceivedRedirection(char *payload, tConf *conf) {
	char status[MAX_REDIRECT_STATUS_LENGTH];

	/* Clear out the buffer */
	memset(status, 0, sizeof(status));

	/* Read the status line from the broker's reply */
	sscanf(payload, "%[^\n]", status);

	/* Log the message including the server and the redirection status */
	Display(LOG_LEVEL_1, ELInfo, "tspLogReceivedRedirection", GOGO_STR_RECEIVED_REDIRECTION, conf->server, status); 

	return TSP_REDIRECT_OK;
}

/* Main function to handle a redirection message */
tRedirectStatus tspHandleRedirect(char *payload, tConf *conf, tBrokerList **broker_list)
{
	tTunnel tunnel_info;
	sint32_t broker_count = 0;

	tspLogReceivedRedirection(payload, conf);

	/* Parse the XML data in the payload */
	if (tspExtractPayload(payload, &tunnel_info) != 0) {
		Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_EXTRACT_PAYLOAD);
		return TSP_REDIRECT_CANT_EXTRACT_PAYLOAD;
	}

	/* Create a broker list from that information */
	if (tspCreateBrokerList(&tunnel_info, broker_list, &broker_count) != TSP_REDIRECT_OK) {
		Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_CREATE_LIST);
		return TSP_REDIRECT_CANT_CREATE_LIST;
	}

	/* Log the redirection message and details */
	if (tspLogRedirectionList(*broker_list, 0) != TSP_REDIRECT_OK) {
		Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_LOG);
		//return TSP_REDIRECT_CANT_LOG_REDIRECTION;
	}

	/* If the broker list has more than one element */
	if (broker_count > 1) {
		/* Sort the broker list */
		if (tspSortBrokerList(broker_list, conf, broker_count) != TSP_REDIRECT_OK) {
			Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_SORT_LIST);
			return TSP_REDIRECT_CANT_SORT_LIST;
		}

		/* Log the redirection message and details */
		if (tspLogRedirectionList(*broker_list, 1) != TSP_REDIRECT_OK) {
			Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_LOG);
			//return TSP_REDIRECT_CANT_LOG_REDIRECTION;
		}

		/* Write the broker list to file */
		if (tspWriteBrokerListToFile(conf->broker_list_file, *broker_list) != TSP_REDIRECT_OK) {
			Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_CANT_SAVE_LIST);
			// return TSP_REDIRECT_CANT_SAVE_BROKER_LIST;
		}
	}

	/* An empty list is an error */
	if (broker_count == 0) {
		Display(LOG_LEVEL_1, ELError, "tspHandleRedirect", GOGO_STR_RDR_NULL_LIST);
		return TSP_REDIRECT_EMPTY_BROKER_LIST;
	}

	return TSP_REDIRECT_OK;
}

/* Read the last server from the last_server file */
tRedirectStatus tspReadLastServerFromFile(char *last_server_file, char *buffer) {
	FILE *file;
	char line[MAX_REDIRECT_LAST_SERVER_LINE_LENGTH];
	sint32_t found_server = 0;

	/* Some internal checking */
	if (last_server_file == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspReadLastServerFromFile", GOGO_STR_RDR_READ_LAST_SERVER_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	if (buffer == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspReadLastServerFromFile", GOGO_STR_RDR_READ_LAST_SERVER_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	/* Try to open the last_server file */
	if ((file = fopen(last_server_file, "r")) == NULL) {
		return TSP_REDIRECT_CANT_OPEN_FILE;
	}

	/* Loop through the lines in the file */
	while (fgets(line, sizeof(line), file)) {
		/* Skip blank lines and comments */
		if (*line == '#' || *line == '\r' || *line == '\n') {
			continue;
		}
			
		/* Remove CR and LF */
		if (strlen(line) && (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')) line[strlen(line) - 1] = '\0';
		if (strlen(line) && (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')) line[strlen(line) - 1] = '\0';

		/* Copy the line to the buffer */
		sprintf(buffer, line);

		/* We found a server, so stop looking */
		found_server = 1;
		break;
	}

	/* Close the file */
	fclose(file);

	if (found_server) {
		return TSP_REDIRECT_OK;
	}
	else {
		return TSP_REDIRECT_NO_LAST_SERVER;
	}
}

/* Write the last server to the last_server file */
tRedirectStatus tspWriteLastServerToFile(char *last_server_file, char *last_server) {
	FILE *file;
	
	/* Some internal checking */
	if (last_server_file == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspWriteLastServerToFile", GOGO_STR_RDR_WRITE_LAST_SERVER_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	if (last_server == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspWriteLastServerToFile", GOGO_STR_RDR_WRITE_LAST_SERVER_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	/* Try to open the last_server file */
	if ((file = fopen(last_server_file, "w")) == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspWriteLastServerToFile", GOGO_STR_RDR_WRITE_LAST_SERVER_CANT_OPEN, last_server_file);
		return TSP_REDIRECT_CANT_OPEN_FILE;
	}

	/* Write the last server to the last_server file */
	if (fprintf(file, "%s\n", last_server) < 0) {
		Display(LOG_LEVEL_1, ELError, "tspWriteLastServerToFile", GOGO_STR_RDR_WRITE_LAST_SERVER_CANT_WRITE, last_server, last_server_file);
		fclose(file);
		return TSP_REDIRECT_CANT_WRITE_TO_FILE;
	}

	/* Close the file */
	fclose(file);

	return TSP_REDIRECT_OK;
}

/* Write a broker list to the broker_list file */
tRedirectStatus tspWriteBrokerListToFile(char *broker_list_file, tBrokerList *broker_list) {
	FILE *file;
	tBrokerList *current_broker = NULL;

	/* Some internal checking */
	if (broker_list_file == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspWriteBrokerListToFile", GOGO_STR_RDR_WRITE_BROKER_LIST_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	/* Try to open the broker_list file */
	if ((file = fopen(broker_list_file, "w")) == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspWriteBrokerListToFile", GOGO_STR_RDR_WRITE_BROKER_LIST_CANT_OPEN, broker_list_file);
		return TSP_REDIRECT_CANT_OPEN_FILE;
	}

	/* Loop through the broker list */
	for (current_broker = broker_list; current_broker != NULL; current_broker = current_broker->next) {
		/* Write each broker's address to the broker_list file */
		if (fprintf(file, "%s\n", current_broker->address) < 0) {
			Display(LOG_LEVEL_1, ELError, "tspWriteBrokerListToFile", GOGO_STR_RDR_WRITE_BROKER_LIST_CANT_WRITE, broker_list_file);
			fclose(file);
			return TSP_REDIRECT_CANT_WRITE_TO_FILE;
		}
	}

	/* Close the file */
	fclose(file);

	return TSP_REDIRECT_OK;
}

/* Create a broker list from the broker_list file */
tRedirectStatus tspReadBrokerListFromFile(char *broker_list_file, tBrokerList **broker_list) {
	FILE *file;
	char line[MAX_REDIRECT_BROKER_LIST_LINE_LENGTH];
	sint32_t broker_count = 0;
	
	/* Some internal checking */
	if (broker_list_file == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspReadBrokerListFromFile", GOGO_STR_RDR_WRITE_BROKER_LIST_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	if (broker_list == NULL) {
		Display(LOG_LEVEL_1, ELError, "tspReadBrokerListFromFile", GOGO_STR_RDR_WRITE_BROKER_LIST_INT_ERR);
		return TSP_REDIRECT_INTERNAL_ERR;
	}

	*broker_list = NULL;

	/* Try to open the broker_list file */
	if ((file = fopen(broker_list_file, "r")) == NULL) {
		return TSP_REDIRECT_CANT_OPEN_FILE;
	}

	/* Loop through the lines in the file */
	while (fgets(line, sizeof(line), file)) {
			
		/* Remove CR and LF */
		if (strlen(line) && (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')) line[strlen(line) - 1] = '\0';
		if (strlen(line) && (line[strlen(line) - 1] == '\n' || line[strlen(line) - 1] == '\r')) line[strlen(line) - 1] = '\0';

		if (strlen(line) > 0) {

			if (broker_count < MAX_REDIRECT_BROKERS_IN_LIST) {
				/* Add a new element to the broker list */
				if (tspAddBrokerToList(broker_list, line, TSP_REDIRECT_BROKER_TYPE_NONE, 0) != TSP_REDIRECT_OK) {
					Display(LOG_LEVEL_1, ELError, "tspReadBrokerListFromFile", GOGO_STR_RDR_READ_BROKER_LIST_CANT_ADD, broker_list_file);
					fclose(file);
					return TSP_REDIRECT_CANT_ADD_BROKER_TO_LIST;
				}
				else {
					broker_count++;
				}
			}
			else {
				fclose(file);
				return TSP_REDIRECT_TOO_MANY_BROKERS;
			}
		}
	}

	/* Close the file */
	fclose(file);	

	return TSP_REDIRECT_OK;
}
