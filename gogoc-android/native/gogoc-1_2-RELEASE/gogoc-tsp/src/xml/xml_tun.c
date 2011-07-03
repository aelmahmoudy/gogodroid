/*
-----------------------------------------------------------------------------
 $Id: xml_tun.c,v 1.2 2010/03/07 20:15:45 carl Exp $
-----------------------------------------------------------------------------
Copyright (c) 2001-2007 gogo6 Inc. All rights reserved.

  For license information refer to CLIENT-LICENSE.TXT
-----------------------------------------------------------------------------
*/

#include "platform.h"

#include "xmlparse.h"

#define   XMLTUN
#include "xml_tun.h"

#define TEST 0

/*
 * Here are declarations of the appropriate treatment functions, taking care
 * of the content of the nodes parsed. This is required to insure binding to the
 * procedures in the node structure.
 *
 * Use the PROC macro to help insure proper declaration.
 */

PROC(tunnel);

PROC(server);
PROC(client);
PROC(broker);

PROC(address);
PROC(dns_server);
PROC(router);
PROC(prefix);
PROC(as);
PROC(keepalive);

/*
 * Here we define a simple node structure containing only one entry
 * corresponding to the root node.
 *
 * Such a structure must be created for each level of information that
 * is part of the expected xml content.
 */

tNode Root[] =

  STARTLIST
    NODE(tunnel) ATTR(action) ATTR(type) ATTR(lifetime) ATTR(proxy) ATTR(mtu) ENDNODE
  ENDLIST

tNode Tunnel[] =

  STARTLIST
    NODE(server)                  ENDNODE
    NODE(client)                  ENDNODE
    NODE(broker)                  ENDNODE
  ENDLIST

tNode Server[] =

  STARTLIST
    NODE(address) ATTR(type)      ENDNODE
    NODE(router)  ATTR(protocol)  ENDNODE
  ENDLIST

tNode Client[] =

  STARTLIST
    NODE(address) ATTR(type)       ENDNODE
    NODE(dns_server) ATTR(type)    ENDNODE
    NODE(router)  ATTR(protocol)   ENDNODE
    NODE(keepalive) ATTR(interval) ENDNODE
  ENDLIST

tNode Router[] =

  STARTLIST
    NODE(prefix)     ATTR(length) ENDNODE
    NODE(dns_server) ATTR(type)   ENDNODE
    NODE(as)         ATTR(number) ENDNODE
  ENDLIST

tNode DNSServer[] =

  STARTLIST
    NODE(address) ATTR(type)      ENDNODE
  ENDLIST

tNode Broker[] =

  STARTLIST
    NODE(address) ATTR(type) ENDNODE
  ENDLIST

tNode Keepalive[] =

  STARTLIST
    NODE(address) ATTR(type) ENDNODE
  ENDLIST

/*
 * Support functions
 */

static int Assign(char *str, char **toStr)
{
  char *tol;

  if (str == NULL) return 0;

  /* turn answer to lower case */
  /* should help homogenize    */
  /* scripting				   */

  tol = str;
  while (*tol != '\0') {
	  *tol = (char)tolower((int)*tol);
	  tol++;
  }

  *toStr = (char *) malloc(strlen(str) + 1);

  if (*toStr == NULL) {
    printf("Assign: Memory allocation error!\n");
    return -1;
  }

  strcpy(*toStr, str);

  return 0;
}

static int AssignToList(char *str, tLinkedList **toList)
{
  tLinkedList *ll;

  if (str == NULL) return 0;

  ll = (tLinkedList *) malloc(sizeof(tLinkedList));
  if (ll == NULL) {
    printf("AssignToList: Memory allocation error!\n");
    return -1;
  }

  ll->Value = (char *) malloc(strlen(str) + 1);

  if (ll->Value == NULL) {
    printf("AssignToList: Memory allocation error!\n");
    return -1;
  }

  strcpy(ll->Value, str);

  ll->next = *toList;
  *toList  = ll;

  return 0;
}

/*
 * Here are defined every treatment functions required. use the PROC
 * macro to help insure proper declaration.
 *
 * These event functions update the contain of the tTunnel instance with the
 * information that is part of the XML structure.
 */

static tTunnel *theTunnelInfo;

static int
  client,
  server,
  router,
  dns_server,
  broker,
  keepalive;

PROC(default)
{
  printf("Found <%s>%.10s%s</%s>\n", n->name, content, strlen(content) > 10 ? "..." : "", n->name);
  return 0;
}

PROC(tunnel)
{
  Assign(n->attributes[0].value, &theTunnelInfo->action  );
  Assign(n->attributes[1].value, &theTunnelInfo->type    );
  Assign(n->attributes[2].value, &theTunnelInfo->lifetime);
  Assign(n->attributes[3].value, &theTunnelInfo->proxy   );
  Assign(n->attributes[4].value, &theTunnelInfo->mtu	 );

  return XMLParse(content, Tunnel);
}

PROC(server)
{
  int res;

  server = 1;
  res = XMLParse(content, Server);
  server = 0;
  return res;
}

PROC(broker)
{
  int res;

  broker = 1;
  res = XMLParse(content, Broker);
  broker = 0;
  return res;
}

PROC(client)
{
  int res;

  client = 1;
  res = XMLParse(content, Client);
  client = 0;

  return res;
}

PROC(dns_server)
{
  int res;

  dns_server = 1;
  res = XMLParse(content, DNSServer);
  dns_server = 0;

  return res;
}

PROC(router)
{
  int res;

  router = 1;
  Assign(n->attributes[0].value, &theTunnelInfo->router_protocol);
  res = XMLParse(content, Router);
  router = 0;

  return res;
}

PROC(as)
{
  if (client) {
    Assign(n->attributes[0].value, &theTunnelInfo->client_as);
  } else if (server) {
    Assign(n->attributes[0].value, &theTunnelInfo->server_as);
  }
  return 0;
}

PROC(keepalive)
{
  int res;
  Assign(n->attributes[0].value, &theTunnelInfo->keepalive_interval);
  keepalive = 1;
  res = XMLParse(content, Keepalive);
  keepalive = 0;
  return res;
}


PROC(prefix) {
  Assign(n->attributes[0].value, &theTunnelInfo->prefix_length);
  Assign(content, &theTunnelInfo->prefix);
  return 0;
}

PROC(address)
{
  if (client) {
    if (keepalive) {
      if ((!strcmp(n->attributes[0].value, "ipv6")) ||
	  (!strcmp(n->attributes[0].value, "ipv4"))) {
	Assign(content, &theTunnelInfo->keepalive_address);
      }
    }
    else if (router) {
      if (dns_server) {
        if (!strcmp(n->attributes[0].value, "ipv4")) {
          AssignToList(content, &theTunnelInfo->dns_server_address_ipv4);
        } else if (!strcmp(n->attributes[0].value, "ipv6")) {
          AssignToList(content, &theTunnelInfo->dns_server_address_ipv6);
        }
      }
    } else if (dns_server) {
      if (!strcmp(n->attributes[0].value, "ipv6")) {
        Assign(content, &theTunnelInfo->client_dns_server_address_ipv6);
      }
    } else {
      if (!strcmp(n->attributes[0].value, "ipv4")) {
        Assign(content, &theTunnelInfo->client_address_ipv4);
      } else if (!strcmp(n->attributes[0].value, "ipv6")) {
        Assign(content, &theTunnelInfo->client_address_ipv6);
      } else if (!strcmp(n->attributes[0].value, "dn")) {
        Assign(content, &theTunnelInfo->client_dns_name);
      }
    }
  } else if (server) {
    if (!strcmp(n->attributes[0].value, "ipv4")) {
      Assign(content, &theTunnelInfo->server_address_ipv4);
    } else if (!strcmp(n->attributes[0].value, "ipv6")) {
      Assign(content, &theTunnelInfo->server_address_ipv6);
    }
  } else if (broker) {
	if (!strcmp(n->attributes[0].value, "ipv4")) {
		AssignToList(content, &theTunnelInfo->broker_redirect_ipv4);
	}
	else if (!strcmp(n->attributes[0].value, "ipv6")) {
		AssignToList(content, &theTunnelInfo->broker_redirect_ipv6);
	}
	else if (!strcmp(n->attributes[0].value, "dn")) {
		AssignToList(content, &theTunnelInfo->broker_redirect_dn);
	}
  }
  return 0;
}

/* Put here any relevant code... */

void tspFree(char *var)
{
  if (var) free(var);
}

void tspFreeLinkedList(tLinkedList *list)
{
  if(list) {
    if(list->next) {
      tspFreeLinkedList(list->next);
      free(list->next);
      list->next = NULL;
    }
    if(list->Value) {
      free(list->Value);
      list->Value = NULL;
    }
  }
}

void tspClearTunnelInfo(tTunnel *Tunnel)
{
  if (Tunnel) {
    tspFree(Tunnel->action);
    tspFree(Tunnel->type);
    tspFree(Tunnel->lifetime);
    tspFree(Tunnel->proxy);
    tspFree(Tunnel->mtu);
    tspFree(Tunnel->client_address_ipv4);
    tspFree(Tunnel->client_address_ipv6);
    tspFree(Tunnel->client_dns_server_address_ipv6);
    tspFree(Tunnel->client_dns_name);
    tspFree(Tunnel->server_address_ipv4);
    tspFree(Tunnel->server_address_ipv6);
    tspFree(Tunnel->router_protocol);
    tspFree(Tunnel->prefix_length);
    tspFree(Tunnel->prefix);
    tspFree(Tunnel->client_as);
    tspFree(Tunnel->server_as);
    tspFree(Tunnel->keepalive_interval);
    tspFree(Tunnel->keepalive_address);
    tspFreeLinkedList(Tunnel->dns_server_address_ipv4);
    tspFreeLinkedList(Tunnel->dns_server_address_ipv6);
    tspFreeLinkedList(Tunnel->broker_address_ipv4);
	tspFreeLinkedList(Tunnel->broker_redirect_ipv4);
	tspFreeLinkedList(Tunnel->broker_redirect_ipv6);
	tspFreeLinkedList(Tunnel->broker_redirect_dn);
  }
}

void ShowList(tLinkedList *l)
{
  int first = 1;

  while (l != NULL) {
    if (!first) printf(", ");
    if (l->Value != NULL) printf("%s", l->Value);
    l = l->next;
    first = 0;
  }
}

void tspXMLShowInfo(tTunnel *Tunnel)
{
  printf("Parsed Info:\n\n");

  printf("  action                         = [%s]\n", Tunnel->action              == NULL ? "" : Tunnel->action             );
  printf("  type                           = [%s]\n", Tunnel->type                == NULL ? "" : Tunnel->type               );
  printf("  lifetime                       = [%s]\n", Tunnel->lifetime            == NULL ? "" : Tunnel->lifetime           );
  printf("  proxy                          = [%s]\n", Tunnel->proxy               == NULL ? "" : Tunnel->proxy              );
  printf("  mtu                            = [%s]\n", Tunnel->mtu                 == NULL ? "" : Tunnel->mtu                );
  printf("  client address ipv4            = [%s]\n", Tunnel->client_address_ipv4 == NULL ? "" : Tunnel->client_address_ipv4);
  printf("  client address ipv6            = [%s]\n", Tunnel->client_address_ipv6 == NULL ? "" : Tunnel->client_address_ipv6);
  printf("  client dns server address ipv6 = [%s]\n", Tunnel->client_dns_server_address_ipv6 == NULL ? "" : Tunnel->client_dns_server_address_ipv6);
  printf("  client dns name                = [%s]\n", Tunnel->client_dns_name     == NULL ? "" : Tunnel->client_dns_name    );
  printf("  server address ipv4            = [%s]\n", Tunnel->server_address_ipv4 == NULL ? "" : Tunnel->server_address_ipv4);
  printf("  server address ipv6            = [%s]\n", Tunnel->server_address_ipv6 == NULL ? "" : Tunnel->server_address_ipv6);
  printf("  router protocol                = [%s]\n", Tunnel->router_protocol     == NULL ? "" : Tunnel->router_protocol    );
  printf("  prefix length                  = [%s]\n", Tunnel->prefix_length       == NULL ? "" : Tunnel->prefix_length      );
  printf("  prefix                         = [%s]\n", Tunnel->prefix              == NULL ? "" : Tunnel->prefix             );
  printf("  client as number               = [%s]\n", Tunnel->client_as           == NULL ? "" : Tunnel->client_as          );
  printf("  server as number               = [%s]\n", Tunnel->server_as           == NULL ? "" : Tunnel->server_as          );
  printf("  keepalive interval             = [%s]\n", Tunnel->keepalive_interval  == NULL ? "" : Tunnel->keepalive_interval );
  printf("  keepalive address              = [%s]\n", Tunnel->keepalive_address == NULL ? "" : Tunnel->keepalive_address);
  printf("  dns server addresse(s) ipv4    = ["); ShowList(Tunnel->dns_server_address_ipv4); printf("]\n");
  printf("  dns server addresse(s) ipv6    = ["); ShowList(Tunnel->dns_server_address_ipv6); printf("]\n");
  printf("  broker addresse(s) ipv4        = ["); ShowList(Tunnel->broker_address_ipv4);     printf("]\n");

  printf("  broker redirect ipv4           = ["); ShowList(Tunnel->broker_redirect_ipv4);     printf("]\n");
  printf("  broker redirect ipv6           = ["); ShowList(Tunnel->broker_redirect_ipv6);     printf("]\n");
  printf("  broker redirect dn             = ["); ShowList(Tunnel->broker_redirect_dn);       printf("]\n");
}

int tspXMLParse(char *Data, tTunnel *Tunnel)
{
  tspClearTunnelInfo(Tunnel);

  client = server = router = dns_server = broker = 0;

  theTunnelInfo = Tunnel;

  return XMLParse(Data, Root);
}

#if TEST

char *testData =
" <tunnel action=\"info\" type=\"v6v4\" lifetime=\"1440\" proxy=\"yes\" mtu=\"1280\">\n"
"  <server>\n"
"   <address type=\"ipv4\">206.123.31.114</address>\n"
"   <address type=\"ipv6\">3ffe:b00:c18:ffff:0000:0000:0000:0000</address>\n"
"   <router protocol=\"bgp\">\n"
"    <as number=\"23456\"/>\n"
"   </router>\n"
"  </server>\n"
"  <client>\n"
"   <address type=\"ipv4\">1.1.1.1</address>\n"
"   <address type=\"ipv6\">3ffe:b00:c18:ffff::0000:0000:0000:0001</address>\n"
"   <address type=\"dn\">userid.domain</address>\n"
"   <router protocol=\"bgp\">\n"
"    <prefix length=\"48\">3ffe:0b00:c18:1234::</prefix>\n"
"    <as number=\"12345\"/>\n"
"    <dns_server>\n"
"     <address type=\"ipv4\">2.3.4.5</address>\n"
"     <address type=\"ipv4\">2.3.4.6</address>\n"
"     <address type=\"ipv6\">3ffe:0c00::1</address>\n"
"    </dns_server>\n"
"   </router>\n"
"   <keepalive interval=\"30\"><address type=\"ipv6\">::</address></keepalive>\n"
"  </client>\n"
" </tunnel>\n";

main()
{
  tTunnel t;
  static char string[5000];
  int res;

  printf("Start....\n");

  memset(&t, 0, sizeof(t));
  strcpy(string, testData);

  res = tspXMLParse(string, &t);

  if (res != 0) {
    printf("Result = %d\n", res);
  }

  tspXMLShowInfo(&t);

  return 0;
}

#endif

/*----- xmltsp.c -----------------------------------------------------------------------*/

